# ESP32-S3-Touch-LCD-4.3C Hardware Reference

This reference is derived from the repository's V1.1 schematic and cross-checked against the first-party example headers. It describes the ESP32-S3-WROOM-1-N16R8 board variant. Check the schematic under `hardware/schematics/` before adapting these assignments to another revision.

## Core Hardware

| Feature | Device / interface |
| --- | --- |
| MCU module | ESP32-S3-WROOM-1-N16R8, 16 MB flash, 8 MB OPI PSRAM |
| Display | 4.3-inch 800 x 480 RGB565 parallel panel |
| Touch | GT911 capacitive touch over I2C |
| RTC | PCF85063A over I2C |
| Speaker | ES8311 codec over I2S and I2C |
| Microphone | ES7210 ADC over I2S and I2C |
| Storage | microSD in one-bit SDMMC mode |
| IO expansion | Board controller at I2C address `0x24` |

## Shared I2C And Control Signals

| Signal | Assignment |
| --- | --- |
| I2C SDA | GPIO8 |
| I2C SCL | GPIO9 |
| Bus frequency used by examples | 400 kHz |
| Touch interrupt | GPIO4 |
| Touch reset | IO-expander IO1 |
| LCD backlight | IO-expander IO2 |
| Power amplifier enable | IO-expander IO3 |
| SD-card control | IO-expander IO4 |

The GT911 examples use address `0x5D`, with `0x14` documented as the alternate strap address. The existing code and the published managed BSP use different names for the IO-expander device; see `docs/components.md` before changing its driver.

## RGB Display

| Signal | GPIO | Signal | GPIO |
| --- | --- | --- | --- |
| VSYNC | 3 | HSYNC | 46 |
| DE | 5 | PCLK | 7 |
| B3..B7 | 14, 38, 18, 17, 10 | G2..G7 | 39, 0, 45, 48, 47, 21 |
| R3..R7 | 1, 2, 42, 41, 40 | Backlight | IO-expander IO2 |

The example timing uses a 16 MHz pixel clock and two RGB framebuffers. Treat timing changes as hardware-facing changes and test them on the panel.

## Audio And Storage

| Function | Assignment |
| --- | --- |
| I2S MCLK | GPIO6 |
| I2S BCLK | GPIO44 |
| I2S LRCK/WS | GPIO16 |
| I2S data to speaker | GPIO15 |
| I2S data from microphone | GPIO43 |
| SDMMC CLK | GPIO12 |
| SDMMC CMD | GPIO11 |
| SDMMC D0 | GPIO13 |

Several legacy example READMEs showed audio MCLK as GPIO4. The V1.1 schematic and source code agree on GPIO6; GPIO4 is the GT911 interrupt signal.

## Validation Scope

The assignments above were checked against local hardware references and source. Automated CI only validates compilation. Physical-board testing is still required for display timing and color order, touch orientation, codec gain and routing, SD-card integrity, isolated IO polarity, and behavior across hardware revisions.
