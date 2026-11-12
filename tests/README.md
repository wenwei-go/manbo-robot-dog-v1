# 固件验证

运行 `python tests/run.py`（依赖 PATH 中的 GCC）。测试直接编译真实 `RobotControl.c` 和 `Servo.c`，用最小 HAL 模拟验证：

- 全部 256 个指令值：有效动作、表情、次数和保留指令停止行为。
- 多字节顺序处理、DMA 半满/满缓冲/空闲事件、跨边界及重复通知。
- 接收回调不直接修改运动状态；队列溢出停止并能继续接收。
- USART1/USART3 共用指令队列，接收错误和启动失败后的重试。
- 原理图五路舵机对应通道，0～180° 读写往返、越界限幅、PWM 启动前的中位脉宽。

测试不打开摄像头、不连接蓝牙、不向设备发指令。ARM 编译仍需打开 `源码/MDK-ARM/Project.uvprojx`，选择 `Project` 执行 Rebuild。

2026-11-12 验证结果：主机测试通过；Keil ARM Compiler 5.06 update 5，完整重建 0 Error(s)、0 Warning(s)。Code=12592、RO-data=8504、RW-data=40、ZI-data=3072 字节，适合 STM32F103C8T6 的 64 KiB Flash / 20 KiB RAM。尚未进行实机烧录、示波器 PWM 测量或蓝牙/语音模块联调。
