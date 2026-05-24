declare name "Shimmer Verb";
declare author "Blitz";
declare description "Lush shimmer reverb with feedback tail";
declare category "Effect";

import("stdfaust.lib");

mix = hslider("[0]mix[style:knob]", 0.35, 0, 1, 0.01) : si.smoo;
decay = hslider("[1]decay[style:knob]", 0.8, 0.1, 0.97, 0.01) : si.smoo;
tone = hslider("[2]tone[style:knob]", 0.6, 0, 1, 0.01) : si.smoo;
size = hslider("[3]size[style:knob]", 0.6, 0.1, 1, 0.01) : si.smoo;

cutoff = 1000 + tone * 10000;
dt1 = int(2137 * size + 200);
dt2 = int(3571 * size + 300);

verb(sig) = (+ : de.fdelay(16384, dt1) : fi.lowpass(1, cutoff) : *(decay)) ~ _ :
            (+ : de.fdelay(16384, dt2) : fi.lowpass(1, cutoff * 0.8) : *(decay * 0.9)) ~ _;

process(l, r) = l * (1-mix) + verb(l) * mix, r * (1-mix) + verb(r) * mix;
