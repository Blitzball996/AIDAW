declare name "Tape Warble";
declare author "Blitz";
declare description "Lo-fi tape warble effect with wow, flutter, and saturation";
declare category "Effect";

import("stdfaust.lib");

mix = hslider("[0]mix[style:knob]", 0.5, 0, 1, 0.01) : si.smoo;
wow = hslider("[1]wow[style:knob]", 0.3, 0, 1, 0.01) : si.smoo;
flutter = hslider("[2]flutter[style:knob]", 0.2, 0, 1, 0.01) : si.smoo;
saturation = hslider("[3]saturation[style:knob]", 0.3, 0, 1, 0.01) : si.smoo;
hiss = hslider("[4]hiss[style:knob]", 0.1, 0, 0.5, 0.01) : si.smoo;
tone = hslider("[5]tone[style:knob]", 0.7, 0, 1, 0.01) : si.smoo;

// Wow: slow pitch modulation
wowLfo = os.osc(0.5 + wow * 2) * wow * 3;

// Flutter: fast pitch modulation
flutterLfo = os.osc(6 + flutter * 10) * flutter * 0.5;

// Combined modulated delay
modDelay = 200 + wowLfo + flutterLfo;

// Soft saturation
saturate(sig) = sig : *(1 + saturation * 3) : ma.tanh : *(1 / (1 + saturation * 2));

// Tape hiss
tapeHiss = no.noise * hiss * 0.05;

// Tone (lowpass)
cutoff = 2000 + tone * 14000;

tape(sig) = sig : de.fdelay(1024, max(1, modDelay)) : saturate : fi.lowpass(1, cutoff) : +(tapeHiss);

process(l, r) = l * (1-mix) + tape(l) * mix, r * (1-mix) + tape(r) * mix;
