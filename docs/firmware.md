# Firmware And Factory Recovery

This repository contains two distinct firmware classes.

## Source-Built Example Packages

CI and `releases/package_firmware.py` create flashable ZIP archives from successful ESP-IDF and Arduino builds. Each archive contains:

```text
manifest.json
flash_args.txt
flash.sh
flash.bat
bin/<combined image>
bin/<original binary segments>
```

After extracting an archive, install esptool and flash through the included helper:

```text
python -m pip install esptool
flash.bat COMx
./flash.sh /dev/ttyACM0
```

The combined image is written at offset `0x0`. The manifest records exact offsets, framework version, target, project path, source commit, baud rate, hashes, and the equivalent esptool command.

## Factory Recovery Firmware

`firmware/ESP32-S3-Touch-LCD-4.3C-Test.bin` is the complete factory test/recovery image. Its size, SHA-256 digest, target, and flash offset are recorded in `config/factory-firmware.json`. CI does not rebuild or repackage it. The separate factory-integrity workflow validates the pinned file and uploads nothing.

The files below `firmware/sdcard/` are runtime media for the factory demonstration. Write them to a FAT-formatted microSD card while preserving the `image/` and `music/` directory names. They are not embedded in the recovery image.

Source and build instructions for the factory image are not included in this repository yet and may be added in a later update.

Run `python3 releases/validate_factory_firmware.py` to verify the checked-in image before flashing it. Generated source-build archives and downloaded artifacts belong under ignored output directories. Do not commit them as new factory firmware.
