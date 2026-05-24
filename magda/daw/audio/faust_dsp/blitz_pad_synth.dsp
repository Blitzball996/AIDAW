declare name "Space Echo";
declare author "Blitz";
declare description "Vintage space echo with modulated delay and spring reverb";
declare category "Effect";

import("stdfaust.lib");

mix = hslider("[0]mix[style:knob]", 0.35, 0, 1, 0.01) : si.smoo;
time = hslider("[1]time[style:knob]", 0.4, 0.05, 1, 0.01) : si.smoo;
feedback = hslider("[2]feedback[style:knob]", 0.4, 0, 0.9, 0.01) : si.smoo;
mod = hslider("[3]mod[style:knob]", 0.2, 0, 1, 0.01) : si.smoo;
tone = hslider("[4]tone[style:knob]", 0.6, 0, 1, 0.01) : si.smoo;
spread = hslider("[5]spread[style:knob]", 0.3, 0, 1, 0.01) : si.smoo;

// Modulated delay time
lfo = os.osc(0.8) * mod * 50;
delayTime = int(time * 44100 * 0.5 + 1000);

// Filtered feedback delay
cutoff = 1000 + tone * 8000;
echoL(sig) = (+ : de.fdelay(44100, max(1, delayTime + lfo)) : fi.lowpass(1, cutoff) : *(feedback)) ~ _ ;
echoR(sig) = (+ : de.fdelay(44100, max(1, delayTime * 1.07 - lfo * 0.8)) : fi.lowpass(1, cutoff * 0.9) : *(feedback * 0.95)) ~ _ ;

process(l, r) = l + echoL(l) * mix * (1 - spread * 0.3) + echoR(r) * mix * spread * 0.3,
                r + echoR(r) * mix * (1 - spread * 0.3) + echoL(l) * mix * spread * 0.3;
