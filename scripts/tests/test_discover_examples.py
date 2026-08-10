from __future__ import annotations

import argparse
import json
import sys
import tempfile
import unittest
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import discover_examples  # noqa: E402


class DiscoverExamplesTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.repo = Path(self.temporary.name)
        (self.repo / "config").mkdir()
        (self.repo / "config/ci.json").write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "board": {"target": "esp32s3"},
                    "esp_idf": {"versions": ["v5.5.5", "v6.0.2"]},
                    "arduino": {
                        "cli": "1.5.1",
                        "core": "3.3.11",
                        "fqbn": "esp32:esp32:esp32s3:PSRAM=opi",
                    },
                }
            ),
            encoding="utf-8",
        )

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def args(self, surface: str, selector: str = "all") -> argparse.Namespace:
        return argparse.Namespace(
            repo=str(self.repo),
            config="config/ci.json",
            surface=surface,
            selector=selector,
            idf_versions=None,
            arduino_core=None,
            fqbn=None,
            target=None,
            expect_examples=None,
            allow_empty_selector=False,
        )

    def test_idf_discovery_only_accepts_direct_projects(self) -> None:
        (self.repo / "examples/esp-idf/01_first").mkdir(parents=True)
        (self.repo / "examples/esp-idf/01_first/CMakeLists.txt").touch()
        (self.repo / "examples/esp-idf/01_first/components/vendor/example").mkdir(parents=True)
        (self.repo / "examples/esp-idf/01_first/components/vendor/example/CMakeLists.txt").touch()
        (self.repo / "examples/esp-idf/not_a_project").mkdir()

        matrix, count = discover_examples.build_matrix(self.args("esp-idf"))

        self.assertEqual(count, 1)
        self.assertEqual(len(matrix["include"]), 2)
        self.assertEqual({item["path"] for item in matrix["include"]}, {"examples/esp-idf/01_first"})
        self.assertEqual({item["idf"] for item in matrix["include"]}, {"v5.5.5", "v6.0.2"})

    def test_arduino_discovery_excludes_libraries_and_nested_ino_files(self) -> None:
        sketch = self.repo / "examples/arduino/01_first"
        sketch.mkdir(parents=True)
        (sketch / "01_first.ino").touch()
        nested = self.repo / "examples/arduino/02_nested/src"
        nested.mkdir(parents=True)
        (nested / "not_a_top_level_sketch.ino").touch()
        vendor = self.repo / "examples/arduino/libraries/vendor/examples/demo"
        vendor.mkdir(parents=True)
        (vendor / "demo.ino").touch()

        matrix, count = discover_examples.build_matrix(self.args("arduino"))

        self.assertEqual(count, 1)
        self.assertEqual([item["name"] for item in matrix["include"]], ["01_first"])
        self.assertEqual(matrix["include"][0]["cli"], "1.5.1")
        self.assertIn("PSRAM=opi", matrix["include"][0]["fqbn"])

    def test_selector_matches_name_or_complete_path(self) -> None:
        for name in ("01_first", "02_second"):
            path = self.repo / "examples/esp-idf" / name
            path.mkdir(parents=True)
            (path / "CMakeLists.txt").touch()

        by_name, _ = discover_examples.build_matrix(self.args("esp-idf", "02_second"))
        by_path, _ = discover_examples.build_matrix(
            self.args("esp-idf", "examples/esp-idf/01_first")
        )

        self.assertEqual({item["name"] for item in by_name["include"]}, {"02_second"})
        self.assertEqual({item["name"] for item in by_path["include"]}, {"01_first"})

    def test_unknown_selector_is_rejected(self) -> None:
        path = self.repo / "examples/esp-idf/01_first"
        path.mkdir(parents=True)
        (path / "CMakeLists.txt").touch()

        with self.assertRaisesRegex(ValueError, "matched no esp-idf examples"):
            discover_examples.build_matrix(self.args("esp-idf", "missing"))

    def test_arduino_main_file_must_match_directory_case(self) -> None:
        sketch = self.repo / "examples/arduino/01_first"
        sketch.mkdir(parents=True)
        (sketch / "01_FIRST.ino").touch()

        with self.assertRaisesRegex(ValueError, "case-exact"):
            discover_examples.build_matrix(self.args("arduino"))

    def test_empty_surface_can_be_allowed_after_cross_surface_validation(self) -> None:
        sketch = self.repo / "examples/arduino/01_first"
        sketch.mkdir(parents=True)
        (sketch / "01_first.ino").touch()
        args = self.args("esp-idf", "01_first")
        args.allow_empty_selector = True

        matrix, count = discover_examples.build_matrix(args)

        self.assertEqual(count, 0)
        self.assertEqual(matrix, {"include": []})


if __name__ == "__main__":
    unittest.main()
