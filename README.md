# SCU Boat Model

本项目（scu-boat-model）是一个基于B站红外遥控小船的 STM32F103 工程重构而来的遥控小船模型，实现了机械、电控、测试和代码的结构化分离。

## 目录结构

- `firmware/`: 固件代码，包含手写业务逻辑与 CubeMX 自动生成代码的解耦实现。
- `hardware/`: 占位，计划存放真实的硬件交付物（接线图/BOM/原理图）。
- `mechanical/`: 占位，计划存放船体 3D 打印件与尺寸草图。
- `docs/`: 详细的文档与机制说明（如计分规则、测试日志摘要）。
- `test/`: 各类测试用例与记录（包含 Host自测、台架测试与下水测试）。

## 快速开始

请查阅 `docs/00_overview.md` 获取该项目的总体介绍和构建指南。
如果准备手搓遥控器, 且希望优先保证抗干扰能力, 先看 `docs/07_diy_remote_anti_interference.md`。
