#include "AidawApiLuaBindings.hpp"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

// TODO: These includes reference the actual AIDAW API layer once ported.
// For now, the AidawApi class is a forward-declared stub. The binding
// functions below are structured identically to the magda-core originals
// but will need the real API types wired in.
//
// Original magda-core dependencies that need AIDAW equivalents:
//   - clip_api.hpp, focused_api.hpp, magda_api.hpp, midi_api.hpp
//   - project_api.hpp, selection_api.hpp, session_api.hpp
//   - track_api.hpp, transport_api.hpp
//   - ClipInfo.hpp, ClipTypes.hpp, SessionViewState.hpp
//   - TrackInfo.hpp, TrackTypes.hpp, TypeIds.hpp
//   - ProjectInfo.hpp, version.hpp

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

#include <unordered_set>
#include <vector>

namespace aidaw::scripting {

namespace {

// Address of this static is used as a unique key into LUA_REGISTRYINDEX.
char kAidawApiRegistryKey = 0;

AidawApi* getApi(lua_State* L) {
    lua_pushlightuserdata(L, &kAidawApiRegistryKey);
    lua_rawget(L, LUA_REGISTRYINDEX);
    auto* api = static_cast<AidawApi*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    if (api == nullptr) {
        luaL_error(L, "AidawApi has not been registered with this Lua state");
    }
    return api;
}

// Push a `juce::String` as a Lua string (UTF-8).
void pushJuceString(lua_State* L, const juce::String& s) {
    auto raw = s.toRawUTF8();
    lua_pushlstring(L, raw, static_cast<size_t>(s.getNumBytesAsUTF8()));
}

// ---- log -------------------------------------------------------------------

juce::String buildLogMessage(lua_State* L) {
    int n = lua_gettop(L);
    juce::String msg;
    for (int i = 1; i <= n; ++i) {
        size_t len = 0;
        const char* s = luaL_tolstring(L, i, &len);
        if (i > 1)
            msg << ' ';
        msg << juce::String::fromUTF8(s, static_cast<int>(len));
        lua_pop(L, 1);
    }
    return msg;
}

int lua_log_info(lua_State* L) {
    juce::Logger::writeToLog("[lua] " + buildLogMessage(L));
    return 0;
}

int lua_log_warn(lua_State* L) {
    juce::Logger::writeToLog("[lua warn] " + buildLogMessage(L));
    return 0;
}

int lua_log_error(lua_State* L) {
    juce::Logger::writeToLog("[lua error] " + buildLogMessage(L));
    return 0;
}

// ---- selection -------------------------------------------------------------
// TODO: Wire these to the real AidawApi::selection() once ported.

int lua_selection_track(lua_State* L) {
    // auto* api = getApi(L);
    // TrackId id = api->selection().getSelectedTrack();
    (void)getApi(L);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_selection_clip(lua_State* L) {
    (void)getApi(L);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_selection_clips(lua_State* L) {
    (void)getApi(L);
    lua_newtable(L);  // stub: empty array
    return 1;
}

int lua_selection_has_notes(lua_State* L) {
    (void)getApi(L);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_selection_note_clip(lua_State* L) {
    (void)getApi(L);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_selection_note_indices(lua_State* L) {
    (void)getApi(L);
    lua_newtable(L);  // stub
    return 1;
}

int lua_selection_select_track(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);  // validate arg
    // TODO: api->selection().selectTrack(trackId);
    return 0;
}

int lua_selection_select_tracks(lua_State* L) {
    (void)getApi(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    // TODO: api->selection().selectTracks(ids);
    return 0;
}

int lua_selection_select_clip(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    // TODO: api->selection().selectClip(clipId);
    return 0;
}

int lua_selection_select_clips(lua_State* L) {
    (void)getApi(L);
    luaL_checktype(L, 1, LUA_TTABLE);
    // TODO: api->selection().selectClips(ids);
    return 0;
}

int lua_selection_clear_notes(lua_State* L) {
    (void)getApi(L);
    // TODO: api->selection().clearNoteSelection();
    return 0;
}

// ---- tracks ----------------------------------------------------------------
// TODO: Wire these to the real AidawApi::tracks() once ported.

int lua_tracks_create(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    // TODO: create track and return id
    lua_pushinteger(L, 0);  // stub
    return 1;
}

int lua_tracks_delete(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    // TODO: api->tracks().deleteTrack(id);
    return 0;
}

int lua_tracks_count(lua_State* L) {
    (void)getApi(L);
    lua_pushinteger(L, 0);  // stub
    return 1;
}

int lua_tracks_list(lua_State* L) {
    (void)getApi(L);
    lua_newtable(L);  // stub: empty array
    return 1;
}

int lua_tracks_get(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_tracks_set_name(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checkstring(L, 2);
    return 0;
}

int lua_tracks_set_volume(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checknumber(L, 2);
    return 0;
}

int lua_tracks_set_pan(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checknumber(L, 2);
    return 0;
}

int lua_tracks_set_muted(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    return 0;
}

int lua_tracks_set_soloed(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);
    return 0;
}

// ---- clips -----------------------------------------------------------------
// TODO: Wire these to the real AidawApi::clips() once ported.

int lua_clips_create_midi(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checknumber(L, 2);
    luaL_checknumber(L, 3);
    lua_pushinteger(L, 0);  // stub
    return 1;
}

int lua_clips_delete(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

int lua_clips_list_on_track(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_newtable(L);  // stub
    return 1;
}

int lua_clips_list_arrangement(lua_State* L) {
    (void)getApi(L);
    lua_newtable(L);  // stub
    return 1;
}

int lua_clips_set_name(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checkstring(L, 2);
    return 0;
}

int lua_clips_set_groove(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checkstring(L, 2);
    return 0;
}

int lua_clips_colour(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushnil(L);  // stub
    return 1;
}

// ---- session ---------------------------------------------------------------
// TODO: Wire these to the real AidawApi::session() once ported.

int lua_session_launch_clip(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

int lua_session_stop_clip(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

int lua_session_stop_track(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

int lua_session_stop_all(lua_State* L) {
    (void)getApi(L);
    return 0;
}

int lua_session_launch_scene(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

int lua_session_active_clip_on_track(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_session_clip_in_slot(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checkinteger(L, 2);
    lua_pushnil(L);  // stub
    return 1;
}

int lua_session_clip_play_state(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushstring(L, "stopped");  // stub
    return 1;
}

int lua_session_set_view(lua_State* L) {
    luaL_checkinteger(L, 1);
    // TODO: SessionViewState::getInstance().setControllerSceneWindow(...)
    return 0;
}

// ---- project ---------------------------------------------------------------

int lua_project_info(lua_State* L) {
    (void)getApi(L);
    // Stub: return a table with placeholder values.
    lua_createtable(L, 0, 7);

    lua_pushstring(L, "Untitled");
    lua_setfield(L, -2, "name");

    lua_pushstring(L, "");
    lua_setfield(L, -2, "file_path");

    lua_pushnumber(L, 120.0);
    lua_setfield(L, -2, "tempo");

    lua_pushinteger(L, 4);
    lua_setfield(L, -2, "time_sig_num");

    lua_pushinteger(L, 4);
    lua_setfield(L, -2, "time_sig_den");

    lua_pushnumber(L, 44100.0);
    lua_setfield(L, -2, "sample_rate");

    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "loop_enabled");

    return 1;
}

// ---- app -------------------------------------------------------------------

#ifndef AIDAW_VERSION
#define AIDAW_VERSION "0.1.0-dev"
#endif

int lua_app_version(lua_State* L) {
    lua_pushstring(L, AIDAW_VERSION);
    return 1;
}

// ---- midi ------------------------------------------------------------------
// TODO: Wire these to the real AidawApi::midi() once ported.

int lua_midi_send_cc(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);
    luaL_checkinteger(L, 4);
    lua_pushboolean(L, 0);  // stub: not sent
    return 1;
}

int lua_midi_send_note_on(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);
    luaL_checkinteger(L, 4);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_midi_send_note_off(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_midi_send(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_midi_send_sysex(lua_State* L) {
    (void)getApi(L);
    luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_midi_outputs(lua_State* L) {
    (void)getApi(L);
    lua_newtable(L);  // stub: empty list
    return 1;
}

int lua_midi_default_output(lua_State* L) {
    (void)getApi(L);
    lua_pushstring(L, "");  // stub
    return 1;
}

// ---- transport -------------------------------------------------------------
// TODO: Wire these to the real AidawApi::transport() once ported.

int lua_transport_play(lua_State* L) {
    (void)getApi(L);
    return 0;
}

int lua_transport_stop(lua_State* L) {
    (void)getApi(L);
    return 0;
}

int lua_transport_set_recording(lua_State* L) {
    (void)getApi(L);
    luaL_checktype(L, 1, LUA_TBOOLEAN);
    return 0;
}

int lua_transport_is_playing(lua_State* L) {
    (void)getApi(L);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_transport_is_recording(lua_State* L) {
    (void)getApi(L);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_transport_is_loop_enabled(lua_State* L) {
    (void)getApi(L);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_transport_set_loop_enabled(lua_State* L) {
    (void)getApi(L);
    luaL_checktype(L, 1, LUA_TBOOLEAN);
    return 0;
}

int lua_transport_position_beats(lua_State* L) {
    (void)getApi(L);
    lua_pushnumber(L, 0.0);  // stub
    return 1;
}

int lua_transport_set_position_beats(lua_State* L) {
    (void)getApi(L);
    luaL_checknumber(L, 1);
    return 0;
}

// ---- focused ---------------------------------------------------------------
// TODO: Wire these to the real AidawApi::focused() once ported.

int lua_focused_has_focus(lua_State* L) {
    (void)getApi(L);
    lua_pushboolean(L, 0);  // stub
    return 1;
}

int lua_focused_name(lua_State* L) {
    (void)getApi(L);
    lua_pushstring(L, "");  // stub
    return 1;
}

int lua_focused_macro_name(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushstring(L, "");  // stub
    return 1;
}

int lua_focused_macro_value(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    lua_pushnumber(L, 0.0);  // stub
    return 1;
}

int lua_focused_set_macro(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    luaL_checknumber(L, 2);
    return 0;
}

int lua_focused_auto_map(lua_State* L) {
    (void)getApi(L);
    return 0;
}

int lua_focused_clear_auto_map(lua_State* L) {
    (void)getApi(L);
    return 0;
}

int lua_focused_cycle_device(lua_State* L) {
    (void)getApi(L);
    luaL_checkinteger(L, 1);
    return 0;
}

// ---- registration ----------------------------------------------------------

struct FnReg {
    const char* name;
    lua_CFunction fn;
};

void setFunctions(lua_State* L, const FnReg* funcs) {
    for (const FnReg* f = funcs; f->name != nullptr; ++f) {
        lua_pushcfunction(L, f->fn);
        lua_setfield(L, -2, f->name);
    }
}

const FnReg kLogFns[] = {
    {"info", lua_log_info},
    {"warn", lua_log_warn},
    {"error", lua_log_error},
    {nullptr, nullptr},
};

const FnReg kSelectionFns[] = {
    {"track", lua_selection_track},
    {"clip", lua_selection_clip},
    {"clips", lua_selection_clips},
    {"has_notes", lua_selection_has_notes},
    {"note_clip", lua_selection_note_clip},
    {"note_indices", lua_selection_note_indices},
    {"select_track", lua_selection_select_track},
    {"select_tracks", lua_selection_select_tracks},
    {"select_clip", lua_selection_select_clip},
    {"select_clips", lua_selection_select_clips},
    {"clear_notes", lua_selection_clear_notes},
    {nullptr, nullptr},
};

const FnReg kTrackFns[] = {
    {"create", lua_tracks_create},
    {"delete", lua_tracks_delete},
    {"count", lua_tracks_count},
    {"list", lua_tracks_list},
    {"get", lua_tracks_get},
    {"set_name", lua_tracks_set_name},
    {"set_volume", lua_tracks_set_volume},
    {"set_pan", lua_tracks_set_pan},
    {"set_muted", lua_tracks_set_muted},
    {"set_soloed", lua_tracks_set_soloed},
    {nullptr, nullptr},
};

const FnReg kClipFns[] = {
    {"create_midi", lua_clips_create_midi},
    {"delete", lua_clips_delete},
    {"list_on_track", lua_clips_list_on_track},
    {"list_arrangement", lua_clips_list_arrangement},
    {"set_name", lua_clips_set_name},
    {"set_groove", lua_clips_set_groove},
    {"colour", lua_clips_colour},
    {nullptr, nullptr},
};

const FnReg kSessionFns[] = {
    {"launch_clip", lua_session_launch_clip},
    {"stop_clip", lua_session_stop_clip},
    {"stop_track", lua_session_stop_track},
    {"stop_all", lua_session_stop_all},
    {"launch_scene", lua_session_launch_scene},
    {"active_clip_on_track", lua_session_active_clip_on_track},
    {"clip_in_slot", lua_session_clip_in_slot},
    {"clip_play_state", lua_session_clip_play_state},
    {"set_view", lua_session_set_view},
    {nullptr, nullptr},
};

const FnReg kProjectFns[] = {
    {"info", lua_project_info},
    {nullptr, nullptr},
};

const FnReg kAppFns[] = {
    {"version", lua_app_version},
    {nullptr, nullptr},
};

const FnReg kMidiFns[] = {
    {"send", lua_midi_send},
    {"send_cc", lua_midi_send_cc},
    {"send_note_on", lua_midi_send_note_on},
    {"send_note_off", lua_midi_send_note_off},
    {"send_sysex", lua_midi_send_sysex},
    {"outputs", lua_midi_outputs},
    {"default_output", lua_midi_default_output},
    {nullptr, nullptr},
};

const FnReg kFocusedFns[] = {
    {"has_focus", lua_focused_has_focus},
    {"name", lua_focused_name},
    {"macro_name", lua_focused_macro_name},
    {"macro_value", lua_focused_macro_value},
    {"set_macro", lua_focused_set_macro},
    {"auto_map", lua_focused_auto_map},
    {"clear_auto_map", lua_focused_clear_auto_map},
    {"cycle_device", lua_focused_cycle_device},
    {nullptr, nullptr},
};

const FnReg kTransportFns[] = {
    {"play", lua_transport_play},
    {"stop", lua_transport_stop},
    {"set_recording", lua_transport_set_recording},
    {"is_playing", lua_transport_is_playing},
    {"is_recording", lua_transport_is_recording},
    {"is_loop_enabled", lua_transport_is_loop_enabled},
    {"set_loop_enabled", lua_transport_set_loop_enabled},
    {"position_beats", lua_transport_position_beats},
    {"set_position_beats", lua_transport_set_position_beats},
    {nullptr, nullptr},
};

void installSubtable(lua_State* L, const char* name, const FnReg* fns) {
    lua_newtable(L);
    setFunctions(L, fns);
    lua_setfield(L, -2, name);
}

}  // namespace

void registerAidawApi(lua_State* L, AidawApi& api) {
    // Stash the api pointer in the registry under our static-address key.
    lua_pushlightuserdata(L, &kAidawApiRegistryKey);
    lua_pushlightuserdata(L, &api);
    lua_rawset(L, LUA_REGISTRYINDEX);

    // Build the aidaw namespace table.
    lua_newtable(L);
    installSubtable(L, "log", kLogFns);
    installSubtable(L, "selection", kSelectionFns);
    installSubtable(L, "tracks", kTrackFns);
    installSubtable(L, "clips", kClipFns);
    installSubtable(L, "session", kSessionFns);
    installSubtable(L, "project", kProjectFns);
    installSubtable(L, "app", kAppFns);
    installSubtable(L, "midi", kMidiFns);
    installSubtable(L, "transport", kTransportFns);
    installSubtable(L, "focused", kFocusedFns);
    lua_setglobal(L, "aidaw");
}

}  // namespace aidaw::scripting
