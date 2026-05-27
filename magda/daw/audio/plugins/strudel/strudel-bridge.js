// strudel-bridge.js — Minimal bridge for QuickJS
// Strategy: No eval(). Pattern stays in JS. Only serialized events cross to C++.

// Cache pattern object (stays in JS, never serialized to C++)
var __currentPattern = null;
var __lastCode = "";
var __lastError = "";

// Compile a pattern from mini notation string
globalThis.__setPattern = function(code) {
  try {
    __lastError = "";
    __lastCode = code;
    __currentPattern = Strudel.mini(code);
    return "";
  } catch(e) {
    __lastError = String(e.message || e);
    __currentPattern = null;
    return __lastError;
  }
};

// Compile using controls.note (for note names like "c3 e3 g3")
globalThis.__setPatternNote = function(code) {
  try {
    __lastError = "";
    __lastCode = code;
    if (Strudel.controls && Strudel.controls.note) {
      __currentPattern = Strudel.controls.note(code);
    } else {
      __currentPattern = Strudel.mini(code);
    }
    return "";
  } catch(e) {
    __lastError = String(e.message || e);
    __currentPattern = null;
    return __lastError;
  }
};

// Query current pattern for events in a time range
// Returns JSON string: [{"note":60,"onset":0.0,"dur":0.25,"vel":1.0}, ...]
globalThis.__queryPattern = function(startCycle, endCycle) {
  if (!__currentPattern) return "[]";

  try {
    var haps = __currentPattern.queryArc(startCycle, endCycle);
    var events = [];

    for (var i = 0; i < haps.length; i++) {
      var h = haps[i];
      if (!h.hasOnset || !h.hasOnset()) continue;

      var val = h.value;
      var onset = h.whole.begin.valueOf();
      var dur = h.whole.end.valueOf() - onset;

      // Extract note value
      var note = 60;
      if (typeof val === "object" && val !== null) {
        if (val.note !== undefined) note = val.note;
        else if (val.n !== undefined) note = val.n;
        else if (val.freq !== undefined) note = val.freq;
      } else if (typeof val === "number") {
        note = val;
      } else if (typeof val === "string") {
        note = val;
      }

      // Extract velocity
      var vel = 1.0;
      if (typeof val === "object" && val !== null) {
        if (val.velocity !== undefined) vel = val.velocity;
        else if (val.gain !== undefined) vel = val.gain;
      }

      events.push({note: note, onset: onset, dur: dur, vel: vel});
    }

    return JSON.stringify(events);
  } catch(e) {
    return "[]";
  }
};

globalThis.__hasPattern = function() {
  return __currentPattern !== null;
};

globalThis.__getError = function() {
  return __lastError;
};
