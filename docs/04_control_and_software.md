# 04 控制与软件

## 原始代码到当前结构的映射

| 原始模块 | 当前模块 | 说明 |
| --- | --- | --- |
| `Hardware/IR.c` 中的 `IR_Rcv()` | `app/drivers/boat_ir_decoder.*` | 解码逻辑从中断旁路出来 |
| `Hardware/IR.c` 中的 `IR_Reply()` | `app/control/boat_controller.*` | 控制逻辑集中管理 |
| `TIM2_IRQHandler()` | `app/drivers/ir_capture_queue.*` + `app/main_app.*` | 中断只负责采集 |
| `Motor_Speed/Motor_Dir/Servo_Angel` | `BoatController` | 统一为显式状态结构体 |

## 当前软件行为

- `0`：停车、舵机回中、状态复位
- `*`：切到前进档并清零目标油门
- `#`：切到后退档并清零目标油门
- `UP/DOWN`：按固定步进调节目标油门
- `LEFT/RIGHT`：按固定步进调节舵角
- `OK`：舵机回中
- 重复帧：支持长按连续调节
- 输出：电机占空比平滑逼近目标值

## 新增的结构优化

### 1. `MainApp`

`main_app.c/.h` 是现在唯一建议由 CubeMX 生成代码直接调用的应用入口。这样后续你换板子、换 IDE，业务逻辑都不需要重写。

### 2. 安全层

`boat_safety.*` 目前已经统一处理三类安全状态：

- 遥控失联
- 低压输入
- 外部急停

现在低压和急停主要是接口预留，接上 ADC 或独立按键后就能继续往下做。

### 3. 参数集中管理

`boat_config.h` 里统一了这些参数：

- 油门最大值、步进值、平滑步长
- 舵机最小 / 最大 / 中位 / 步进角度
- NEC 协议时序窗口
- 失联超时时间

## 建议的 CubeMX 集成方式

1. 在板级 `main.c` 中初始化硬件。
2. 组装一个 `MainAppPlatform` 结构体。
3. 调用 `MainApp_Init(&app, &platform)`。
4. 主循环里调用 `MainApp_RunOnce(&app)`。
5. `TIM2_IRQHandler` 中读取捕获宽度后调用 `MainApp_OnCaptureIntervalUs(&app, interval_us)`。

## 后续适合继续加的功能

- 电池电压 ADC 采样
- 航速闭环或软启动曲线优化
- 双电机混控
- 超声避障或自动返航
- OLED 菜单和参数在线调节
