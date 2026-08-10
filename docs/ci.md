# Continuous Integration

The `Build Examples and Firmware` workflow discovers the repository's first-party projects with `scripts/discover_examples.py` and reads toolchain and board settings from `config/ci.json`.

## Supported Matrix

| Surface | Pinned version | Target | Entries |
| --- | --- | --- | --- |
| ESP-IDF | `v5.5.5` | `esp32s3` | 14 |
| ESP-IDF | `v6.0.2` | `esp32s3` | 14 |
| Arduino CLI / Arduino-ESP32 | `1.5.1` / `3.3.11` | ESP32S3 Dev Module | 15 |

The full run therefore contains 43 firmware build jobs, plus discovery jobs. These pins are intentionally machine-readable in `config/ci.json`; update that file and the related tests together.

## Triggers And Selection

Pull requests and pushes run the workflow when example source, shared build configuration, discovery code, workflow files, or release packaging code changes. Documentation-only and governance-only changes do not start the firmware matrix.

Manual dispatch accepts:

- `all` for the complete surface;
- an example name such as `03_lcd`;
- a repository-relative path such as `examples/esp-idf/03_lcd`.

An unknown selector produces an empty matrix and fails discovery instead of reporting a misleading successful build.

## Arduino Board Options

Arduino uses the ESP32S3 Dev Module FQBN with QIO 80 MHz flash, 16 MB flash, OPI PSRAM, `app3M_fat9M_16MB`, USB CDC enabled at boot, hardware CDC/JTAG upload, and 921600 baud. Bundled libraries under `examples/arduino/libraries/` are passed to the build but their own example sketches are excluded.

## Firmware Artifacts

Successful source-build jobs call `releases/package_firmware.py` and upload a ZIP containing `manifest.json`, `flash_args.txt`, `flash.sh`, `flash.bat`, a combined image at offset `0x0`, and the original binary segments under `bin/`. The separate `Validate Factory Firmware` workflow only checks the pinned factory image's metadata, digest, and image header; it does not package or upload that binary.

CI proves that source configures, compiles, and packages on the selected toolchain. Display timing, touch coordinates, audio routing, SD-card behavior, radio behavior, and IO levels still require physical-board validation.
