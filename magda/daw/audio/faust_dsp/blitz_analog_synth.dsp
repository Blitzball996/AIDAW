declare name "Analog Warmth";
declare author "Blitz";
declare description "Analog-style saturation and warmth with harmonics";
declare category "Effect";

import("stdfaust.lib");

drive = hslider("[0]drive[style:knob]", 0.3, 0, 1, 0.01) : si.smoo;
warmth = hslider("[1]warmth[style:knob]", 0.5, 0, 1, 0.01) : si.smoo;
presence = hslider("[2]presence[style:knob]", 0.5, 0, 1, 0.01) : si.smoo;
output = hslider("[3]output[style:knob]", 0.7, 0, 1, 0.01) : si.smoo;

// Soft clip saturation
softClip(sig) = sig * (1 + drive * 4) : ma.tanh;

// Warmth: gentle lowpass
warmthFilter(sig) = sig : fi.lowpass(1, 4000 + (1 - warmth) * 12000);

// Presence: gentle high shelf boost
presenceBoost(sig) = sig + (sig : fi.highpass(1, 3000)) * presence * 0.5;

chain(sig) = sig : softClip : warmthFilter : presenceBoost : *(output);

process(l, r) = chain(l), chain(r);
