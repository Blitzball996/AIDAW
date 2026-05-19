#pragma once

namespace aidaw {

class SelectionApi;
class AutomationApi;
class TrackApi;
class ClipApi;
class SessionApi;
class ProjectApi;
class UndoApi;
class MidiApi;
class TransportApi;

/**
 * Programmatic facade for AIDAW's DAW state.
 *
 * Composed of focused sub-interfaces, one per DAW concept. Consumers
 * (the agent layer today; Lua / controllers / CLI in future) take a
 * DawApi& and route every state read or mutation through these
 * accessors instead of reaching into singletons.
 */
class DawApi {
  public:
    virtual ~DawApi() = default;

    virtual SelectionApi& selection() = 0;
    virtual AutomationApi& automation() = 0;
    virtual TrackApi& tracks() = 0;
    virtual ClipApi& clips() = 0;
    virtual SessionApi& session() = 0;
    virtual ProjectApi& project() = 0;
    virtual UndoApi& undo() = 0;
    virtual MidiApi& midi() = 0;
    virtual TransportApi& transport() = 0;
};

}  // namespace aidaw
