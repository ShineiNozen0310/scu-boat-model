# CRSF UART Integration

更新日期: 2026-03-11

## 1. 当前固件入口

当前应用层已经新增两个 CRSF 入口:

- `MainApp_OnCrsfByte(&app, rx_byte)`
- `MainApp_OnCrsfFrame(&app, frame_buf, frame_len)`

相关文件:

- `firmware/app/drivers/boat_crsf.h`
- `firmware/app/drivers/boat_crsf.c`
- `firmware/app/main_app.h`
- `firmware/app/main_app.c`

设计意图:

- 如果你现在用 `UART IRQ` 单字节接收, 直接在回调里喂 `MainApp_OnCrsfByte(...)`
- 如果你已经拿到一段连续缓冲区, 现在也可以直接喂 `MainApp_OnCrsfBytes(...)`
- 如果你后面改成 `DMA + IDLE`, 可以把一整帧喂给 `MainApp_OnCrsfFrame(...)`

## 2. CubeMX 配置建议

接收机侧按 ELRS/CRSF 方式接入时, 建议:

- 使用独立 UART
- 关闭反相
- 采用全双工 UART
- UART 串口速率和接收机当前 CRSF 串口设置保持一致

说明:

- TBS CRSF 公开协议里常见默认值是 `416666 baud`
- ExpressLRS 文档里常见集成示例是 `420000 baud`

工程上建议:

- 你如果用的是 `ELRS -> RP1 V2 -> CRSF`, 先按 `420000` 配
- 如果你后面换严格按 TBS 默认的设备, 再核对到 `416666`

## 3. 默认通道映射

默认映射写在 `firmware/app/boat_config.h`:

- `CH1`: 方向舵
- `CH2`: 炮塔俯仰
- `CH3`: 油门
- `CH4`: 炮塔水平
- `CH5`: 水炮开关
- `CH6`: 灯光开关
- `CH7`: 急停开关

当前开关判定阈值:

- 大于等于 `1700us` 视为打开

当前摇杆缩放:

- 中位 `1500us`
- 最小 `1000us`
- 最大 `2000us`
- 死区 `20us`

对于 `RadioMaster Pocket / EdgeTX Mode 2`, 建议对应:

- `CH1 <- Ail`
- `CH2 <- Ele`
- `CH3 <- Thr`
- `CH4 <- Rud`

如果你的手台通道顺序不同, 只改 `boat_config.h` 宏定义即可。

## 4. 推荐接线

以 `STM32F103 + RP1 V2` 为例:

- `RP1 5V` -> `UBEC 5V`
- `RP1 GND` -> `MCU GND`
- `RP1 TX` -> `MCU UART RX`
- `RP1 RX` -> `MCU UART TX`

说明:

- 即使当前你只打算“接收遥控”, 也建议把 `RX/TX` 双线都接上
- 这样后面如果你要做 CRSF 遥测、链路配置或更完整兼容, 不用重新改线

## 5. CubeMX 集成示例

### 5.1 单字节中断方式

```c
static uint8_t g_crsf_rx_byte;

void App_StartCrsfRx(void)
{
    HAL_UART_Receive_IT(&huart1, &g_crsf_rx_byte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        (void)MainApp_OnCrsfByte(&g_main_app, g_crsf_rx_byte);
        HAL_UART_Receive_IT(&huart1, &g_crsf_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        HAL_UART_Receive_IT(&huart1, &g_crsf_rx_byte, 1);
    }
}
```

### 5.2 DMA + IDLE 方式

```c
static uint8_t g_crsf_dma_buf[64];

void App_StartCrsfRx(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, g_crsf_dma_buf, sizeof(g_crsf_dma_buf));
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart != &huart1)
    {
        return;
    }

    (void)MainApp_OnCrsfBytes(&g_main_app, g_crsf_dma_buf, size);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, g_crsf_dma_buf, sizeof(g_crsf_dma_buf));
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}
```

说明:

- 当前应用层已经自己做了 CRSF 帧拼接, 所以 DMA 回调里逐字节喂就可以
- 如果你后面想自己在板级代码里先切完整帧, 也可以直接改喂 `MainApp_OnCrsfFrame(...)`

## 6. 当前实现范围

当前版本已经支持:

- CRSF 流式字节解析
- `RC_CHANNELS_PACKED (0x16)` 帧校验
- CRC8 校验
- 16 路 11-bit 通道安全解包
- 映射到 `BoatCommand`
- 用急停通道直接驱动 `MainApp_SetEmergencyStop(...)`
- 板级 `UART` 错误回调后自动重启接收

当前版本还没有做:

- CRSF 遥测回传
- 链路质量显示
- 接收机配置透传
- 动态通道反向/校准

## 7. 参考来源

- ExpressLRS serial protocols: https://www.expresslrs.org/software/serial-protocols/
- ExpressLRS ArduPilot setup: https://www.expresslrs.org/quick-start/ardupilot-setup/
- ExpressLRS source, CRSF constants: https://raw.githubusercontent.com/ExpressLRS/ExpressLRS/master/src/lib/CrsfProtocol/crsf_protocol.h
- Betaflight CRSF source: https://github.com/betaflight/betaflight/blob/master/src/main/rx/crsf.c
