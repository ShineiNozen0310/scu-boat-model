# Firmware & CubeMX

本目录负责硬件初始化与 CubeMX/Keil 工程集成相关指引。

## 内容与规范

- **板级外设配置**：提供面向 STM32F103、`USART1` 无线链路、双向电调和多路舵机（`TIM3 PWM`）的接入模板。
- **业务代码集成**：演示如何将手写业务逻辑（例如 `boat_app_port.*`）无缝接入到生成的外部中断、主循环中。

## 当前默认真机方案

当前仓库默认对齐 `800` 元内预算方案：

- `USART1` 接 `E220` 透明串口模块
- 应用层链路走 `boat_radio_protocol`
- 模式开关在 [boat_board_config.h](/c:/Users/60385/Desktop/scu-boat-model/firmware/cubemx/boat_board_config.h#L33) 里，默认是 `BOAT_UART_LINK_MODE_RADIO_PACKET`

如果后面切回 `ELRS -> CRSF`，只需要把 `BOAT_UART_LINK_MODE` 改回 `BOAT_UART_LINK_MODE_CRSF`，上层控制逻辑不用重写。

## CubeMX 真机配置清单

要让现在这套预算链路在真机上直接跑通，`CubeMX` 里至少确认下面几项：

1. `USART1` 设为 `Asynchronous`
2. `PA10` 作为 `USART1_RX`，接 `E220 TXD`
3. `PA9` 作为 `USART1_TX`，接 `E220 RXD`
4. 波特率设成 `BOAT_UART1_BAUD`
5. 串口格式使用 `8 data bits / no parity / 1 stop bit / no flow control`
6. 打开 `USART1 global interrupt`
7. `TIM3` 保持 `50Hz PWM`

如果你想把 `M0/M1/AUX` 也接上，建议：

- `PB12` -> `M0`，推挽输出，默认低
- `PB13` -> `M1`，推挽输出，默认低
- `PB14` <- `AUX`，输入或上拉输入

这些引脚默认是关闭的；只有在 [boat_board_config.h](/c:/Users/60385/Desktop/scu-boat-model/firmware/cubemx/boat_board_config.h#L54) 后面的开关打开时，`boat_app_port.c` 才会在初始化时去控制/等待它们。

## 集成位置

- 主循环和 UART 回调模板见 [main_template.c](/c:/Users/60385/Desktop/scu-boat-model/firmware/cubemx/main_template.c#L1)
- 板级接入实现见 [boat_app_port.c](/c:/Users/60385/Desktop/scu-boat-model/firmware/cubemx/boat_app_port.c#L1)
- 真机前最少要保证生成工程里的 `MX_USART1_UART_Init()` 最终使用了 `BOAT_UART1_BAUD`
