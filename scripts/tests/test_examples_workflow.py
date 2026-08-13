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


if __name__ == "__main__":
    unittest.main()
