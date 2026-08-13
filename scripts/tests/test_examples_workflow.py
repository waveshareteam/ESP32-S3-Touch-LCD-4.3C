from __future__ import annotations

import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
WORKFLOW = REPO_ROOT / ".github/workflows/examples.yml"


class ExamplesWorkflowTests(unittest.TestCase):
    def test_container_git_command_trusts_the_workspace(self) -> None:
        workflow = WORKFLOW.read_text(encoding="utf-8")

        self.assertIn(
            'git -c safe.directory="$GITHUB_WORKSPACE" show -s --format=%ct "$SOURCE_SHA"',
            workflow,
        )

    def test_release_runs_after_matrix_failures_and_validates_coverage(self) -> None:
        workflow = WORKFLOW.read_text(encoding="utf-8")

        self.assertIn("if: always() && startsWith(github.ref, 'refs/tags/v')", workflow)
        self.assertIn("python3 releases/validate_release_artifacts.py release-artifacts", workflow)

        release_section = workflow.split("  release:\n", 1)[1]
        self.assertLess(
            release_section.index("actions/checkout@v4"),
            release_section.index("releases/validate_release_artifacts.py"),
        )


if __name__ == "__main__":
    unittest.main()
