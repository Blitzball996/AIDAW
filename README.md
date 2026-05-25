# Blitz — AI-Driven Audio Workstation

[中文](#中文) | [English](#english)

---

## English

**Blitz** is an AI-powered Digital Audio Workstation (DAW) built with JUCE and Tracktion Engine. It combines traditional music production tools with AI-assisted composition, sound design, and automation.

### Features

- **AI Chat Assistant** — Natural language control: create tracks, generate chord progressions, automate parameters
- **261 Built-in Instruments** — General MIDI SoundFont library (piano, strings, brass, woodwinds, guitars, synths, drums)
- **4OSC Synthesizer** — Four-oscillator subtractive synth with 26 factory presets
- **Faust DSP Effects** — Shimmer Verb, Tape Warble, Stereo Widener, Analog Warmth, Space Echo
- **Virtual Keyboard** — Logic Pro-style Musical Typing (Ctrl+K) with pitch bend, mod wheel, sustain
- **Piano Roll** — Pencil tool for drawing notes, keyboard slide preview
- **Plugin Support** — VST3/CLAP auto-scanning on first launch
- **Multi-track Recording** — Audio and MIDI recording with unlimited tracks

### Quick Start

1. Download the latest release from [Releases](https://github.com/Blitzball996/AIDAW/releases)
2. Run the installer (Windows) or open the DMG (macOS)
3. Add an instrument from the browser (Instrument > Piano/Strings/Brass...)
4. Press Ctrl+K to open the virtual keyboard and start playing

### Building from Source

```bash
git clone --recursive https://github.com/Blitzball996/AIDAW.git
cd AIDAW
mkdir -p assets/soundfonts
curl -L "https://github.com/mrbumpy409/GeneralUser-GS/raw/main/GeneralUser%20GS%20v1.471.sf2" \
  -o assets/soundfonts/GeneralUser_GS.sf2
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target magda_daw_app
```

---

## 中文

**Blitz** 是一款 AI 驱动的数字音频工作站，基于 JUCE 和 Tracktion Engine 构建。

### 功能

- **AI 聊天助手** — 自然语言控制：创建轨道、生成和弦、自动化参数
- **261 种内置乐器** — GM 音色库（钢琴、弦乐、铜管、木管、吉他、合成器、鼓组）
- **4OSC 合成器** — 四振荡器减法合成，26 个工厂预设
- **Faust 效果器** — 微光混响、磁带抖动、立体声加宽、模拟温暖、太空回声
- **虚拟键盘** — Logic 风格 Musical Typing（Ctrl+K），弯音/调制/延音
- **钢琴卷帘窗** — 铅笔工具画音符、键盘滑音预览
- **插件支持** — 首次启动自动扫描 VST3/CLAP
- **多轨录音** — 音频和 MIDI，无限轨道

### 虚拟键盘

| 按键 | 功能 |
|------|------|
| A S D F G H J K L ; ' | 白键 C-F（2八度）|
| W E T Y U O P | 黑键 |
| Z / X | 八度 -/+ |
| C / V | 力度 -/+ |
| 1 / 2 | 弯音 下/上 |
| 3-8 | 调制轮 |
| Tab | 延音踏板 |
