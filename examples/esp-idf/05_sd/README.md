| Supported Targets | ESP32-S3 |
| ----------------- | -------- |

| Supported LCD Controller    | ST7262 |
| ----------------------------| -------|

## How to use the example

## ESP-IDF Required

### Hardware Required

* An Waveshare ESP32-S3-Touch-LCD-4.3 development board

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                              SD Card
+-----------------------+              +-------------------+
|                   GND +--------------+GND                |
|                       |              |                   |
|                   3V3 +--------------+VCC                |
|                       |              |                   |
|                 GPIO11+--------------+CMD                |
|                       |              |                   |
|                 GPIO12+--------------+CLK                |
|                       |              |                   |
|                 GPIO13+--------------+D0                 |
+-----------------------+              |                   |
            IO EXTENSIO.EXIO4+--------------+SD_CS              |
                                       |                   |
                                       +-------------------+
```

* Mount an SD card, output related parameters via serial,unmount the SD card, and display memory information on the screen.

### Configure the Project

### Build and Flash

Run `idf.py set-target esp32s3` to select the target chip.

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A fancy animation will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

For any technical queries, please open an https://service.waveshare.com/. We will get back to you soon.
