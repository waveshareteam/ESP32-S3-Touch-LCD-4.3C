# Shared Configuration

`ci.json` is the single source of truth for the CI framework versions, Arduino
CLI version, and ESP32-S3 N16R8 board options. `scripts/discover_examples.py`
validates and reads this file before producing GitHub Actions matrices.

`factory-firmware.json` identifies the complete factory flash image at offset
`0x0` and pins its size and SHA-256 digest. The files below
`firmware/sdcard/` are runtime SD-card content; they are not part of the flash
image. Factory integrity CI verifies the checked-in binary but does not publish
a generated archive.

Example-local `sdkconfig.defaults` files remain authoritative for ESP-IDF
project settings. Add a shared overlay here only when multiple projects consume
the same file.
