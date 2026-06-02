// pcm-capture-worklet.js — taps Strudel/superdough's master output and forwards
// fixed-size stereo Float32 chunks to the main thread, which relays them to the
// JUCE C++ side (jucePcmChunk) for recording / routing into the DAW track.
//
// Loaded via audioWorklet.addModule('/pcm-capture-worklet.js') and connected in
// parallel off the master GainNode (see index.html installPcmCapture()).

class PcmCaptureProcessor extends AudioWorkletProcessor {
  constructor() {
    super();
    this._frames = 1024; // ~21 ms @ 48 kHz
    this._l = new Float32Array(this._frames);
    this._r = new Float32Array(this._frames);
    this._pos = 0;
  }

  process(inputs, outputs) {
    const input = inputs[0];
    if (!input || input.length === 0) return true;

    const src0 = input[0];
    const src1 = input[1] || input[0]; // mono -> duplicate
    const n = src0.length;

    // Pass-through so this node can also sit inline if needed.
    const out = outputs[0];
    if (out && out[0]) out[0].set(src0);
    if (out && out[1]) out[1].set(src1);

    let i = 0;
    while (i < n) {
      const space = this._frames - this._pos;
      const copy = Math.min(space, n - i);
      this._l.set(src0.subarray(i, i + copy), this._pos);
      this._r.set(src1.subarray(i, i + copy), this._pos);
      this._pos += copy;
      i += copy;

      if (this._pos >= this._frames) {
        const lBuf = this._l.buffer;
        const rBuf = this._r.buffer;
        this.port.postMessage({ l: lBuf, r: rBuf, sampleRate }, [lBuf, rBuf]);
        this._l = new Float32Array(this._frames);
        this._r = new Float32Array(this._frames);
        this._pos = 0;
      }
    }
    return true;
  }
}

registerProcessor('pcm-capture-processor', PcmCaptureProcessor);
