# SanbuCraft AI 架构决策（Phase 1）

## 边界

本阶段只建立可执行基线：应用入口、配置、日志、测试与构建。`--world` 只记录用户选择的目录，**不会**读取 `level.dat`；实际世界读取属于 Phase 3，依赖 Phase 2 的 NBT parser。

## 稳定接口原则

- `core::Config` 负责本地非敏感设置的验证、加载与保存。
- `core::Logger` 负责结构化等级日志，错误不得被静默吞掉。
- 后续模块依赖 `core`，而 `core` 不反向依赖 Minecraft、数据库、GUI、AI 或网络模块。

## 后续依赖策略

第三方依赖将在其所属阶段通过 CMake target 链接，而不会扩散进核心头文件：zlib（Phase 2）、SQLite3（Phase 7）、Qt6（Phase 10）。这使无 GUI、无网络的存档分析测试保持轻量可运行。
