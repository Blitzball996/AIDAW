# AIDAW 新功能 UI 使用说明

## 虚拟钢琴键盘

### 键位映射（默认八度 C4）

| 电脑键 | 音符 | 类型 |
|--------|------|------|
| A | C | 白键 |
| W | C# | 黑键 |
| S | D | 白键 |
| E | D# | 黑键 |
| D | E | 白键 |
| F | F | 白键 |
| T | F# | 黑键 |
| G | G | 白键 |
| Y | G# | 黑键 |
| H | A | 白键 |
| U | A# | 黑键 |
| J | B | 白键 |
| K | C (高八度) | 白键 |

### 控制键

| 键 | 功能 |
|----|------|
| Z | 八度下移 |
| X | 八度上移 |

### 录制
- 点击录制按钮开始录制
- 弹奏的音符会按时间戳记录
- 停止后音符写入当前轨道的 MIDI clip

---

## 鼓机 (Beat Box)

在底部面板中选择 "Beat Box" tab：
- 16 列 = 16 个步进（一小节）
- 8 行 = 8 个乐器（Kick, Snare, HiHat, Open HH, Clap, Tom, Rim, Crash）
- 点击格子 = 开/关该步进
- 右上角下拉菜单选择内置模式：Four on Floor, Breakbeat, Bossa Nova, Shuffle
- Swing 旋钮调节摇摆感

---

## 导出对话框

菜单 File → Export Audio：
- 格式选择：WAV (16/24/32bit), FLAC, MP3 (128/192/320), OGG, AIFF
- 采样率：44100, 48000, 88200, 96000, 192000
- "Export Stems" 勾选 = 每轨单独导出
- "Normalize" 勾选 = 导出后自动标准化
- 进度条显示导出进度

---

## 音频操作

右键点击音频 clip：
- **Strip Silence** — 自动检测并移除静音段（可设阈值 dB）
- **Normalize** — 标准化到目标电平
- **Reverse** — 反转音频
- **Fade In** — 添加淡入
- **Fade Out** — 添加淡出

---

## 节拍器

传输栏的节拍器按钮（🔔图标）：
- 左键点击 = 开/关节拍器
- 右键点击 = 设置菜单：
  - 音量
  - Count-in（1/2 小节预备拍）
  - 细分（1/4, 1/8, 1/16）

---

## 素材浏览器 (Clip Browser)

左侧面板 "Browser" tab：
- 左边：文件夹树形导航
- 右边：音频/MIDI 文件列表
- 顶部搜索栏
- ⭐ 按钮收藏常用素材
- 拖拽文件到时间线即可导入

---

## 量化 (Quantize)

在 Piano Roll 中选中音符后，菜单 Edit → Quantize：
- Grid Size：1/4, 1/8, 1/16, 1/32
- Strength：0-100%（100% = 完全对齐网格）
- Swing：0-100%（偶数拍偏移）
- Groove Template 下拉：MPC Swing 54%, MPC Swing 71%, Shuffle Light, Shuffle Heavy
- Humanize：0-100%（随机微偏移）

---

## VCA 推子

混音器视图中：
- VCA 推子条显示在普通通道条右侧
- 创建 VCA：右键混音器空白处 → "Add VCA Fader"
- 分配轨道：拖拽轨道到 VCA 条，或右键 VCA → "Assign Tracks"
- VCA 推子控制所有分配轨道的音量（比例缩放）

---

## Send/Return

每个通道条上的 Send 旋钮：
- 小旋钮控制发送电平
- 右键旋钮 → Pre/Post Fader 切换
- 创建 Return Bus：菜单 Track → Add Return Bus

---

## Route Groups

混音器中：
- 选中多个轨道 → 右键 → "Create Group"
- 组内轨道联动：音量/声像/静音/独奏/录制
- 组名和颜色可自定义

---

## 控制器映射

菜单 Settings → Controllers：
- **OSC**：设置端口（默认 8000），自动映射 /track/N/volume 等
- **MIDI**：选择设备，Learn 模式（点击参数 → 转动旋钮 → 自动绑定）
- 内置预设：Korg nanoKONTROL2, Akai APC40

---

## 视频轨道

菜单 Track → Add Video Track：
- 导入视频文件（需要系统安装 FFmpeg）
- 视频缩略图显示在时间线上
- 播放时视频同步音频位置

---

## 环绕声

轨道属性 → Output → Surround：
- 支持格式：Stereo, Quad, 5.1, 7.1, 7.1.4 (Atmos)
- 环绕声定位器：拖拽声源位置（方位角/仰角/距离）
- LFE 发送旋钮
