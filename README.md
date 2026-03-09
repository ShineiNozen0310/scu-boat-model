# scu-boat-model

基于 B 站红外遥控小船 STM32F103 工程整理出的项目骨架。当前仓库重点不是“把原工程原样搬进来”，而是把它改造成一个后续能继续做机械、电控、测试和比赛交付的结构化项目。

## 当前整理思路

- `firmware/cubemx/` 放 CubeMX/Keil 自动生成工程和板级初始化。
- `firmware/app/` 放手写业务逻辑，避免以后再次把中断、解码、控制和硬件驱动搅在一起。
- `docs/` 拆成概览、规则、机械、电控、软件、测试五类文档，后续更容易补充。
- `hardware/`、`mechanical/`、`test/` 预留真实交付物位置，而不是把所有资料都堆在 README 里。

## 仓库结构

```text
scu-boat-model/
├─ README.md
├─ LICENSE
├─ docs/
│  ├─ 00_overview.md
│  ├─ 01_rules_and_scoring.md
│  ├─ 02_mechanics.md
│  ├─ 03_electronics.md
│  ├─ 04_control_and_software.md
│  ├─ 05_test_log.md
│  └─ images/
├─ firmware/
│  ├─ cubemx/
│  ├─ app/
│  │  ├─ control/
│  │  ├─ drivers/
│  │  ├─ safety/
│  │  └─ main_app.c/.h
│  └─ tools/
├─ hardware/
│  └─ schematics/
├─ mechanical/
│  ├─ hull_design/
│  ├─ 3d_print/
│  └─ assembly_notes.md
├─ test/
│  ├─ bench/
│  └─ water/
└─ .gitignore
```

## 固件现状

已经把原来单层的 `firmware/include + firmware/src` 调整成按职责拆分的结构：

- `app/drivers/boat_ir_decoder.*`：NEC 红外解码
- `app/drivers/ir_capture_queue.*`：中断到主循环的捕获队列
- `app/control/boat_controller.*`：档位、油门、舵角、平滑输出
- `app/safety/boat_safety.*`：失联保护、低压占位、急停接口
- `app/main_app.*`：应用层入口，供 CubeMX 生成代码调用

## 与原始 B 站工程的关系

原工程压缩包里主要是：

- `User/main.c`
- `Hardware/IR.c`
- `Hardware/Motor.c`
- `Hardware/Servo.c`
- `Hardware/Led.c`
- 一套标准库和 Keil 工程文件

现在仓库保留了它的控制语义和引脚映射信息，但把原本在 `TIM2_IRQHandler()` 里同时做“采集 + 解码 + 改状态”的方式拆开了，方便后续继续加低压检测、急停、测速、避障或 PID。

## 下一步建议

1. 把你实际的 `.ioc`、`Core/Src`、`Core/Inc` 或 Keil 工程放进 `firmware/cubemx/`。
2. 在 `hardware/` 中补接线图、BOM 和原理图导出文件。
3. 在 `mechanical/` 中补船体尺寸、推进器和舵机安装数据。
4. 按 `test/bench` 和 `test/water` 的模板记录调试数据，不要只留视频。
