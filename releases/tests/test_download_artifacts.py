from __future__ import annotations

import importlib.util
import unittest
from pathlib import Path
from unittest import mock


SCRIPT = Path(__file__).resolve().parents[1] / "download_artifacts.py"
SPEC = importlib.util.spec_from_file_location("download_artifacts", SCRIPT)
assert SPEC and SPEC.loader
MODULE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MODULE)


class DownloadArtifactsTests(unittest.TestCase):
    @mock.patch.object(MODULE, "run_text", return_value="origin/main")
    def test_default_branch_uses_remote_head(self, run_text: mock.Mock) -> None:
        self.assertEqual(MODULE.default_branch(), "main")
        run_text.assert_called_once()

    @mock.patch.object(MODULE, "run_text", return_value=None)
    def test_default_branch_falls_back_to_main(self, run_text: mock.Mock) -> None:
        self.assertEqual(MODULE.default_branch(), "main")

    @mock.patch.object(MODULE, "read_json")
    def test_latest_run_filters_success_by_branch(self, read_json: mock.Mock) -> None:
        expected = {"id": 123, "html_url": "https://github.com/example/repo/actions/runs/123"}
        read_json.return_value = {"workflow_runs": [expected]}

        self.assertEqual(MODULE.latest_run("example/repo", "examples.yml", "main", None), expected)
        url = read_json.call_args.args[0]
        self.assertIn("status=success", url)
        self.assertIn("branch=main", url)

    @mock.patch.object(MODULE, "read_json")
    def test_latest_run_reports_latest_failure(self, read_json: mock.Mock) -> None:
        run_url = "https://github.com/example/repo/actions/runs/456"
        read_json.side_effect = [
            {"workflow_runs": []},
            {"workflow_runs": [{"id": 456, "conclusion": "failure", "html_url": run_url}]},
        ]

        with self.assertRaisesRegex(
            RuntimeError,
            r"no successful examples\.yml run found for branch 'main'; "
            rf"latest run is failure: {run_url}",
        ):
            MODULE.latest_run("example/repo", "examples.yml", "main", None)

        success_url = read_json.call_args_list[0].args[0]
        latest_url = read_json.call_args_list[1].args[0]
        self.assertIn("status=success", success_url)
        self.assertNotIn("status=success", latest_url)
        self.assertIn("branch=main", latest_url)


if __name__ == "__main__":
    unittest.main()
