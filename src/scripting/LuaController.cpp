#include "LuaController.hpp"

#include <juce_audio_devices/juce_audio_devices.h>

#include "AidawApiLuaBindings.hpp"
#include "LuaRuntime.hpp"

// TODO: Port MidiDeviceMatch from magda-core or provide AIDAW equivalent.
// For now, a simple string comparison stub is used.

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

#include <utility>

namespace aidaw::scripting {

namespace {

// Stub for MIDI device matching until the real implementation is ported.
bool midiDeviceMatches(const juce::String& filter, const juce::String& deviceId,
                       const juce::String& deviceName) {
    return filter.equalsIgnoreCase(deviceId) || filter.equalsIgnoreCase(deviceName);
}

const char* midiTypeName(const juce::MidiMessage& m) {
    if (m.isNoteOn())
        return "note_on";
    if (m.isNoteOff())
        return "note_off";
    if (m.isController())
        return "cc";
    if (m.isPitchWheel())
        return "pitch_bend";
    if (m.isChannelPressure() || m.isAftertouch())
        return "aftertouch";
    if (m.isProgramChange())
        return "program_change";
    if (m.isSysEx())
        return "sysex";
    return "other";
}

// Push a Lua table describing the MIDI event.
void pushMidiEvent(lua_State* L, const juce::String& deviceName, const juce::MidiMessage& m) {
    const bool sysex = m.isSysEx();
    lua_createtable(L, 0, sysex ? 6 : 5);

    lua_pushstring(L, midiTypeName(m));
    lua_setfield(L, -2, "type");

    lua_pushinteger(L, m.getChannel());
    lua_setfield(L, -2, "channel");

    int number = 0;
    int value = 0;
    if (m.isController()) {
        number = m.getControllerNumber();
        value = m.getControllerValue();
    } else if (m.isNoteOnOrOff()) {
        number = m.getNoteNumber();
        value = m.getVelocity();
    } else if (m.isPitchWheel()) {
        value = m.getPitchWheelValue() - 8192;
    } else if (m.isChannelPressure()) {
        value = m.getChannelPressureValue();
    } else if (m.isAftertouch()) {
        number = m.getNoteNumber();
        value = m.getAfterTouchValue();
    } else if (m.isProgramChange()) {
        number = m.getProgramChangeNumber();
    }

    lua_pushinteger(L, number);
    lua_setfield(L, -2, "number");

    lua_pushinteger(L, value);
    lua_setfield(L, -2, "value");

    if (sysex) {
        const auto* data = m.getSysExData();
        const int size = m.getSysExDataSize();
        lua_createtable(L, size, 0);
        for (int i = 0; i < size; ++i) {
            lua_pushinteger(L, static_cast<lua_Integer>(data[i]));
            lua_rawseti(L, -2, i + 1);
        }
        lua_setfield(L, -2, "bytes");
    }

    auto raw = deviceName.toRawUTF8();
    lua_pushlstring(L, raw, static_cast<size_t>(deviceName.getNumBytesAsUTF8()));
    lua_setfield(L, -2, "port");
}

// Tick rate for on_tick callbacks (~30 Hz).
constexpr int kTickIntervalMs = 33;

int luaMessageHandler(lua_State* L) {
    const char* msg = lua_tostring(L, 1);
    if (msg == nullptr) {
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
            return 1;
        msg = lua_pushfstring(L, "(error object is a %s value)", luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

}  // namespace

class LuaController::TickTimer : public juce::Timer {
  public:
    explicit TickTimer(LuaController& owner) : owner_(owner) {}
    void timerCallback() override {
        auto now = juce::Time::getMillisecondCounter();
        double dt = (now - owner_.lastTickMs_) / 1000.0;
        owner_.lastTickMs_ = now;
        owner_.callOnTick(dt);
    }

  private:
    LuaController& owner_;
};

LuaController::LuaController(AidawApi& api) : api_(api) {}

LuaController::~LuaController() {
    detach();
    unloadScript();
}

void LuaController::attach(MidiBridge& bridge) {
    if (bridge_ != nullptr)
        detach();
    bridge_ = &bridge;
    // TODO: bridge_->addRawMidiListener(this);
    setDawInputPort(dawInputPort_);
}

void LuaController::detach() {
    if (bridge_ != nullptr) {
        // TODO: bridge_->removeRawMidiListener(this);
        bridge_ = nullptr;
    }
}

bool LuaController::loadScript(const juce::File& file) {
    juce::Logger::writeToLog("[lua-debug] LuaController::loadScript ENTER file='" +
                             file.getFullPathName() + "' active='" + currentScriptName_ +
                             "' bridge=" + (bridge_ != nullptr ? "attached" : "null"));
    unloadScript();

    if (!file.existsAsFile()) {
        lastError_ = "Script file does not exist: " + file.getFullPathName();
        juce::Logger::writeToLog("[lua-debug] loadScript FAIL: file missing");
        return false;
    }

    auto rt = std::make_unique<LuaRuntime>();
    registerAidawApi(rt->state(), api_);

    if (!rt->evalFile(file)) {
        lastError_ = rt->lastError();
        juce::Logger::writeToLog("[lua-debug] loadScript FAIL: evalFile error: " + lastError_);
        return false;
    }

    rt_ = std::move(rt);
    currentScriptName_ = file.getFileName();
    lastError_ = {};

    juce::Logger::writeToLog("[lua-debug] loadScript: calling on_load");
    callOptionalNullary("on_load");

    const bool hasMessageManager = juce::MessageManager::getInstanceWithoutCreating() != nullptr;
    if (hasMessageManager) {
        if (!tickTimer_)
            tickTimer_ = std::make_unique<TickTimer>(*this);
        lastTickMs_ = juce::Time::getMillisecondCounter();
        tickTimer_->startTimer(kTickIntervalMs);
    }
    juce::Logger::writeToLog("[lua-debug] loadScript OK script='" + currentScriptName_ + "'");
    return true;
}

void LuaController::unloadScript() {
    if (tickTimer_) {
        tickTimer_->stopTimer();
        tickTimer_.reset();
    }
    if (rt_ != nullptr) {
        callOptionalNullary("on_unload");
    }
    rt_.reset();
    currentScriptName_ = {};
    lastError_ = {};
}

juce::String LuaController::currentScriptName() const {
    return currentScriptName_;
}

void LuaController::setDawInputPort(const juce::String& port) {
    dawInputPort_ = port;
    if (bridge_ == nullptr)
        return;
    // TODO: Enable MIDI inputs on bridge based on port selection.
    // if (dawInputPort_.isEmpty()) {
    //     for (const auto& dev : juce::MidiInput::getAvailableDevices())
    //         bridge_->enableMidiInput(dev.identifier);
    // } else {
    //     bridge_->enableMidiInput(dawInputPort_);
    // }
}

juce::String LuaController::lastError() const {
    return lastError_;
}

void LuaController::onRawMidi(const juce::String& deviceId, const juce::String& deviceName,
                              const juce::MidiMessage& msg) {
    if (dawInputPort_.isNotEmpty() && !midiDeviceMatches(dawInputPort_, deviceId, deviceName))
        return;

    // MIDI thread. Capture POD copies and bounce to the message thread.
    juce::WeakReference<LuaController> weak(this);
    juce::MidiMessage msgCopy(msg);
    juce::String nameCopy(deviceName);

    juce::MessageManager::callAsync(
        [weak, msgCopy = std::move(msgCopy), nameCopy = std::move(nameCopy)]() mutable {
            if (auto* self = weak.get()) {
                self->dispatchToLua(nameCopy, msgCopy);
            }
        });
}

void LuaController::dispatchEventForTest(const juce::String& deviceName,
                                         const juce::MidiMessage& msg) {
    dispatchToLua(deviceName, msg);
}

void LuaController::tickForTest(double dt) {
    callOnTick(dt);
}

void LuaController::callOptionalNullary(const char* name) {
    if (rt_ == nullptr)
        return;
    lua_State* L = rt_->state();
    if (L == nullptr)
        return;

    lua_getglobal(L, name);
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        lua_pop(L, 1);
        return;
    }

    int fnIdx = lua_gettop(L);
    lua_pushcfunction(L, luaMessageHandler);
    int msgHandlerIdx = fnIdx;
    lua_insert(L, msgHandlerIdx);

    int rc = lua_pcall(L, 0, 0, msgHandlerIdx);
    lua_remove(L, msgHandlerIdx);

    if (rc != LUA_OK) {
        size_t len = 0;
        const char* err = lua_tolstring(L, -1, &len);
        if (err != nullptr) {
            juce::Logger::writeToLog(juce::String("[lua ") + name + " error] " +
                                     juce::String::fromUTF8(err, static_cast<int>(len)));
        }
        lua_pop(L, 1);
    }
}

void LuaController::callOnTick(double dt) {
    if (rt_ == nullptr)
        return;
    lua_State* L = rt_->state();
    if (L == nullptr)
        return;

    lua_getglobal(L, "on_tick");
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        lua_pop(L, 1);
        return;
    }

    int fnIdx = lua_gettop(L);
    lua_pushcfunction(L, luaMessageHandler);
    int msgHandlerIdx = fnIdx;
    lua_insert(L, msgHandlerIdx);
    lua_pushnumber(L, dt);

    int rc = lua_pcall(L, 1, 0, msgHandlerIdx);
    lua_remove(L, msgHandlerIdx);

    if (rc != LUA_OK) {
        size_t len = 0;
        const char* err = lua_tolstring(L, -1, &len);
        if (err != nullptr) {
            juce::Logger::writeToLog("[lua on_tick error] " +
                                     juce::String::fromUTF8(err, static_cast<int>(len)));
        }
        lua_pop(L, 1);
    }
}

void LuaController::dispatchToLua(const juce::String& deviceName, const juce::MidiMessage& msg) {
    if (rt_ == nullptr)
        return;

    lua_State* L = rt_->state();
    if (L == nullptr)
        return;

    lua_getglobal(L, "on_midi");
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        lua_pop(L, 1);
        return;
    }

    int handlerIdx = lua_gettop(L);
    pushMidiEvent(L, deviceName, msg);

    lua_pushcfunction(L, luaMessageHandler);
    int msgHandlerIdx = handlerIdx;
    lua_insert(L, msgHandlerIdx);

    int rc = lua_pcall(L, 1, 0, msgHandlerIdx);
    lua_remove(L, msgHandlerIdx);

    if (rc != LUA_OK) {
        size_t len = 0;
        const char* err = lua_tolstring(L, -1, &len);
        if (err != nullptr) {
            juce::Logger::writeToLog("[lua on_midi error] " +
                                     juce::String::fromUTF8(err, static_cast<int>(len)));
        }
        lua_pop(L, 1);
    }
}

}  // namespace aidaw::scripting
