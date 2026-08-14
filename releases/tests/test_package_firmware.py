from __future__ import annotations

import json
import os
import subprocess
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path


RELEASES_DIR = Path(__file__).resolve().parents[1]
PACKAGE_SCRIPT = RELEASES_DIR / "package_firmware.py"
VALIDATE_SCRIPT = RELEASES_DIR / "validate_firmware.py"


class PackageFirmwareTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_script(self, script: Path, *arguments: str, expect_success: bool = True) -> subprocess.CompletedProcess[str]:
        result = subprocess.run(
            [sys.executable, str(script), *arguments],
            cwd=self.repo,
            capture_output=True,
            text=True,
            env={**os.environ, "SOURCE_DATE_EPOCH": "0"},
        )
        if expect_success and result.returncode != 0:
            self.fail(f"{script.name} failed:\nstdout: {result.stdout}\nstderr: {result.stderr}")
        return result

    def create_esp_idf_build(self) -> tuple[Path, Path]:
        project = self.repo / "examples/esp-idf/demo"
        project.mkdir(parents=True)
        build = self.repo / "build"
        (build / "bootloader").mkdir(parents=True)
        (build / "partition_table").mkdir()
        (build / "bootloader/bootloader.bin").write_bytes(b"\xe9" + b"B" * 31)
        (build / "partition_table/partition-table.bin").write_bytes(b"P" * 32)
        (build / "demo.bin").write_bytes(b"A" * 64)
        (build / "flasher_args.json").write_text(
            json.dumps(
                {
                    "flash_files": {
                        "0x0": "bootloader/bootloader.bin",
                        "0x8000": "partition_table/partition-table.bin",
                        "0x10000": "demo.bin",
                    },
                    "extra_esptool_args": {"chip": "esp32s3"},
                }
            ),
            encoding="utf-8",
        )
        return project, build

    def test_esp_idf_segments_are_combined_and_validated(self) -> None:
        self.create_esp_idf_build()

        self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "esp-idf",
            "--project",
            "examples/esp-idf/demo",
            "--build-dir",
            "build",
            "--name",
            "idf-demo",
            "--output-dir",
            "out",
            "--framework-version",
            "v6.0.2",
            "--git-sha",
            "0123456789abcdef",
        )
        artifact_name = "idf-demo-0123456"
        archive = self.repo / f"out/{artifact_name}.zip"
        self.run_script(VALIDATE_SCRIPT, str(archive))
        with zipfile.ZipFile(archive) as package:
            combined_name = f"{artifact_name}/bin/{artifact_name}.combined.bin"
            self.assertEqual(len(package.read(combined_name)), 0x10000 + 64)
            manifest = json.loads(package.read(f"{artifact_name}/manifest.json"))
            self.assertEqual(manifest["project_path"], "examples/esp-idf/demo")
            self.assertEqual(manifest["timestamp_utc"], "1970-01-01T00:00:00Z")
            self.assertEqual(manifest["git_sha"], "0123456789abcdef")
            self.assertEqual(len(manifest["files"]), 4)
            self.assertTrue(all(info.date_time == (1980, 1, 1, 0, 0, 0) for info in package.infolist()))

    def test_arduino_exported_binaries_are_combined_and_validated(self) -> None:
        project = self.repo / "examples/arduino/demo"
        project.mkdir(parents=True)
        (project / "demo.ino").write_text("void setup() {}\nvoid loop() {}\n", encoding="utf-8")
        build = self.repo / "arduino-build"
        build.mkdir()
        (build / "demo.ino.bootloader.bin").write_bytes(b"\xe9" + b"B" * 31)
        (build / "demo.ino.partitions.bin").write_bytes(b"P" * 32)
        (build / "boot_app0.bin").write_bytes(b"O" * 32)
        (build / "demo.ino.bin").write_bytes(b"A" * 64)
        with (build / "demo.ino.merged.bin").open("wb") as merged:
            merged.write(b"\xe9")
            merged.truncate(16 * 1024 * 1024)

        self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "arduino",
            "--project",
            "examples/arduino/demo",
            "--build-dir",
            "arduino-build",
            "--name",
            "arduino-demo",
            "--output-dir",
            "out",
            "--framework-version",
            "3.3.11",
        )

        archive = self.repo / "out/arduino-demo.zip"
        self.run_script(VALIDATE_SCRIPT, str(archive))
        with zipfile.ZipFile(archive) as package:
            manifest = json.loads(package.read("arduino-demo/manifest.json"))
            self.assertEqual(manifest["framework"], "arduino")
            combined = package.read(f"arduino-demo/{manifest['combined_bin']}")
            self.assertEqual(len(combined), 0x10000 + 64)
            self.assertEqual(
                {record["offset"] for record in manifest["segments"]},
                {"0x0", "0x8000", "0xe000", "0x10000"},
            )
            self.assertEqual(len(manifest["files"]), 5)
            self.assertFalse(
                any(record["file"].endswith(".merged.bin") for record in manifest["files"])
            )

    def test_arduino_merged_image_without_source_segments_is_rejected(self) -> None:
        project = self.repo / "examples/arduino/demo"
        project.mkdir(parents=True)
        build = self.repo / "arduino-build"
        build.mkdir()
        (build / "demo.ino.merged.bin").write_bytes(b"\xe9" + b"M" * 31)

        result = self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "arduino",
            "--project",
            "examples/arduino/demo",
            "--build-dir",
            "arduino-build",
            expect_success=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("expected one Arduino application binary, found 0", result.stderr)

    def test_arduino_missing_required_segment_is_rejected(self) -> None:
        project = self.repo / "examples/arduino/demo"
        project.mkdir(parents=True)
        build = self.repo / "arduino-build"
        build.mkdir()
        (build / "demo.ino.bootloader.bin").write_bytes(b"\xe9" + b"B" * 31)
        (build / "boot_app0.bin").write_bytes(b"O" * 32)
        (build / "demo.ino.bin").write_bytes(b"A" * 64)

        result = self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "arduino",
            "--project",
            "examples/arduino/demo",
            "--build-dir",
            "arduino-build",
            expect_success=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn(
            "missing required Arduino firmware segments at offsets: 0x8000",
            result.stderr,
        )

    def test_arduino_empty_required_segment_is_rejected(self) -> None:
        project = self.repo / "examples/arduino/demo"
        project.mkdir(parents=True)
        build = self.repo / "arduino-build"
        build.mkdir()
        (build / "demo.ino.bootloader.bin").write_bytes(b"\xe9" + b"B" * 31)
        (build / "demo.ino.partitions.bin").write_bytes(b"P" * 32)
        (build / "boot_app0.bin").write_bytes(b"O" * 32)
        (build / "demo.ino.bin").write_bytes(b"")

        result = self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "arduino",
            "--project",
            "examples/arduino/demo",
            "--build-dir",
            "arduino-build",
            expect_success=False,
        )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("empty Arduino firmware segments at offsets: 0x10000", result.stderr)

    def test_same_inputs_produce_identical_archives(self) -> None:
        self.create_esp_idf_build()
        common_arguments = (
            "--repo",
            str(self.repo),
            "--framework",
            "esp-idf",
            "--project",
            "examples/esp-idf/demo",
            "--build-dir",
            "build",
            "--name",
            "reproducible",
            "--framework-version",
            "v6.0.2",
            "--git-sha",
            "0123456789abcdef",
        )

        self.run_script(PACKAGE_SCRIPT, *common_arguments, "--output-dir", "out-one")
        self.run_script(PACKAGE_SCRIPT, *common_arguments, "--output-dir", "out-two")

        archive_name = "reproducible-0123456.zip"
        first = (self.repo / "out-one" / archive_name).read_bytes()
        second = (self.repo / "out-two" / archive_name).read_bytes()
        self.assertEqual(first, second)

    def test_packaging_never_removes_a_preexisting_named_directory(self) -> None:
        self.create_esp_idf_build()
        protected = self.repo / "docs"
        protected.mkdir()
        sentinel = protected / "keep.txt"
        sentinel.write_text("keep", encoding="utf-8")

        self.run_script(
            PACKAGE_SCRIPT,
            "--repo",
            str(self.repo),
            "--framework",
            "esp-idf",
            "--project",
            "examples/esp-idf/demo",
            "--build-dir",
            "build",
            "--name",
            "docs",
            "--output-dir",
            ".",
        )

        self.assertEqual(sentinel.read_text(encoding="utf-8"), "keep")
        self.assertTrue((self.repo / "docs.zip").is_file())

    def test_project_outside_repository_is_rejected(self) -> None:
        self.create_esp_idf_build()
        with tempfile.TemporaryDirectory() as outside_value:
            outside = Path(outside_value)
            result = self.run_script(
                PACKAGE_SCRIPT,
                "--repo",
                str(self.repo),
                "--framework",
                "esp-idf",
                "--project",
                str(outside),
                "--build-dir",
                "build",
                expect_success=False,
            )

        self.assertNotEqual(result.returncode, 0)
        self.assertIn("inside the repository", result.stderr)

    def test_factory_is_not_a_source_build_framework(self) -> None:
        result = self.run_script(
            PACKAGE_SCRIPT,
            "--framework",
            "factory",
            expect_success=False,
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("invalid choice", result.stderr)


if __name__ == "__main__":
    unittest.main()
