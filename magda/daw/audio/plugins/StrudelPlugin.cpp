#include "StrudelPlugin.hpp"

#include <juce_gui_extra/juce_gui_extra.h>

#include <cmath>
#include <cstring>

namespace magda::daw::audio {

const char* StrudelPlugin::xmlTypeName = "strudel";

//==============================================================================
// Plugin
//==============================================================================

StrudelPlugin::StrudelPlugin(const te::PluginCreationInfo& info) : te::Plugin(info) {
    auto um = getUndoManager();
    codeValue.referTo(state, juce::Identifier("code"), um,
                      "// Tidal (Strudel) — write a pattern, then press Play\n"
                      "note(\"c3 e3 g3 b3\").sound(\"sawtooth\").lpf(800)");
    fifoL_.assign(kFifoCapacity, 0.0f);
    fifoR_.assign(kFifoCapacity, 0.0f);
}

StrudelPlugin::~StrudelPlugin() = default;

void StrudelPlugin::initialise(const te::PluginInitialisationInfo& info) {
    sampleRate_ = info.sampleRate;
    // NOTE: deliberately does NOT evaluate the stored code here. The editor opens
    // silent; sound only begins when the user presses Play in the REPL.
    clearFifo();
}

void StrudelPlugin::deinitialise() {}

void StrudelPlugin::reset() { clearFifo(); }

void StrudelPlugin::clearFifo() {
    writePos_.store(0, std::memory_order_relaxed);
    readPos_.store(0, std::memory_order_relaxed);
    resamplePhase_ = 0.0;
    prevL_ = prevR_ = 0.0f;
    havePrev_ = false;
}

void StrudelPlugin::restorePluginStateFromValueTree(const juce::ValueTree& v) {
    if (v.hasProperty("code"))
        codeValue = v.getProperty("code").toString();
}

//==============================================================================
// PCM capture (message thread) -> resample -> lock-free FIFO
//==============================================================================

void StrudelPlugin::pushCapturedAudio(const float* left, const float* right, int numFrames,
                                      double srcSampleRate) {
    if (numFrames <= 0 || srcSampleRate <= 0.0 || sampleRate_ <= 0.0 || left == nullptr ||
        right == nullptr)
        return;

    const double step = srcSampleRate / sampleRate_;  // input samples per output sample

    // sampleAt(i): i == -1 refers to the carried previous sample from the last chunk.
    auto sampleAt = [](const float* buf, int i, float prev) -> float {
        return (i < 0) ? prev : buf[i];
    };

    int w = writePos_.load(std::memory_order_relaxed);
    const int r = readPos_.load(std::memory_order_acquire);

    double pos = havePrev_ ? resamplePhase_ : 0.0;

    // Produce output samples at pos, pos+step, ... up to the last available point.
    while (pos <= (double)(numFrames - 1) + 1e-9) {
        const int i0 = (int)std::floor(pos);
        const float frac = (float)(pos - i0);

        const float l0 = sampleAt(left, i0, prevL_);
        const float l1 = sampleAt(left, i0 + 1, prevL_);
        const float r0 = sampleAt(right, i0, prevR_);
        const float r1 = sampleAt(right, i0 + 1, prevR_);

        const float outL = l0 + (l1 - l0) * frac;
        const float outR = r0 + (r1 - r0) * frac;

        // Drop new samples if the consumer is too far behind (never clobber unread data).
        if (w - r < kFifoCapacity - 1) {
            const int idx = w & (kFifoCapacity - 1);
            fifoL_[(size_t)idx] = outL;
            fifoR_[(size_t)idx] = outR;
            ++w;
        }
        pos += step;
    }

    writePos_.store(w, std::memory_order_release);

    // Carry: shift the origin so the next chunk's sample[0] aligns with index numFrames.
    resamplePhase_ = pos - numFrames;
    prevL_ = left[numFrames - 1];
    prevR_ = right[numFrames - 1];
    havePrev_ = true;
}

//==============================================================================
// Audio thread: drain FIFO into the track buffer
//==============================================================================

void StrudelPlugin::applyToBuffer(const te::PluginRenderContext& rc) {
    if (rc.destBuffer == nullptr)
        return;

    const int numSamples = rc.bufferNumSamples;
    const int startSample = rc.bufferStartSample;
    const int numChannels = rc.destBuffer->getNumChannels();

    int r = readPos_.load(std::memory_order_relaxed);
    const int w = writePos_.load(std::memory_order_acquire);
    const int avail = w - r;

    const int toCopy = juce::jlimit(0, numSamples, avail);

    for (int i = 0; i < toCopy; ++i) {
        const int idx = r & (kFifoCapacity - 1);
        const float l = fifoL_[(size_t)idx];
        const float ri = fifoR_[(size_t)idx];
        ++r;
        if (numChannels >= 1) rc.destBuffer->addSample(0, startSample + i, l);
        if (numChannels >= 2) rc.destBuffer->addSample(1, startSample + i, ri);
    }

    readPos_.store(r, std::memory_order_release);
    // Underflow (toCopy < numSamples) leaves the remainder of the block as silence.
}

//==============================================================================
// Editor — hosts the real Strudel REPL in a WebView2
//==============================================================================

#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE

class StrudelEditorComponent : public te::Plugin::EditorComponent {
  public:
    explicit StrudelEditorComponent(StrudelPlugin& p) : plugin_(p) {
        using WB = juce::WebBrowserComponent;

        auto strudelDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                              .getParentDirectory()
                              .getChildFile("strudel");

        auto options =
            WB::Options{}
                .withBackend(WB::Options::Backend::webview2)
                .withWinWebView2Options(
                    WB::Options::WinWebView2{}
                        .withUserDataFolder(
                            juce::File::getSpecialLocation(juce::File::tempDirectory))
                        .withBackgroundColour(juce::Colour(0xff1e1e2e)))
                .withNativeIntegrationEnabled(true)
                .withResourceProvider(
                    [strudelDir](const juce::String& path)
                        -> std::optional<WB::Resource> {
                        return provideResource(strudelDir, path);
                    })
                .withNativeFunction(
                    juce::Identifier("jucePcmChunk"),
                    [this](const juce::Array<juce::var>& args,
                           WB::NativeFunctionCompletion completion) {
                        handlePcmChunk(args);
                        completion(juce::var());
                    })
                .withEventListener(juce::Identifier("strudelReady"),
                                   [this](juce::var) { onStrudelReady(); })
                .withEventListener(juce::Identifier("strudelCode"),
                                   [this](juce::var payload) {
                                       if (auto* o = payload.getDynamicObject())
                                           plugin_.setCode(o->getProperty("code").toString());
                                   });

        web_ = std::make_unique<WB>(options);
        addAndMakeVisible(*web_);
        web_->goToURL(WB::getResourceProviderRoot());

        setSize(760, 520);
    }

    bool allowWindowResizing() override { return true; }
    juce::ComponentBoundsConstrainer* getBoundsConstrainer() override { return nullptr; }

    void resized() override {
        if (web_)
            web_->setBounds(getLocalBounds());
    }

  private:
    // Serve index.html / bundles / samples from the on-disk strudel resource dir.
    static std::optional<juce::WebBrowserComponent::Resource>
    provideResource(const juce::File& root, const juce::String& path) {
        using WB = juce::WebBrowserComponent;
        if (path.contains(".."))
            return std::nullopt;

        juce::File file = (path == "/")
                              ? root.getChildFile("index.html")
                              : root.getChildFile(path.trimCharactersAtStart("/"));
        if (!file.existsAsFile())
            return std::nullopt;

        juce::MemoryBlock mb;
        if (!file.loadFileAsData(mb))
            return std::nullopt;

        std::vector<std::byte> data((size_t)mb.getSize());
        if (mb.getSize() > 0)
            std::memcpy(data.data(), mb.getData(), mb.getSize());

        const auto ext = file.getFileExtension().toLowerCase();
        juce::String mime = "application/octet-stream";
        if (ext == ".html") mime = "text/html";
        else if (ext == ".js" || ext == ".mjs") mime = "text/javascript";
        else if (ext == ".css") mime = "text/css";
        else if (ext == ".json") mime = "application/json";
        else if (ext == ".wav") mime = "audio/wav";
        else if (ext == ".mp3") mime = "audio/mpeg";
        else if (ext == ".ogg") mime = "audio/ogg";

        return WB::Resource{std::move(data), mime};
    }

    void onStrudelReady() {
        // Push the persisted pattern text into the REPL editor (does NOT play).
        auto code = plugin_.getCode();
        auto js = "window.strudelSetCode(" + code.quoted() + ");";
        if (web_)
            web_->evaluateJavascript(js, nullptr);
    }

    void handlePcmChunk(const juce::Array<juce::var>& args) {
        if (args.size() < 3)
            return;
        const auto* lArr = args[0].getArray();
        const auto* rArr = args[1].getArray();
        const double srcRate = (double)args[2];
        if (lArr == nullptr || rArr == nullptr)
            return;

        const int n = juce::jmin(lArr->size(), rArr->size());
        if (n <= 0)
            return;

        scratchL_.resize((size_t)n);
        scratchR_.resize((size_t)n);
        for (int i = 0; i < n; ++i) {
            scratchL_[(size_t)i] = (float)(double)lArr->getReference(i);
            scratchR_[(size_t)i] = (float)(double)rArr->getReference(i);
        }
        plugin_.pushCapturedAudio(scratchL_.data(), scratchR_.data(), n, srcRate);
    }

    StrudelPlugin& plugin_;
    std::unique_ptr<juce::WebBrowserComponent> web_;
    std::vector<float> scratchL_, scratchR_;
};

#else  // !JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE

// Fallback editor when the WebView2 SDK is unavailable at build time. The plugin
// still loads (and audio routing works once a WebView build is supplied), but the
// REPL UI requires the WebView2 backend.
class StrudelEditorComponent : public te::Plugin::EditorComponent {
  public:
    explicit StrudelEditorComponent(StrudelPlugin&) {
        label_.setJustificationType(juce::Justification::centred);
        label_.setText("Tidal (Strudel) requires the WebView2 backend.\n\n"
                       "Install the NuGet package \"Microsoft.Web.WebView2\" and rebuild "
                       "with JUCE_USE_WIN_WEBVIEW2 enabled.",
                       juce::dontSendNotification);
        addAndMakeVisible(label_);
        setSize(560, 220);
    }
    bool allowWindowResizing() override { return true; }
    juce::ComponentBoundsConstrainer* getBoundsConstrainer() override { return nullptr; }
    void resized() override { label_.setBounds(getLocalBounds().reduced(20)); }

  private:
    juce::Label label_;
};

#endif  // JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE

std::unique_ptr<te::Plugin::EditorComponent> StrudelPlugin::createEditor() {
    return std::make_unique<StrudelEditorComponent>(*this);
}

}  // namespace magda::daw::audio
