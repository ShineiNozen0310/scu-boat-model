# 03 电控与接线

## 原始 B 站工程引脚映射

根据压缩包里的 `Hardware/*.c` 和 `User/main.c`，原项目使用的是 STM32F103，核心引脚如下：

| 功能 | 引脚 | 外设 | 来源 |
| --- | --- | --- | --- |
| 红外接收 | `PA0` | `TIM2 CH1` 输入捕获 | `Hardware/IR.c` |
| 前/后/左/右状态灯 | `PA1` - `PA4` | GPIO 输出 | `Hardware/Led.c` |
| 舵机 PWM | `PA6` | `TIM3 CH1` | `Hardware/Servo.c` |
| 电机 PWM | `PA8` | `TIM1 CH1` | `Hardware/Motor.c` |
| 电机方向 | `PB12`, `PB13` | GPIO 输出 | `Hardware/Motor.c` |

## 当前仓库建议的电控分层

- `firmware/cubemx/`：GPIO、TIM、ADC、NVIC、时钟初始化
- `firmware/app/drivers/`：红外协议解析、采集队列、板级 HAL 接口
- `firmware/app/control/`：档位、油门、舵角控制
- `firmware/app/safety/`：失联保护、低压、急停策略

## 建议补充的硬件资料

- `hardware/wiring_diagram.png`
  - 画出 MCU、电机驱动、舵机、电池、降压模块、红外接收头之间的真实连线
- `hardware/bom.xlsx`
  - 记录型号、数量、采购链接、单价和替代料
- `hardware/schematics/`
  - 如果后续自己画板，放原理图 PDF 或源文件导出件

## 低压检测建议

现在 `MainAppPlatform` 里已经预留了 `battery_is_low` 回调，建议实际工程里补一个简单方案：

- 电池分压到 ADC
- 软件里做多次采样平均
- 低于阈值后先报警，再禁止推进

## 上电检查顺序

1. 检查电池电压和极性。
2. 断开螺旋桨或让船体离水，先做空载测试。
3. 确认舵机初始化后回中。
4. 观察 LED 和 OLED 是否与控制状态一致。
5. 最后再验证电机正反转是否和逻辑定义一致。
