| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

| Supported LCD Controller    | ST7262 |
| ----------------------------| -------|

## How to use the example

## ESP-IDF Required

### Hardware Required

* An Waveshare ESP32-S3-Touch-LCD-4.3C development board

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                           RGB  Panel
+-----------------------+              +-------------------+
|                   GND +--------------+GND                |
|                       |              |                   |
|                   3V3 +--------------+VCC                |
|                       |              |                   |
|                   PCLK+--------------+PCLK               |
|                       |              |                   |
|             DATA[15:0]+--------------+DATA[15:0]         |
|                       |              |                   |
|                  HSYNC+--------------+HSYNC              |
|                       |              |                   |
|                  VSYNC+--------------+VSYNC              |
|                       |              |                   |
|                     DE+--------------+DE                 |
|                       |              |                   |
|               BK_LIGHT+--------------+BLK                |
+-----------------------+              |                   |
       IO EXTENSION_Init.EXIO2+--------------+DISP_EN      |
                                       |                   |
                                       +-------------------+
```

* Create an Access Point (AP), and when devices connect, 
* display their MAC addresses on the screen.

*使用汉字的朋友需要注意，如果用vs code打开，需要切换编码方式，修改成GB2312才能正常显示
*Just use the characters and ignore the Chinese characters above.

### Configure the Project

### Build and Flash

Run `idf.py set-target esp32s3` to select the target chip.

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A fancy animation will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

For any technical queries, please open an https://service.waveshare.com/. We will get back to you soon.
