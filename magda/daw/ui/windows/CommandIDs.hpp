#pragma once

namespace magda {
namespace CommandIDs {

enum {
    // File menu
    newProject = 0x0001,
    openProject = 0x0002,
    saveProject = 0x0003,
    saveProjectAs = 0x0004,
    exportAudio = 0x0005,

    // Edit menu
    undo = 0x1000,
    redo = 0x1001,
    cut = 0x1002,
    copy = 0x1003,
    paste = 0x1004,
    duplicate = 0x1005,
    deleteCmd = 0x1006,
    selectAll = 0x1007,
    splitOrTrim = 0x1008,
    joinClips = 0x1009,
    renderClip = 0x100A,
    renderTimeSelection = 0x100B,
    setLoopFromClip = 0x100C,
    toggleClipLoop = 0x100D,
    quantize = 0x100E,
    stripSilence = 0x100F,
    normalizeAudio = 0x1010,
    reverseAudio = 0x1011,

    // Transport menu
    play = 0x2000,
    stop = 0x2001,
    record = 0x2002,
    goToStart = 0x2003,
    goToEnd = 0x2004,
    toggleMetronome = 0x2005,
    toggleLoop = 0x2006,

    // Track menu
    newAudioTrack = 0x3000,
    newMidiTrack = 0x3001,
    deleteTrack = 0x3002,
    addVCAFader = 0x3003,
    createRouteGroup = 0x3004,

    // View menu
    zoom = 0x4000,
    toggleArrangeSession = 0x4001,
    uiScaleUp = 0x4002,
    uiScaleDown = 0x4003,
    showBeatBox = 0x4004,
    showClipBrowser = 0x4005,
    showVirtualKeyboard = 0x4006,
    showExportDialog = 0x4007,

    // Help menu
    showHelp = 0x5000,
    about = 0x5001
};

}  // namespace CommandIDs
}  // namespace magda
