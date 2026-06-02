# CI/CD 构建与发布指南

## 概览

项目使用 GitHub Actions 自动构建和发布。所有配置文件在 `.github/workflows/` 目录下。

## 自动构建（每次 push master）

每次你 push 代码到 master 分支，会自动触发：

- **Build macOS DMG** — macOS arm64 (Apple Silicon, 11.0+)
- **Build Linux** — Linux x86_64

这两个只是验证编译能通过，产物在 Actions 页面可下载（7天过期）。

## 发布新版本

### 步骤

```bash
# 1. 确保代码在 master 上且 CI 通过
git push origin master

# 2. 打版本 tag
git tag v0.2.0

# 3. 推送 tag（如果需要代理）
git -c http.proxy=http://127.0.0.1:7897 -c https.proxy=http://127.0.0.1:7897 push origin v0.2.0

# 或者不需要代理时
git push origin v0.2.0
```

### 自动发生的事

推送 `v*` tag 后，`release.yml` 自动：

1. 在 4 台云服务器上并行构建：
   | 平台 | 文件名 | 说明 |
   |------|--------|------|
   | macOS arm64 | `Blitz-macOS-arm64.dmg` | Apple Silicon, macOS 11+, Metal GPU + 本地 GGUF |
   | macOS x86_64 | `Blitz-macOS-x86_64-legacy.dmg` | Intel Mac, macOS 10.13+, 无本地推理 |
   | Linux x86_64 | `Blitz-linux-x86_64.tar.gz` | Linux 通用 |
   | Windows x64 | `Blitz-Windows-x64.zip` | Windows 10/11 |

2. 全部构建成功后，自动创建 GitHub Release 页面
3. 4 个安装包作为下载附件挂在 Release 上

### 查看进度

```bash
# 查看 release workflow 状态
gh run list --workflow release.yml --limit 3

# 查看具体 run 的日志
gh run view <run-id> --log-failed

# 查看已发布的 releases
gh release list
```

## 版本号规则

使用语义化版本 `vX.Y.Z`：

- `v0.1.0` → 第一个公开版本
- `v0.1.1` → bug 修复
- `v0.2.0` → 新功能
- `v1.0.0` → 正式稳定版

## 常见操作

### 删除错误的 tag 重新发布

```bash
# 删除本地和远程 tag
git tag -d v0.2.0
git push origin :refs/tags/v0.2.0

# 修复后重新打 tag
git tag v0.2.0
git push origin v0.2.0
```

### 只构建不发布（手动触发）

在 GitHub 仓库页面 → Actions → Release → Run workflow，手动触发构建但不会创建 Release（因为没有 tag）。

### 查看构建产物

即使不打 tag，每次 push master 的构建产物也可以在 Actions 页面下载：
GitHub → Actions → 选择一个成功的 run → 底部 Artifacts 区域

## 文件结构

```
.github/workflows/
├── build-macos.yml    # push master 触发，验证 macOS 编译
├── build-linux.yml    # push master 触发，验证 Linux 编译
└── release.yml        # push tag 触发，4 平台构建 + 发布
```

## 注意事项

- Release workflow 需要 4 个平台全部构建成功才会创建 Release
- 如果某个平台失败，不会发布，需要修复后重新打 tag
- macOS legacy 版本禁用了本地 GGUF 推理（`-DMAGDA_ENABLE_LOCAL_LLM=OFF`）以兼容 10.13
- Windows 构建使用 Visual Studio 2022
- 构建时间约 30-40 分钟（4 个平台并行）
