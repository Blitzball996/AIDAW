declare name "Stereo Widener";
declare author "Blitz";
declare description "Mid-side stereo width control with bass mono";
declare category "Effect";

import("stdfaust.lib");

width = hslider("[0]width[style:knob]", 1.0, 0, 3, 0.01) : si.smoo;
bassMono = hslider("[1]bass_mono[style:knob]", 200, 20, 500, 1) : si.smoo;
balance = hslider("[2]balance[style:knob]", 0, -1, 1, 0.01) : si.smoo;

process(l, r) = outL, outR
with {
    mid = (l + r) * 0.5;
    side = (l - r) * 0.5;

    // Widen the side signal
    wideSide = side * width;

    // Keep bass mono
    bassL = l : fi.lowpass(2, bassMono);
    bassR = r : fi.lowpass(2, bassMono);
    bassMid = (bassL + bassR) * 0.5;

    // Reconstruct
    rawL = mid + wideSide;
    rawR = mid - wideSide;

    // Replace bass with mono bass
    highL = rawL : fi.highpass(2, bassMono);
    highR = rawR : fi.highpass(2, bassMono);

    outL = (bassMid + highL) * (1 - max(0, balance));
    outR = (bassMid + highR) * (1 + min(0, balance));
};
