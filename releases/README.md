# Firmware Archives

The `Build Examples and Firmware` workflow packages each successful ESP-IDF and
Arduino source build as a directly flashable ZIP. Checked-in factory firmware is
not repackaged or uploaded as a generated CI artifact.

Every archive contains:

```text
README.md
SHA256SUMS
flash.sh
flash.bat
flash_args.txt
manifest.json
bin/*.bin
```

The flash helpers write one complete image at offset `0x0`. Install `esptool`
before using them:

```bash
python -m pip install esptool
./flash.sh /dev/ttyUSB0
```

On Windows use `flash.bat COMx`.

## Factory Firmware Boundary

The checked-in factory image, expected size, and SHA-256 digest are pinned in
`config/factory-firmware.json`. Verify the original file with:

```bash
python3 releases/validate_factory_firmware.py
```

The separate factory-integrity workflow checks the pinned size, digest, target,
offset, and Espressif image header. It does not build, package, or upload the
factory binary. Files below `firmware/sdcard/` are runtime SD-card content and
are deliberately excluded from every source-build flash archive.

## ESP-IDF Build Output

```bash
idf.py -C examples/esp-idf/03_lcd -B build/03_lcd set-target esp32s3 build
python3 releases/package_firmware.py \
  --framework esp-idf \
  --project examples/esp-idf/03_lcd \
  --build-dir build/03_lcd \
  --framework-version v6.0.2 \
  --git-sha "$(git rev-parse HEAD)"
```

The packager reads ESP-IDF's generated `flasher_args.json`, preserves every
source segment, and creates a combined image.

## Arduino Build Output

Use the FQBN from `config/ci.json` and compile with `--build-path` set to a stable
build directory, then run the packager with `--framework arduino` and the same
directory as `--build-dir`. The build path contains Arduino's bootloader,
partition table, OTA data initializer (`boot_app0.bin`), and application image.
The packager ignores the full-flash `.merged.bin` and creates a directly
flashable combined image that ends at the application's actual end offset
instead of padding it to the configured flash size.

The compact image does not change the configured 16 MB flash or partition
layout. Flashing it resets NVS and OTA state within the image range but leaves
later data partitions, including FATFS, untouched. Run
`python -m esptool --chip esp32s3 --port <PORT> erase-flash` first when a clean
install is required.

Pass `--git-sha "$(git rev-parse HEAD)"` for local packages so `manifest.json`
records the source commit. Set `SOURCE_DATE_EPOCH` to that commit's Unix
timestamp when reproducible ZIP bytes are required.

## Download CI Artifacts

Download all firmware artifacts from the latest successful workflow run on the
repository's default branch:

```bash
python3 releases/download_artifacts.py --clean
```

Pass `--branch <name>`, `--run-id <id>`, `--artifact <exact-name>`, or
`--pattern "firmware-esp-idf-*"` to select a branch, run, or artifact. If the
selected branch has no successful run, the error includes the latest run's
status and URL. Authentication is read from `GH_TOKEN`,
`GITHUB_TOKEN`, or `gh auth token`. Extracted packages are written below
`releases/downloads/`.
