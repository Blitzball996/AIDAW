#pragma once

extern "C" {
struct lua_State;
}

namespace aidaw {

// Stub: replace with actual AIDAW API class once ported.
class AidawApi;

namespace scripting {

/**
 * Install the `aidaw.*` namespace into `L`, wired to `api`.
 *
 * After this call, Lua scripts can use:
 *   aidaw.log.{info,warn,error}      — juce::Logger pass-through
 *   aidaw.selection.{...}            — SelectionApi
 *   aidaw.tracks.{...}               — TrackApi
 *   aidaw.clips.{...}                — ClipApi
 *   aidaw.project.info()             — ProjectApi snapshot
 *
 * The AidawApi reference is borrowed — its lifetime must outlive `L`. The
 * caller (e.g. the live app or the test harness) is responsible for that
 * ordering.
 *
 * Threading: every binding routes through AidawApi, which is message-thread
 * only. The caller must guarantee Lua execution happens on the message thread.
 *
 * Idempotent: calling twice on the same state replaces the registered
 * AidawApi pointer and re-creates the `aidaw` table.
 */
void registerAidawApi(lua_State* L, AidawApi& api);

}  // namespace scripting
}  // namespace aidaw
