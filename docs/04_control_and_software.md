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

## 结合宣讲 PPT 补充的控制建议

### 1. 打靶控制应优先使用比例控制

宣讲 PPT 里“云台两轴 + 水枪启停”的结构，对当前工程是直接适用的。对这个项目，建议把打靶控制固定为:

- 航行控制: `throttle + rudder`
- 炮塔控制: `turret_yaw + turret_pitch`
- 触发控制: `water_cannon_enabled`

这和当前命令结构一致:

- `BoatCommand.throttle_percent`
- `BoatCommand.rudder_percent`
- `BoatCommand.turret_yaw_percent`
- `BoatCommand.turret_pitch_percent`
- `BoatCommand.flags`

如果后面从红外彻底切到自制发射端或 `CRSF`, 炮塔控制建议优先使用比例摇杆而不是离散按键。原因很简单:

- 手动瞄准更顺手
- 打靶时修正角度更快
- 可以减少“按一下走一格”的过冲

### 2. 云台控制必须保留中位校准和角度限幅

宣讲 PPT 提到“中位校准”和“角度限幅”，这两点对打靶机构很关键，应该继续保留在工程里：

- 船上电后云台先回中
- 水平轴和俯仰轴都做角度上限 / 下限
- 发射端不要直接把摇杆值原样透传成脉宽
- 先映射成归一化控制量, 再由船端统一限幅输出

当前工程已经按这个方向做了:

- 角度边界集中定义在 `boat_config.h`
- 控制量统一先进入 `BoatCommand`
- `BoatController_ApplyCommand()` 再映射到各个执行器

### 3. 水枪触发先做开关量, 不急着做复杂射频节奏

宣讲 PPT 里提到水枪可做“单发或连续喷水”。对你现在这个工程，建议先收敛成:

- 第一版: `GPIO/MOSFET` 开关量启停
- 第二版: 如有必要再做点射节奏

这样更符合当前项目阶段:

- 先把船开稳
- 先把云台转向和触发可靠性做好
- 后面再谈更复杂的开火模式
