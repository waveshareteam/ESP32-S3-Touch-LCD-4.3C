from __future__ import annotations

import importlib.util
import json
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).resolve().parents[1] / "validate_release_artifacts.py"
SPEC = importlib.util.spec_from_file_location("validate_release_artifacts", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class ValidateReleaseArtifactsTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        self.artifacts = self.repo / "artifacts"
        self.artifacts.mkdir()
        idf = self.repo / "examples/esp-idf/01_idf"
        idf.mkdir(parents=True)
        (idf / "CMakeLists.txt").touch()
        arduino = self.repo / "examples/arduino/01_arduino"
        arduino.mkdir(parents=True)
        (arduino / "01_arduino.ino").touch()

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def create_archive(self, name: str, framework: str, project: str) -> Path:
        path = self.artifacts / name
        with zipfile.ZipFile(path, "w") as archive:
            archive.writestr(
                f"{name.removesuffix('.zip')}/manifest.json",
                json.dumps({"framework": framework, "project_path": project}),
            )
        return path

    @mock.patch.object(MODULE, "validate_zip")
    def test_accepts_at_least_one_archive_per_example(self, validate_zip: mock.Mock) -> None:
        self.create_archive("idf-v5.zip", "esp-idf", "examples/esp-idf/01_idf")
        self.create_archive("idf-v6.zip", "esp-idf", "examples/esp-idf/01_idf")
        self.create_archive("arduino.zip", "arduino", "examples/arduino/01_arduino")

        self.assertEqual(MODULE.validate_release(self.repo, self.artifacts), (3, 2))
        self.assertEqual(validate_zip.call_count, 3)

    @mock.patch.object(MODULE, "validate_zip")
    def test_rejects_a_missing_example(self, validate_zip: mock.Mock) -> None:
        self.create_archive("idf.zip", "esp-idf", "examples/esp-idf/01_idf")

        with self.assertRaisesRegex(ValueError, "examples/arduino/01_arduino"):
            MODULE.validate_release(self.repo, self.artifacts)


if __name__ == "__main__":
    unittest.main()
