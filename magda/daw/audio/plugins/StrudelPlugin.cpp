#include "StrudelPlugin.hpp"

#include <choc/javascript/choc_javascript.h>
#include <choc/javascript/choc_javascript_QuickJS.h>

namespace magda::daw::audio {

const char* StrudelPlugin::xmlTypeName = "strudel";

// JS Engine wrapper
struct StrudelPlugin::JSEngine {
    choc::javascript::Context ctx;
    bool ready = false;

    bool init() {
        ctx = choc::javascript::createQuickJSContext();
        if (!ctx) return false;

        auto exeDir = juce::File::getSpecialLocation(
            juce::File::currentExecutableFile).getParentDirectory();
        auto strudelDir = exeDir.getChildFile("strudel");

        // Browser polyfills
        try {
            ctx.evaluateExpression(
                "var console={log:function(){},warn:function(){},error:function(){},"
                "info:function(){},debug:function(){}};"
                "var document={createElement:function(){return{style:{}}},body:{appendChild:function(){}},"
                "addEventListener:function(){},removeEventListener:function(){},"
                "dispatchEvent:function(){},createEvent:function(){return{initEvent:function(){}}}};"
                "var navigator={userAgent:'QuickJS'};"
                "var location={href:'',hostname:'localhost',protocol:'file:'};"
                "var setTimeout=function(fn){fn();return 0};"
                "var clearTimeout=function(){};"
                "var setInterval=function(){return 0};"
                "var clearInterval=function(){};"
                "var requestAnimationFrame=function(){return 0};"
                "var performance={now:function(){return 0}};"
                "var AudioContext=function(){};"
                "var SharedWorker=function(){this.port={onmessage:null,postMessage:function(){}};};"
                "var Worker=function(){this.onmessage=null;this.postMessage=function(){};};"
                "var URL=function(a){this.href=a;};"
                "var window=globalThis;var self=globalThis;"
            );
        } catch (...) {
            return false;
        }

        // Load Strudel bundle
        auto bundleFile = strudelDir.getChildFile("strudel-core-bundle.js");
        if (!bundleFile.existsAsFile()) return false;
        auto bundle = bundleFile.loadFileAsString();

        try {
            ctx.evaluateExpression(bundle.toStdString());
        } catch (...) {
            return false;
        }

        // Load bridge
        auto bridgeFile = strudelDir.getChildFile("strudel-bridge.js");
        if (!bridgeFile.existsAsFile()) return false;
        auto bridge = bridgeFile.loadFileAsString();

        try {
            ctx.evaluateExpression(bridge.toStdString());
        } catch (...) {
            return false;
        }

        ready = true;
        return true;
    }

    juce::String setPattern(const juce::String& code) {
        if (!ready) return "JS not ready";
        try {
            auto result = ctx.invoke("__setPattern", code.toStdString());
            return juce::String(result.toString());
        } catch (const std::exception& e) {
            return e.what();
        } catch (...) {
            return "Unknown JS error";
        }
    }

    juce::String queryPattern(double start, double end) {
        if (!ready) return "[]";
        try {
            auto result = ctx.invoke("__queryPattern", start, end);
            return juce::String(result.toString());
        } catch (...) {
            return "[]";
        }
    }
};

// Note name to MIDI
int StrudelPlugin::noteNameToMidi(const juce::String& name) {
    static const int noteMap[] = {9,11,0,2,4,5,7}; // a b c d e f g
    auto s = name.trim().toLowerCase();
    if (s.isEmpty()) return -1;
    if (s.containsOnly("0123456789")) return s.getIntValue();

    char letter = s[0];
    if (letter < 'a' || letter > 'g') return -1;
    int note = noteMap[letter - 'a'];
    int pos = 1;
    if (pos < s.length() && s[pos] == '#') { note++; pos++; }
    else if (pos < s.length() && s[pos] == 'b' && (pos+1 >= s.length() || s[pos+1] < 'a')) { note--; pos++; }
    int octave = 4;
    if (pos < s.length()) octave = s.substring(pos).getIntValue();
    return note + (octave + 1) * 12;
}

StrudelPlugin::StrudelPlugin(const te::PluginCreationInfo& info) : te::Plugin(info) {
    auto um = getUndoManager();
    codeValue.referTo(state, juce::Identifier("code"), um, "c3 e3 g3 b3");
    initJS();
}

StrudelPlugin::~StrudelPlugin() = default;

void StrudelPlugin::initJS() {
    js_ = std::make_unique<JSEngine>();
    if (!js_->init()) {
        lastError_ = "Failed to initialize JS runtime";
        js_.reset();
    }
}

void StrudelPlugin::initialise(const te::PluginInitialisationInfo& info) {
    sampleRate_ = info.sampleRate;
    lastCyclePos_ = 0.0;
    // Try to compile initial pattern
    if (codeValue.get().isNotEmpty())
        evaluate(codeValue.get());
}

void StrudelPlugin::deinitialise() {}

void StrudelPlugin::reset() {
    lastCyclePos_ = 0.0;
    for (auto& v : voices_) v.active = false;
}

juce::String StrudelPlugin::evaluate(const juce::String& code) {
    if (code.isEmpty()) return "Empty pattern";

    // Try JS engine first (for full Strudel syntax)
    if (js_ && js_->ready) {
        auto err = js_->setPattern(code);
        if (err.isEmpty()) {
            patternActive_ = true;
            lastError_ = {};
            codeValue = code;
            cachedCode_ = code;
            // Pre-query one cycle to cache
            auto json = js_->queryPattern(0.0, 1.0);
            cachedEvents_.clear();
            if (json.startsWith("[")) {
                auto parsed = juce::JSON::parse(json);
                if (auto* arr = parsed.getArray()) {
                    for (const auto& item : *arr) {
                        NoteEvent ev;
                        auto noteVal = item.getProperty("note", 60);
                        if (noteVal.isString())
                            ev.note = noteNameToMidi(noteVal.toString());
                        else
                            ev.note = (int)noteVal;
                        ev.onset = (double)item.getProperty("onset", 0.0);
                        ev.duration = (double)item.getProperty("dur", 0.25);
                        ev.velocity = (float)(double)item.getProperty("vel", 1.0);
                        if (ev.note >= 0 && ev.note <= 127)
                            cachedEvents_.push_back(ev);
                    }
                }
            }
            return {};
        }
        // JS failed, fall through to C++ parser
    }

    // Fallback: simple C++ mini notation parser
    cachedEvents_.clear();
    auto tokens = juce::StringArray::fromTokens(code.trim(), " \t\n,", "\"'");
    tokens.removeEmptyStrings();
    if (tokens.isEmpty()) { lastError_ = "No notes"; return lastError_; }

    double step = 1.0 / tokens.size();
    for (int i = 0; i < tokens.size(); ++i) {
        auto token = tokens[i].trim();
        if (token == "~" || token == "-" || token == ".") continue;
        int midi = noteNameToMidi(token);
        if (midi < 0 || midi > 127) continue;
        cachedEvents_.push_back({midi, i * step, step * 0.9, 1.0f});
    }

    if (cachedEvents_.empty()) { lastError_ = "No valid notes"; return lastError_; }
    patternActive_ = true;
    lastError_ = {};
    codeValue = code;
    cachedCode_ = code;
    return {};
}

juce::String StrudelPlugin::getCode() const { return codeValue.get(); }
void StrudelPlugin::setCode(const juce::String& code) { evaluate(code); }
juce::String StrudelPlugin::getLastError() const { return lastError_; }
bool StrudelPlugin::hasActivePattern() const { return patternActive_; }

void StrudelPlugin::applyToBuffer(const te::PluginRenderContext& rc) {
    if (!patternActive_ || rc.destBuffer == nullptr)
        return;

    auto editTime = rc.editTime;
    double bpm = edit.tempoSequence.getBpmAt(editTime.getStart());
    double cps = bpm / 60.0 / 4.0;

    int numSamples = rc.bufferNumSamples;
    int startSample = rc.bufferStartSample;
    double blockSeconds = numSamples / sampleRate_;
    double blockCycles = blockSeconds * cps;

    double startCycle = lastCyclePos_;
    double endCycle = startCycle + blockCycles;

    // Trigger new notes from cached pattern
    for (const auto& ev : cachedEvents_) {
        int firstCycle = (int)startCycle;
        int lastCycleInt = (int)endCycle;
        for (int c = firstCycle; c <= lastCycleInt; ++c) {
            double eventTime = c + ev.onset;
            if (eventTime >= startCycle && eventTime < endCycle) {
                // Find free voice
                TidalVoice* voice = nullptr;
                for (auto& v : voices_) {
                    if (!v.active) { voice = &v; break; }
                }
                if (!voice) {
                    // Steal oldest
                    voice = &voices_[0];
                    for (auto& v : voices_)
                        if (v.startTime < voice->startTime) voice = &v;
                }
                double durSeconds = ev.duration / cps;
                voice->trigger(ev.note, ev.velocity, eventTime, durSeconds, (float)sampleRate_);
            }
        }
    }

    // Release voices that have ended
    for (auto& v : voices_) {
        if (v.active && v.envStage < 2) {
            double elapsed = (endCycle - v.startTime) / cps;
            double dur = (v.endTime - v.startTime) / cps;
            if (elapsed > dur) v.release();
        }
    }

    // Render audio
    int numChannels = rc.destBuffer->getNumChannels();
    for (int i = 0; i < numSamples; ++i) {
        float sample = 0.0f;
        for (auto& v : voices_) {
            if (v.active)
                sample += v.render((float)sampleRate_);
        }
        // Soft clip
        sample = std::tanh(sample);

        if (numChannels >= 1)
            rc.destBuffer->addSample(0, startSample + i, sample);
        if (numChannels >= 2)
            rc.destBuffer->addSample(1, startSample + i, sample);
    }

    lastCyclePos_ = endCycle;
}

void StrudelPlugin::restorePluginStateFromValueTree(const juce::ValueTree& v) {
    if (v.hasProperty("code"))
        codeValue = v.getProperty("code").toString();
    if (codeValue.get().isNotEmpty())
        evaluate(codeValue.get());
}

// Editor
class StrudelEditorComponent : public te::Plugin::EditorComponent {
  public:
    StrudelEditorComponent(StrudelPlugin& p) : plugin_(p) {
        document_.replaceAllContent(plugin_.getCode());
        editor_ = std::make_unique<juce::CodeEditorComponent>(document_, nullptr);
        editor_->setColour(juce::CodeEditorComponent::backgroundColourId, juce::Colour(0xff1e1e2e));
        editor_->setColour(juce::CodeEditorComponent::defaultTextColourId, juce::Colour(0xffcdd6f4));
        editor_->setTabSize(2, true);
        addAndMakeVisible(*editor_);

        runBtn_.setButtonText("Run");
        runBtn_.onClick = [this] { run(); };
        addAndMakeVisible(runBtn_);

        status_.setJustificationType(juce::Justification::topLeft);
        updateStatus();
        addAndMakeVisible(status_);

        setSize(680, 440);
    }

    bool allowWindowResizing() override { return true; }
    juce::ComponentBoundsConstrainer* getBoundsConstrainer() override { return nullptr; }

    void resized() override {
        auto area = getLocalBounds().reduced(6);
        auto bottom = area.removeFromBottom(60);
        runBtn_.setBounds(bottom.removeFromTop(28).removeFromLeft(100));
        bottom.removeFromTop(4);
        status_.setBounds(bottom);
        editor_->setBounds(area);
    }

    bool keyPressed(const juce::KeyPress& key) override {
        if (key.getModifiers().isCtrlDown() && key.getKeyCode() == juce::KeyPress::returnKey) {
            run();
            return true;
        }
        return false;
    }

  private:
    void run() {
        auto err = plugin_.evaluate(document_.getAllContent());
        updateStatus();
    }

    void updateStatus() {
        auto err = plugin_.getLastError();
        if (err.isEmpty() && plugin_.hasActivePattern()) {
            status_.setColour(juce::Label::textColourId, juce::Colour(0xffa6e3a1));
            status_.setText("Pattern active", juce::dontSendNotification);
        } else if (err.isNotEmpty()) {
            status_.setColour(juce::Label::textColourId, juce::Colour(0xfff38ba8));
            status_.setText(err, juce::dontSendNotification);
        } else {
            status_.setColour(juce::Label::textColourId, juce::Colour(0xff89b4fa));
            status_.setText("Type a pattern and press Run", juce::dontSendNotification);
        }
    }

    StrudelPlugin& plugin_;
    juce::CodeDocument document_;
    std::unique_ptr<juce::CodeEditorComponent> editor_;
    juce::TextButton runBtn_;
    juce::Label status_;
};

std::unique_ptr<te::Plugin::EditorComponent> StrudelPlugin::createEditor() {
    return std::make_unique<StrudelEditorComponent>(*this);
}

}  // namespace magda::daw::audio
