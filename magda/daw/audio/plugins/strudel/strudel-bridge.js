// strudel-bridge.js — Minimal bridge for QuickJS
// NO IIFE wrapper — everything is global so evaluateExpression can access it

var __currentPattern = null;
var __lastError = "";

// Expose Strudel functions as globals
var mini = Strudel.mini;
var pure = Strudel.pure;
var stack = Strudel.stack;
var cat = Strudel.cat;
var seq = Strudel.seq;
var sequence = Strudel.sequence;
var silence = Strudel.silence;
var Pattern = Strudel.Pattern;
var Fraction = Strudel.Fraction;

// Expose all controls as globals (note, n, sound, s, etc.)
if (Strudel.controls) {
  var __ctrlKeys = Object.keys(Strudel.controls);
  for (var __i = 0; __i < __ctrlKeys.length; __i++) {
    var __k = __ctrlKeys[__i];
    try { eval("var " + __k + " = Strudel.controls['" + __k + "']"); } catch(e) {}
  }
}

// Set pattern from code string (called from C++ via evaluateExpression)
function __setPattern(code) {
  try {
    __lastError = "";
    __currentPattern = Strudel.mini(code);
    return "";
  } catch(e) {
    __lastError = String(e.message || e);
    __currentPattern = null;
    return __lastError;
  }
}

// Query current pattern for events in a time range
// Returns JSON string
function __queryPattern(startCycle, endCycle) {
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
      var note = 60;
      if (typeof val === "object" && val !== null) {
        if (val.note !== undefined) note = val.note;
        else if (val.n !== undefined) note = val.n;
      } else if (typeof val === "number") {
        note = val;
      } else if (typeof val === "string") {
        note = val;
      }
      var vel = 1.0;
      if (typeof val === "object" && val !== null) {
        if (val.velocity !== undefined) vel = val.velocity;
        else if (val.gain !== undefined) vel = val.gain;
      }
      events.push({note:note, onset:onset, dur:dur, vel:vel});
    }
    return JSON.stringify(events);
  } catch(e) {
    return "[]";
  }
}

function __hasPattern() { return __currentPattern !== null; }
function __getError() { return __lastError; }
