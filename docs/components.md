# Components

The examples currently preserve their original local driver layout. Multiple projects contain copies of the I2C, IO-expander, RGB LCD, GT911 touch, PCF85063A RTC, SDMMC, audio, LVGL-port, font, image, and UI components. Three ESP-IDF projects also carry full LVGL source trees.

This duplication is intentional for the first CI baseline: changing every example to a new board-support API at the same time would combine framework migration, LVGL-major migration, pin validation, and behavior changes in one review.

## Managed BSP Direction

The published board component is `waveshare/esp32_s3_touch_lcd_4_3c` version `3.0.1` for `esp32s3` and ESP-IDF 5.3 or newer. It provides a path toward managed display, touch, audio, RTC, and IO-expander dependencies. Its current dependency set uses LVGL 9.4, `esp_lvgl_adapter`, `esp_codec_dev`, GT911, PCF85063A, and the Waveshare CH32V003 IO-expander component.

Migrate examples incrementally:

1. Establish a passing ESP-IDF 5.5 and 6.0 build for the unchanged example.
2. Replace one reusable subsystem with the managed BSP or component.
3. Verify display timing, touch orientation, audio routing, SD access, and IO behavior on hardware.
4. Remove the local copy only after both framework versions and the board test pass.

Project-specific UI assets and demo glue should remain local. Reusable component fixes should be made in the shared component repository and consumed here through a released version.

## Known Hardware Naming Conflict

Existing example comments and READMEs call the IO expander `CH422G`, while the published BSP identifies the board expander as CH32V003. The local register protocol uses I2C address `0x24`. Do not rename or replace that driver until the target hardware revision and protocol are confirmed against the schematic and a physical board.
