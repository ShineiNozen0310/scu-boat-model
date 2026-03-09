# CubeMX / Keil 工程放在这里

建议把自动生成和板级启动代码放在本目录，不要和 `firmware/app/` 的手写业务逻辑混在一起。

推荐做法：

1. `main.c` 里完成时钟、GPIO、TIM、ADC、OLED 初始化。
2. 初始化完成后调用 `MainApp_Init(...)`。
3. 在主循环中调用 `MainApp_RunOnce(...)`。
4. 在 `TIM2_IRQHandler` 中读取捕获间隔后调用 `MainApp_OnCaptureIntervalUs(...)`。
5. 低压检测或独立急停输入可以通过 `MainApp_SetEmergencyStop(...)` 或 `battery_is_low` 回调接入。
