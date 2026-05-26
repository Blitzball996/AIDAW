# AIDAW 音源与合成器集成进度

## 已完成

- [x] **sfizz 引擎** — C++ SFZ 采样引擎（sinc 插值，高音质），作为 `third_party/sfizz` 子模块集成
- [x] **SfizzPlugin** — 内部插件，支持 noteOn/Off, CC, pitchBend，从 `exe_dir/sfz/` 目录加载 .sfz 文件
- [x] **Salamander Drum Kit** — SFZ 格式鼓组，位于 `assets/sfz/drums/salamander/`
- [x] **SoundFont CC/PitchBend 支持** — tsf_channel_set_pitchwheel + tsf_channel_midi_control
- [x] **多 SF2 文件切换** — SoundFont Player 支持运行时切换不同 SF2 文件（从 `soundfonts/` 目录扫描）
- [x] **GM 鼓组预设** — Standard/Room/Power/Electronic/TR-808/Jazz/Brush/Orchestra Kit
- [x] **808 Bass 预设** — 808 Sub Bass / 808 Synth Bass（GM program 38/39）
- [x] **下载脚本** — `scripts/download_soundfonts.bat` 用于下载额外音源
- [x] **File → Download Extra Content** — DAW 内下载弹窗（FluidR3/DX7/Salamander/SSO）
- [x] **AI Agent 乐器列表** — system prompt 中列出所有可用乐器和鼓组
- [x] **导出多轨** — 已修复

---

## 待完成 — 合成器引擎集成

### 优先级 1：自写 808 Bass 合成器（代码量小，立即见效）

- [ ] **808 Bass Synth Plugin** — 纯正弦波 + pitch drop envelope + amp decay
  - 实现: 新建 `EightOhEightPlugin` 类（<200 行代码）
  - 参数: Decay, Pitch Drop, Drive/Saturation, Tone
  - 注册为 'tr808bass' 内部插件
  - 这是 trap/hip-hop 制作的核心音色

### 优先级 2：Dexed FM 合成器集成（中等复杂度）

- [ ] **Dexed (DX7 FM Synth)** — 精确模拟 Yamaha DX7
  - 来源: https://github.com/asb2m10/dexed (GPL, JUCE C++)
  - 特点: 6 算子 FM 合成，支持原版 DX7 SysEx 补丁（数万个免费预设）
  - 集成方式: 提取 DSP 引擎（`msfa/` 目录），创建 `DexedPlugin` 包装
  - 预设: 可加载 .syx 文件（DX7 cartridge dumps，网上有数千个免费预设）

### 优先级 3：OB-Xf 虚拟模拟合成器（经典模拟音色）

- [ ] **OB-Xf (Oberheim OB-X 模拟)** — 经典虚拟模拟合成器
  - 来源: https://github.com/surge-synthesizer/OB-Xf (GPL, JUCE C++)
  - 特点: 温暖的模拟滤波器，经典 pad/lead/bass 音色
  - 集成方式: 提取 DSP 核心，创建 `OBXPlugin` 包装
  - Surge 团队维护，代码质量高

### 优先级 4：波表合成器（现代音色设计）

- [ ] **Wavetable Synth Plugin** — 波表合成器
  - 参考: https://github.com/hsetlik/Octane (JUCE wavetable synth)
  - 参考: https://github.com/TheWaveWarden/odin2 (Odin 2, 混合合成)
  - 特点: 波表振荡器 + morphing + 滤波器 + ADSR + LFO
  - 支持加载自定义 .wav 单周期波表
  - 内置工厂波表（saw, square, sine, triangle + harmonics）

### 优先级 5：Surge XT（最强大，作为外部 VST 加载）

- [ ] **Surge XT 支持** — 不嵌入代码，作为外部 VST3 插件扫描加载
  - 来源: https://github.com/surge-synthesizer/surge (GPL)
  - 特点: 减法+波表+FM 混合，900+ 预设，专业级
  - 集成方式: 用户自行安装 Surge XT VST3，DAW 扫描加载
  - 代码量太大（100k+ LOC）不适合嵌入

---

## 待完成 — 需要 DAW 内下载的音源（文件太大）

### SoundFont (.sf2)

- [ ] **FluidR3_GM.sf2** (141MB) — 高质量通用 GM 音源
  - 来源: https://archive.org/download/fluidr3-gm-gs/FluidR3_GM.sf2
  - 内容: 钢琴/弦乐/管乐/鼓全套，比 GeneralUser 音质更好

- [ ] **Yamaha DX7 ROM Banks** (18-70MB/bank) — 经典 FM 合成器音色
  - 来源: https://github.com/Caskexe/DX (需要 Git LFS)
  - 内容: DX7 原厂 ROM 补丁，多个 bank

### SFZ 采样库

- [ ] **Salamander Drum Kit 完整版** (200MB) — 高质量原声鼓采样
  - 来源: https://freepats.zenvoid.org/Percussion/Drumkits/
  - 当前只有 .sfz 映射文件，缺少 WAV 采样

- [ ] **SM Drums** — 多种鼓组风格
  - 来源: https://sfzinstruments.github.io/drums/sm_drums/

- [ ] **Sonatina Symphonic Orchestra (SSO)** (1.39GB) — 完整管弦乐团
  - 来源: https://sfzinstruments.github.io/orchestra/sso/

- [ ] **Virtual Playing Orchestra (VPO)** (3.2GB) — 高质量管弦乐
  - 来源: https://sfzinstruments.github.io/orchestra/

- [ ] **Versilian Community Sample Library (VCSL)** — 管弦/世界/实验乐器
  - 来源: https://versilian-studios.com/vcsl/

---

## 待完成 — 需要开发的功能

- [ ] **DAW 内音源下载管理器 UI** — 完善 File → Download Extra Content
  - 显示下载进度条
  - 支持断点续传
  - 下载完成后自动刷新音源列表
  - 显示已安装/可用音源状态

---

## 参考资源

### 开源合成器

| 项目 | 类型 | Stars | 链接 |
|------|------|-------|------|
| Surge XT | 混合（减法+波表+FM） | 14k+ | https://github.com/surge-synthesizer/surge |
| Vital/Vitalium | 波表（Spectral Warping） | 4k+ | https://github.com/mtytel/vital |
| Dexed | FM（DX7 克隆） | 3k+ | https://github.com/asb2m10/dexed |
| OB-Xf | 虚拟模拟（Oberheim） | — | https://github.com/surge-synthesizer/OB-Xf |
| Odin 2 | 混合（VA+波表+FM） | 600+ | https://github.com/TheWaveWarden/odin2 |
| Infernal Synth | VA + FM | — | https://github.com/sjoerdvankreel/infernal-synth |
| Octane | 波表 | — | https://github.com/hsetlik/Octane |
| JV-880 JUCE | Roland JV-880 模拟 | — | https://github.com/giulioz/jv880_juce |
| FM-Synthesizer | JUCE FM 合成 | — | https://github.com/torrancecui/FM-Synthesizer |

### 音源资源

- awesome-soundfonts: https://github.com/ad-si/awesome-soundfonts
- free-soundfonts: https://github.com/marmooo/free-soundfonts
- SFZ Instruments: https://sfzinstruments.github.io/
- sfizz: https://github.com/sfztools/sfizz
- awesome-audio-dsp: https://github.com/BillyDM/awesome-audio-dsp
- WeirdDrums: https://github.com/dfilaretti/WeirdDrums
- Free 808 Samples: https://www.echosoundworks.com/free808v1
