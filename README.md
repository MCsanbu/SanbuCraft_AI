# SanbuCraft AI

> 一个面向 Minecraft Java Edition 世界存档的可扩展 AI 世界管家。当前仓库提供 Phase 1–8 的后端基础：配置、NBT、世界/玩家读取、容器扫描、SQLite 持久化和资源统计；不会修改 Minecraft 世界。

## 当前功能

- C++17 / CMake 工程，可在 Windows、Linux 与 macOS 上构建。
- 分层核心模块：配置（`Config`）与线程安全日志（`Logger`）。
- 可持久化的本地 `key=value` 配置文件；配置中**不保存** API Key 或 RCON 密码。
- 最小可运行命令行入口，可接受 Minecraft 世界路径，为后续 `WorldLoader` 保留稳定输入。
- CTest 覆盖配置读写/非法键与日志等级过滤。
- NBT reader/writer 支持 Java Edition 常用 tag、Big Endian、gzip/zlib 和未压缩载荷；WorldLoader 读取 `level.dat`，PlayerLoader 读取 `playerdata/*.dat`。
- Anvil `.mca` 容器扫描支持主世界、下界和末地的 chest、barrel、shulker box、ender chest；扫描结果可进入 SQLite schema（worlds、players、items、containers、container_items、chunks、structures、analysis_results）。

## 最终技术架构

```text
GUI (Qt, Phase 10)
       │
Application/Core: WorldManager · PlayerManager · AnalysisManager
       │
Domain: NBT · Region/Chunk · Player · Container · World Analyzer
       │
Infrastructure: SQLite repositories · AI providers · RCON/Fabric connection
```

- **核心层**只依赖抽象与领域模型，不能直接调用 AI、SQLite 或网络代码。
- **AI 层**将使用 `AIProvider` 抽象，分别实现 OpenAI、Local 与 Mock provider；世界上下文与具体 provider 解耦。
- **通信层**将通过 `MinecraftConnection` 隔离 RCON/Fabric 等实现；任何写操作必须走确认流程。
- **数据层**将在 Phase 7 使用 SQLite prepared statements、事务、迁移与增量扫描元数据。

## 规划目录

```text
src/
  core/          # 当前：Config、Logger；后续管理器
  minecraft/     # Phase 2 起：NBT、region、chunk、player、container
  database/      # Phase 7：SQLite 与 repositories
  ai/            # Phase 11：provider、prompt、agent
  network/       # Phase 15：RCON / Fabric adapter
  gui/           # Phase 10：Qt dashboard、地图与聊天
tests/           # 每个模块的独立测试
resources/ data/ docs/
```

## 环境要求

- CMake 3.20+
- 支持 C++17 的编译器：MSVC 2022、GCC 9+ 或 Clang 10+
- 当前 Phase 1 **没有第三方运行时依赖**。后续将按阶段引入 zlib（NBT 压缩）、SQLite3、Qt 6 与可选 HTTP/RCON 依赖。

## 构建与运行

### Windows（Visual Studio 2022）

在“x64 Native Tools Command Prompt for VS 2022”中，进入项目根目录后执行：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\sanbucraft_ai.exe --world "C:\Users\<用户名>\AppData\Roaming\.minecraft\saves\Survival_01"
```

首次运行会创建 `data/sanbucraft.conf` 与 `data/sanbucraft.log`。可用 `--config <路径>` 选择其他本地配置文件；`--log-level debug|info|warning|error` 调整终端日志等级。

### Linux/macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/sanbucraft_ai --world "$HOME/.minecraft/saves/Survival_01"
```

## 配置与安全

`data/sanbucraft.conf` 当前支持世界路径、未来数据库路径、日志路径、AI provider/model 及 RCON 地址/端口。密钥与密码不会被配置接口写出，未来会接入系统凭据存储或运行时环境变量。配置解析拒绝未知键和无效端口，避免静默拼写错误。

## Minecraft 兼容性

面向 Java Edition 的 gzip/zlib NBT、`level.dat`、`playerdata/*.dat` 与 Anvil `.mca`。已实现的容器解析覆盖常见的 `block_entities` 和旧版 `Level.TileEntities` 布局；其他自定义维度、模组容器和跨版本数据变化将继续补充兼容性测试。

## 开发路线

1. **已完成：Phase 1–8 基础实现** — 骨架、NBT、世界/玩家/背包、Anvil 容器、SQLite schema 与资源分析。
2. Phase 9–10 — 地图与 Qt Dashboard。
3. Phase 11–18 — 可替换 AI、上下文、工具调用、安全实时连接、确认式自动化、优化与打包。

## 常见问题

**程序会修改我的世界吗？** 不会。当前版本完全不读取世界内容，更不会修改存档。未来所有写入操作都将默认要求明确确认。

**为什么没有 API Key 配置？** Phase 1 刻意不处理密钥；绝不会将 API Key 或 RCON 密码硬编码或提交到 Git。
