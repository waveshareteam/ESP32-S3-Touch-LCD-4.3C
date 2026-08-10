# ESP32-S3-Touch-LCD-4.3C 硬件参考

本文档依据仓库中的 V1.1 原理图整理，并与首方示例头文件交叉核对，适用于 ESP32-S3-WROOM-1-N16R8 版本。用于其他硬件修订版前，请先检查 `hardware/schematics/` 中的原理图。

## 核心硬件

| 功能 | 器件 / 接口 |
| --- | --- |
| MCU 模组 | ESP32-S3-WROOM-1-N16R8，16 MB Flash，8 MB OPI PSRAM |
| 显示 | 4.3 英寸 800 x 480 RGB565 并行屏 |
| 触摸 | GT911，I2C 接口 |
| RTC | PCF85063A，I2C 接口 |
| 扬声器 | ES8311，I2S 与 I2C 接口 |
| 麦克风 | ES7210，I2S 与 I2C 接口 |
| 存储 | microSD，单线 SDMMC 模式 |
| IO 扩展 | I2C 地址 `0x24` 的板载控制器 |

## 共用 I2C 与控制信号

| 信号 | 分配 |
| --- | --- |
| I2C SDA | GPIO8 |
| I2C SCL | GPIO9 |
| 示例使用的总线频率 | 400 kHz |
| 触摸中断 | GPIO4 |
| 触摸复位 | IO 扩展器 IO1 |
| LCD 背光 | IO 扩展器 IO2 |
| 功放使能 | IO 扩展器 IO3 |
| SD 卡控制 | IO 扩展器 IO4 |

GT911 示例使用 `0x5D` 地址，并将 `0x14` 记录为另一种上电地址。现有代码与已发布托管 BSP 对 IO 扩展器的命名不同，修改驱动前请先阅读 `docs/components.md`。

## RGB 显示

| 信号 | GPIO | 信号 | GPIO |
| --- | --- | --- | --- |
| VSYNC | 3 | HSYNC | 46 |
| DE | 5 | PCLK | 7 |
| B3..B7 | 14, 38, 18, 17, 10 | G2..G7 | 39, 0, 45, 48, 47, 21 |
| R3..R7 | 1, 2, 42, 41, 40 | 背光 | IO 扩展器 IO2 |

示例使用 16 MHz 像素时钟和两个 RGB 帧缓冲区。显示时序属于硬件相关配置，修改后必须在实物屏幕上验证。

## 音频与存储

| 功能 | 分配 |
| --- | --- |
| I2S MCLK | GPIO6 |
| I2S BCLK | GPIO44 |
| I2S LRCK/WS | GPIO16 |
| I2S 扬声器数据 | GPIO15 |
| I2S 麦克风数据 | GPIO43 |
| SDMMC CLK | GPIO12 |
| SDMMC CMD | GPIO11 |
| SDMMC D0 | GPIO13 |

部分旧示例 README 将音频 MCLK 写为 GPIO4。V1.1 原理图与源代码一致使用 GPIO6；GPIO4 实际为 GT911 中断信号。

## 验证范围

以上分配已与本地硬件资料和源码核对。自动 CI 只能验证编译；显示时序与色序、触摸方向、编解码器增益与路由、SD 卡完整性、隔离 IO 极性，以及不同硬件修订版的行为仍需真机测试。
