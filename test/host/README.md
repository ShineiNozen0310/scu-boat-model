# Host Tests

脱离 STM32 HAL 库的环境下进行主机侧（Host）的模块验证和逻辑自测。

## 核心模块

- `crsf_host_selftest.c`: 测试 CRSF 通道解包、流式喂包通道映射、预算无线链路 `radio packet` 序号处理，以及急停和安全断连保护机制。
