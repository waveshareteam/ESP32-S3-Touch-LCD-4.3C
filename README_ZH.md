# Waveshare ESP32-S3-Touch-LCD-4.3C

[English](README.md)

ESP32-S3-Touch-LCD-4.3C 是微雪推出的一款低成本、高性能开发板，搭载 ESP32-S3R8（双核 Xtensa LX7 @ 240 MHz，8 MB PSRAM，16 MB Flash），支持 2.4 GHz Wi-Fi 和 Bluetooth 5 LE。板载 4.3 英寸 800 × 480 电容触摸屏和音频模块（ES8311 + ES7210），并集成 RTC、TF 卡槽、USB Type-C、隔离数字 I/O 及电池接口，可流畅运行 LVGL 图形界面并支持 AI 语音交互，适用于物联网、移动设备及智能家居等应用。

- [购买链接](https://www.waveshare.net/shop/ESP32-S3-Touch-LCD-4.3C.htm)
- [产品文档](https://docs.waveshare.net/ESP32-S3-Touch-LCD-4.3C/)
- [硬件参考](HARDWARE_REFERENCE_ZH.md)

<img src="assets/Product-1.webp" alt="Waveshare ESP32-S3-Touch-LCD-4.3C" width="500">

## 概述

本仓库提供微雪 ESP32-S3-Touch-LCD-4.3C 的 ESP-IDF 与 Arduino 官方示例、工厂恢复固件、SD 卡资源、原理图和结构文件。

当前支持的硬件版本采用 ESP32-S3-WROOM-1-N16R8 模组，带 16 MB Flash 和 8 MB OPI PSRAM。板载 800 x 480 RGB 显示屏、GT911 电容触摸、PCF85063A RTC、ES8311 扬声器编解码器、ES7210 麦克风 ADC、microSD、Wi-Fi 和 Bluetooth LE。

## 快速开始

选择开发框架，并阅读对应示例目录中的 README：

- ESP-IDF 工程：[`examples/esp-idf/`](examples/esp-idf/)
- Arduino 示例与仓库自带库：[`examples/arduino/`](examples/arduino/)
- 工厂恢复固件与 SD 卡资源：[`firmware/`](firmware/)

ESP-IDF 工程使用各自的 `sdkconfig.defaults`。Arduino 使用 ESP32S3 Dev Module，配置为 QIO 80 MHz、16 MB Flash、OPI PSRAM、`app3M_fat9M_16MB` 分区、USB CDC/JTAG 和 921600 上传波特率。CI 使用的完整板卡字符串保存在 [`config/ci.json`](config/ci.json)。

## 示例

| ESP-IDF 工程 | 功能 |
| --- | --- |
| `01_i2c` | 共用 I2C 总线与 IO 扩展器 |
| `02_rtc` | PCF85063A 实时时钟 |
| `03_lcd` | RGB LCD 初始化 |
| `04_isolation_io` | 隔离数字输入输出 |
| `05_sd` | SDMMC microSD |
| `06_touch` | GT911 电容触摸 |
| `07_display_bmp` | 从 microSD 显示位图 |
| `08_wifi_scan` | Wi-Fi 扫描 |
| `09_wifi_sta` | Wi-Fi STA 模式 |
| `10_wifi_ap` | Wi-Fi AP 模式 |
| `11_speaker_microphone` | ES8311 扬声器与 ES7210 麦克风 |
| `12_lvgl_transplant` | LVGL 集成 |
| `13_lvgl_codec` | LVGL 音频播放器 |
| `14_udp_tcp_ntp` | UDP、TCP 与 NTP 仪表界面 |

| Arduino 示例 | 功能 |
| --- | --- |
| `01_i2c` | 共用 I2C 总线与 IO 扩展器 |
| `02_rtc` | PCF85063A 实时时钟 |
| `03_lcd` | RGB LCD 初始化 |
| `04_isolation_io` | 隔离数字输入输出 |
| `05_sd` | microSD 访问 |
| `06_touch` | GT911 电容触摸 |
| `07_display_bmp` | 从 microSD 显示位图 |
| `08_wifi_scan` | Wi-Fi 扫描 |
| `09_wifi_sta` | Wi-Fi STA 模式 |
| `10_wifi_ap` | Wi-Fi AP 模式 |
| `11_speaker_microphone` | ES8311 扬声器与 ES7210 麦克风 |
| `12_lvgl_transplant` | LVGL 集成 |
| `13_lvgl_btn` | LVGL 按钮交互 |
| `14_lvgl_slider` | LVGL 滑块交互 |
| `15_udp_tcp_ntp` | UDP、TCP 与 NTP 仪表界面 |

第一方 Arduino 示例直接位于 [`examples/arduino/`](examples/arduino/)。仓库自带库内部的示例不会进入产品 CI。

## 持续集成

示例工作流会自动发现工程，不维护硬编码列表。完整矩阵包括两个 ESP-IDF 稳定版本上的 28 个构建，以及 15 个 Arduino 构建。每个成功的源码构建都会打包为可直接烧录的归档，其中包含清单、原始分段固件、合并固件，以及 Windows/POSIX 烧录脚本。

有关手动触发选择器、工具链版本、构件内容与烧录方法，请参阅[持续集成](docs/ci.md)和[固件与工厂恢复](docs/firmware.md)。CI 只验证编译和打包，不能替代真机测试。

## 仓库结构

| 路径 | 用途 |
| --- | --- |
| `examples/esp-idf/` | 首方 ESP-IDF 工程 |
| `examples/arduino/` | 首方 Arduino 示例与仓库自带库 |
| `config/` | CI 版本、板卡选项与工厂固件元数据 |
| `scripts/` | 示例发现与校验工具 |
| `releases/` | 源码固件打包与构件下载工具 |
| `firmware/` | 工厂恢复固件与 SD 卡运行资源 |
| `hardware/` | 原理图与结构文件 |
| `docs/` | CI、组件、固件与仓库维护说明 |

目录职责与生成文件边界见[仓库结构](docs/repository-structure.md)。

## 文档

- [硬件参考](HARDWARE_REFERENCE_ZH.md)
- [持续集成](docs/ci.md)
- [组件说明](docs/components.md)
- [固件与工厂恢复](docs/firmware.md)
- [贡献指南](CONTRIBUTING.md)
- [技术支持](SUPPORT.md)
- [安全策略](SECURITY.md)

## 许可证

本仓库采用 Apache License 2.0，详见 [LICENSE](LICENSE)。
