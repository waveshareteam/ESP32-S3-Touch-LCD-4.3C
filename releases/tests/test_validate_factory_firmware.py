from __future__ import annotations

import hashlib
import json
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


RELEASES_DIR = Path(__file__).resolve().parents[1]
VALIDATOR = RELEASES_DIR / "validate_factory_firmware.py"
sys.path.insert(0, str(RELEASES_DIR))
import validate_factory_firmware  # noqa: E402


class ValidateFactoryFirmwareTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        self.image = self.repo / "firmware/factory.bin"
        self.image.parent.mkdir(parents=True)
        self.image.write_bytes(b"\xe9" + bytes(range(1, 128)))
        self.config_path = self.repo / "config/factory-firmware.json"
        self.config_path.parent.mkdir()
        self.config = {
            "schema_version": 1,
            "name": "test-factory",
            "board": "test-board",
            "hardware_variant": "N16R8",
            "target": "esp32s3",
            "image": "firmware/factory.bin",
            "offset": "0x0",
            "size": self.image.stat().st_size,
            "sha256": hashlib.sha256(self.image.read_bytes()).hexdigest(),
            "sdcard_resources_included": False,
        }
        self.write_config()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def write_config(self) -> None:
        self.config_path.write_text(json.dumps(self.config), encoding="utf-8")

    def test_valid_image_and_cli(self) -> None:
        result = validate_factory_firmware.validate_factory_firmware(self.repo)
        self.assertEqual(result["image"], "firmware/factory.bin")
        self.assertEqual(result["target"], "esp32s3")

        process = subprocess.run(
            [sys.executable, str(VALIDATOR), "--repo", str(self.repo)],
            capture_output=True,
            text=True,
        )
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn("valid: firmware/factory.bin", process.stdout)

    def test_rejects_invalid_schema_fields_and_board_contract(self) -> None:
        cases = {
            "missing field": ("board", None),
            "schema": ("schema_version", 2),
            "target": ("target", "esp32"),
            "offset": ("offset", "0x1000"),
            "sdcard flag": ("sdcard_resources_included", True),
        }
        for name, (field, value) in cases.items():
            with self.subTest(name=name):
                original = self.config.copy()
                if value is None:
                    self.config.pop(field)
                else:
                    self.config[field] = value
                self.write_config()
                with self.assertRaises(ValueError):
                    validate_factory_firmware.validate_factory_firmware(self.repo)
                self.config = original

    def test_rejects_integrity_mismatches(self) -> None:
        self.config["size"] += 1
        self.write_config()
        with self.assertRaisesRegex(ValueError, "size mismatch"):
            validate_factory_firmware.validate_factory_firmware(self.repo)

        self.config["size"] = self.image.stat().st_size
        self.config["sha256"] = "0" * 64
        self.write_config()
        with self.assertRaisesRegex(ValueError, "SHA-256 mismatch"):
            validate_factory_firmware.validate_factory_firmware(self.repo)

        self.image.write_bytes(b"\x00" + self.image.read_bytes()[1:])
        self.config["sha256"] = hashlib.sha256(self.image.read_bytes()).hexdigest()
        self.write_config()
        with self.assertRaisesRegex(ValueError, "image header"):
            validate_factory_firmware.validate_factory_firmware(self.repo)

    def test_rejects_config_or_image_outside_repository(self) -> None:
        with tempfile.TemporaryDirectory() as outside_value:
            outside = Path(outside_value)
            outside_config = outside / "factory.json"
            outside_config.write_text(json.dumps(self.config), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "configuration must be inside"):
                validate_factory_firmware.validate_factory_firmware(
                    self.repo, outside_config
                )

            outside_image = outside / "factory.bin"
            outside_image.write_bytes(self.image.read_bytes())
            self.config["image"] = str(outside_image)
            self.write_config()
            with self.assertRaisesRegex(ValueError, "image must be inside"):
                validate_factory_firmware.validate_factory_firmware(self.repo)

    def test_rejects_sdcard_runtime_file_as_factory_image(self) -> None:
        sdcard_image = self.repo / "firmware/sdcard/factory.bin"
        sdcard_image.parent.mkdir(parents=True)
        sdcard_image.write_bytes(self.image.read_bytes())
        self.config["image"] = "firmware/sdcard/factory.bin"
        self.write_config()

        with self.assertRaisesRegex(ValueError, r"top-level firmware/\*\.bin"):
            validate_factory_firmware.validate_factory_firmware(self.repo)

    def test_rejects_uppercase_bin_extension(self) -> None:
        uppercase_image = self.repo / "firmware/factory.BIN"
        uppercase_image.write_bytes(self.image.read_bytes())
        self.config["image"] = "firmware/factory.BIN"
        self.write_config()

        with self.assertRaisesRegex(ValueError, r"top-level firmware/\*\.bin"):
            validate_factory_firmware.validate_factory_firmware(self.repo)


if __name__ == "__main__":
    unittest.main()
