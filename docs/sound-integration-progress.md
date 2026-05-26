# AIDAW 音源集成进度

## 已完成

- [x] **sfizz 引擎** — C++ SFZ 采样引擎（sinc 插值，高音质），作为 `third_party/sfizz` 子模块集成
- [x] **SfizzPlugin** — 内部插件，支持 noteOn/Off, CC, pitchBend，从 `exe_dir/sfz/` 目录加载 .sfz 文件
- [x] **Salamander Drum Kit** — SFZ 格式鼓组，位于 `assets/sfz/drums/salamander/`
- [x] **SoundFont CC/PitchBend 支持** — tsf_channel_set_pitchwheel + tsf_channel_midi_control
- [x] **多 SF2 文件切换** — SoundFont Player 支持运行时切换不同 SF2 文件（从 `soundfonts/` 目录扫描）
- [x] **下载脚本** — `scripts/download_soundfonts.bat` 用于下载额外音源
- [x] **AI Agent 乐器列表** — system prompt 中列出所有可用乐器（4osc/soundfont/sfizz/drumgrid 等）
- [x] **导出多轨** — 已修复

## 待完成

### 需要 DAW 内部 UI 下载（文件太大，不能放 git）

- [ ] **FluidR3_GM.sf2** (141MB) — 高质量通用 GM 音源（钢琴/弦乐/管乐/鼓全套）
  - 来源: https://member.keymusician.com/Member/FluidR3_GM/ 或 archive.org
  - 需要: DAW 内 "下载音源" 按钮，下载到 `soundfonts/` 目录
  
- [ ] **Yamaha DX7 FM Synth** (18-70MB/bank) — 经典 FM 合成器音色
  - 来源: https://github.com/Caskexe/DX (需要 Git LFS)
  - 需要: DAW 内下载，选择 ROM 1A 或 ROM 1B

- [ ] **Sonatina Symphonic Orchestra (SSO)** (1.39GB) — 完整管弦乐团 SFZ
  - 来源: https://sfzinstruments.github.io/orchestra/sso/
  - 需要: DAW 内下载管理器，分包下载

- [ ] **Virtual Playing Orchestra (VPO)** (3.2GB) — 高质量管弦乐 SFZ
  - 来源: https://sfzinstruments.github.io/orchestra/
  - 需要: DAW 内下载管理器

- [ ] **Versilian Community Sample Library (VCSL)** — 管弦/世界/实验乐器
  - 来源: https://versilian-studios.com/vcsl/
  - 需要: DAW 内下载管理器

### 需要开发的功能

- [ ] **DAW 内音源下载管理器 UI** — 用户在 DAW 内浏览/下载/管理音源包
  - 显示可用音源列表（名称、大小、描述）
  - 一键下载到正确目录
  - 显示下载进度
  - 下载完成后自动刷新音源列表

- [ ] **波表合成器插件 (WavetableSynthPlugin)** — 新建内部合成器
  - 参考: https://github.com/hsetlik/Octane
  - 波表振荡器 + morphing
  - 滤波器 (LP/HP/BP) + ADSR + LFO
  - 支持加载自定义 .wav 单周期波表
  - 内置工厂波表（saw, square, sine, triangle + harmonics）
  - 注册为 'wavetable' 内部插件

- [ ] **更多 SFZ 鼓组预设**
  - SM Drums: https://sfzinstruments.github.io/drums/sm_drums/
  - 需要: DAW 内下载

## 参考资源

- awesome-soundfonts: https://github.com/ad-si/awesome-soundfonts
- free-soundfonts: https://github.com/marmooo/free-soundfonts
- SFZ Instruments: https://sfzinstruments.github.io/
- sfizz: https://github.com/sfztools/sfizz
- Octane (wavetable): https://github.com/hsetlik/Octane
- WeirdDrums: https://github.com/dfilaretti/WeirdDrums
