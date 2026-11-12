# 曼波机器狗 V1

STM32F103C8T6 五舵机机器狗：OLED 表情、非阻塞动作控制、USART3 蓝牙透传，以及 PC MediaPipe 手势识别。

- [硬件引脚、存档差异与分支说明](hardware/引脚连接.md)
- [单字节通信协议](通信协议.md)
- [固件验证与测试](tests/README.md)

## 当前修复分支

`fix/hardware-pins-usart3` 基于 `feature/add-CV`。已按原理图修改舵机为 PA0/PA1/PA2/PA3/PA7，OLED 为 PB6/PB7；蓝牙保留 USART3 PB10/PB11，语音使用 USART1 PA9/PA10。语音侧暂按 115200 8N1、单字节输出配置，实际模块设置待核实。

舵机接口编号以 `Servo.h` 为准：1=左前、2=左后、3=右前、4=右后、5=尾巴。原存档中不同文件的腿位注释不一致，已统一；保留原编号 1/3 的角度反向，实际安装方向和中位需上板校验。

## 编译与烧录

打开 [Keil 工程](源码/MDK-ARM/Project.uvprojx)，选择 `Project` 执行 Rebuild（已用 ARM Compiler 5.06 update 5 验证）。CubeMX 配置在 [Project.ioc](源码/Project.ioc)，引脚和循环 DMA 配置已同步。

本分支中的 [Project.hex](源码/MDK-ARM/Project/Project.hex) 已重新生成，可用于后续烧录。链接起始地址为 `0x08000000`。仓库中其他旧 `.axf/.o/.map` 等是历史构建存档，调试时应先 Rebuild，使它们与当前源码一致。本次未连接设备或执行烧录。

## PC 手势控制

安装 `opencv-python`、`mediapipe`、`pyserial`，通过系统蓝牙配对获取 SPP COM 口，然后运行：

```powershell
python CV/hand_identify.py COM7
```

将 COM7 替换为实际蓝牙端口；电脑 COM 号与主控 USART3 编号无关。波特率 115200；张开五指、握拳、V 手势分别发送原始单字节 `0x21/0x22/0x23`。具体动作和防抖规则见通信协议。
