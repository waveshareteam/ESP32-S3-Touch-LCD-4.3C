#!/usr/bin/env python3
"""Download firmware archives from a successful GitHub Actions run."""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
import re
import shutil
import stat
import subprocess
import sys
import tempfile
import urllib.error
import urllib.parse
import urllib.request
import zipfile
from pathlib import Path, PurePosixPath


DEFAULT_REPO = "waveshareteam/ESP32-S3-Touch-LCD-4.3C"
DEFAULT_WORKFLOW = "examples.yml"
DEFAULT_BRANCH = "main"
DEFAULT_OUTPUT = Path(__file__).resolve().parent / "downloads"
API_ROOT = "https://api.github.com"
USER_AGENT = "waveshare-esp32-s3-touch-lcd-4.3c-artifacts"


def slugify(value: str) -> str:
    value = re.sub(r"[^A-Za-z0-9._-]+", "-", value.strip())
    value = re.sub(r"-+", "-", value).strip("-")
    return "artifact" if value in ("", ".", "..") else value


def run_text(command: list[str]) -> str | None:
    try:
        result = subprocess.run(command, check=True, capture_output=True, text=True)
    except (OSError, subprocess.CalledProcessError):
        return None
    return result.stdout.strip() or None


def parse_github_repo(value: str) -> str | None:
    value = value.strip().removesuffix(".git").rstrip("/")
    if value.startswith("git@github.com:"):
        candidate = value.removeprefix("git@github.com:")
    elif value.startswith(("https://github.com/", "http://github.com/", "ssh://git@github.com/")):
        candidate = value.split("github.com/", 1)[1]
    elif re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", value):
        candidate = value
    else:
        return None
    parts = candidate.split("/")
    return "/".join(parts[:2]) if len(parts) >= 2 else None


def default_repo() -> str:
    remote = run_text(["git", "config", "--get", "remote.origin.url"])
    parsed = parse_github_repo(remote) if remote else None
    return parsed or DEFAULT_REPO


def default_branch() -> str:
    remote_head = run_text(
        ["git", "symbolic-ref", "--quiet", "--short", "refs/remotes/origin/HEAD"]
    )
    if remote_head and "/" in remote_head:
        branch = remote_head.split("/", 1)[1].strip()
        if branch:
            return branch
    return DEFAULT_BRANCH


def token() -> str | None:
    environment = os.environ.get("GH_TOKEN") or os.environ.get("GITHUB_TOKEN")
    return environment.strip() if environment else run_text(["gh", "auth", "token"])


def request(url: str, auth_token: str | None) -> urllib.request.Request:
    headers = {
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
        "User-Agent": USER_AGENT,
    }
    if auth_token:
        headers["Authorization"] = f"Bearer {auth_token}"
    return urllib.request.Request(url, headers=headers)


def read_json(url: str, auth_token: str | None) -> dict:
    try:
        with urllib.request.urlopen(request(url, auth_token)) as response:
            return json.loads(response.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        if exc.code in (401, 403):
            raise RuntimeError("GitHub authentication failed; run `gh auth login` or set GH_TOKEN.") from exc
        raise


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def download(url: str, destination: Path, auth_token: str | None) -> None:
    destination.parent.mkdir(parents=True, exist_ok=True)
    opener = urllib.request.build_opener(NoRedirect)
    try:
        response = opener.open(request(url, auth_token))
    except urllib.error.HTTPError as exc:
        if exc.code not in (301, 302, 303, 307, 308) or not exc.headers.get("Location"):
            raise
        response = urllib.request.urlopen(
            urllib.request.Request(exc.headers["Location"], headers={"User-Agent": USER_AGENT})
        )
    with response, destination.open("wb") as output:
        shutil.copyfileobj(response, output)


def safe_parts(name: str) -> tuple[str, ...]:
    if "\\" in name:
        raise ValueError(f"unsafe archive path: {name}")
    path = PurePosixPath(name)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise ValueError(f"unsafe archive path: {name}")
    return path.parts


def extract_zip(archive_path: Path, destination: Path, strip_single_root: bool) -> None:
    with zipfile.ZipFile(archive_path) as archive:
        files = [item for item in archive.infolist() if not item.is_dir()]
        roots = {safe_parts(item.filename)[0] for item in files}
        strip_root = next(iter(roots)) if strip_single_root and len(roots) == 1 else None
        destination_root = destination.resolve()
        for item in files:
            parts = safe_parts(item.filename)
            if strip_root and parts[0] == strip_root:
                parts = parts[1:]
            if not parts:
                continue
            target = destination.joinpath(*parts)
            if os.path.commonpath([str(destination_root), str(target.resolve())]) != str(destination_root):
                raise ValueError(f"archive path escapes destination: {item.filename}")
            target.parent.mkdir(parents=True, exist_ok=True)
            with archive.open(item) as source, target.open("wb") as output:
                shutil.copyfileobj(source, output)


def extract_actions_artifact(outer_zip: Path, destination: Path, keep_archives: bool) -> list[str]:
    if destination.exists():
        raise FileExistsError(f"artifact destination already exists: {destination}")
    destination.mkdir(parents=True)
    with zipfile.ZipFile(outer_zip) as outer, tempfile.TemporaryDirectory() as temporary:
        inner = [item for item in outer.infolist() if not item.is_dir() and item.filename.endswith(".zip")]
        if len(inner) == 1:
            inner_path = Path(temporary) / Path(inner[0].filename).name
            with outer.open(inner[0]) as source, inner_path.open("wb") as output:
                shutil.copyfileobj(source, output)
            if keep_archives:
                shutil.copy2(inner_path, destination / inner_path.name)
            extract_zip(inner_path, destination, strip_single_root=True)
            return sorted(
                item.relative_to(destination).as_posix()
                for item in destination.rglob("*")
                if item.is_file()
            )
    extract_zip(outer_zip, destination, strip_single_root=False)
    return sorted(
        item.relative_to(destination).as_posix()
        for item in destination.rglob("*")
        if item.is_file()
    )


def make_shell_scripts_executable(root: Path) -> list[str]:
    if os.name == "nt":
        return []
    changed: list[str] = []
    for script in root.rglob("*.sh"):
        if not script.is_file():
            continue
        script.chmod(script.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
        changed.append(script.relative_to(root).as_posix())
    return changed


def latest_run(repo: str, workflow: str, branch: str | None, auth_token: str | None) -> dict:
    workflow_id = urllib.parse.quote(workflow, safe="")
    parameters = {"status": "success", "per_page": "1"}
    if branch:
        parameters["branch"] = branch
    url = f"{API_ROOT}/repos/{repo}/actions/workflows/{workflow_id}/runs?{urllib.parse.urlencode(parameters)}"
    runs = read_json(url, auth_token).get("workflow_runs", [])
    if runs:
        return runs[0]

    branch_context = f" for branch {branch!r}" if branch else ""
    parameters.pop("status")
    latest_url = (
        f"{API_ROOT}/repos/{repo}/actions/workflows/{workflow_id}/runs?"
        f"{urllib.parse.urlencode(parameters)}"
    )
    latest = read_json(latest_url, auth_token).get("workflow_runs", [])
    if latest:
        run = latest[0]
        state = run.get("conclusion") or run.get("status") or "unknown"
        run_url = run.get("html_url") or (
            f"https://github.com/{repo}/actions/runs/{run.get('id', '')}"
        )
        raise RuntimeError(
            f"no successful {workflow} run found{branch_context}; "
            f"latest run is {state}: {run_url}"
        )
    raise RuntimeError(f"no {workflow} runs found{branch_context}")


def artifacts(repo: str, run_id: int, auth_token: str | None) -> list[dict]:
    result: list[dict] = []
    page = 1
    while True:
        url = f"{API_ROOT}/repos/{repo}/actions/runs/{run_id}/artifacts?per_page=100&page={page}"
        batch = read_json(url, auth_token).get("artifacts", [])
        result.extend(batch)
        if len(batch) < 100:
            return result
        page += 1


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", default=default_repo())
    parser.add_argument("--workflow", default=DEFAULT_WORKFLOW)
    parser.add_argument("--run-id", type=int)
    parser.add_argument(
        "--branch",
        default=default_branch(),
        help="GitHub branch to query (defaults to the repository default branch).",
    )
    parser.add_argument("--pattern", default="firmware-*")
    parser.add_argument("--artifact", action="append", help="Exact artifact name; may be repeated.")
    parser.add_argument("--output-dir", default=DEFAULT_OUTPUT.as_posix())
    parser.add_argument("--keep-archives", action="store_true")
    parser.add_argument("--clean", action="store_true")
    args = parser.parse_args()

    auth_token = token()
    try:
        if args.run_id:
            run_id = args.run_id
            run_url = f"https://github.com/{args.repo}/actions/runs/{run_id}"
        else:
            run = latest_run(args.repo, args.workflow, args.branch, auth_token)
            run_id = int(run["id"])
            run_url = str(run["html_url"])
        output = Path(args.output_dir) / f"run-{run_id}"
        if args.clean and output.exists():
            shutil.rmtree(output)
        output.mkdir(parents=True, exist_ok=True)

        available = [item for item in artifacts(args.repo, run_id, auth_token) if not item.get("expired")]
        if args.artifact:
            requested = set(args.artifact)
            selected = [item for item in available if item.get("name") in requested]
            missing = requested - {item.get("name") for item in selected}
            if missing:
                raise RuntimeError("artifacts not found: " + ", ".join(sorted(missing)))
        else:
            selected = [item for item in available if fnmatch.fnmatch(str(item.get("name", "")), args.pattern)]
        if not selected:
            raise RuntimeError(f"no artifacts matched {args.pattern!r}")

        summary = {"repo": args.repo, "run_id": run_id, "run_url": run_url, "artifacts": []}
        for item in selected:
            name = str(item["name"])
            archive = output / "_archives" / f"{slugify(name)}.zip"
            destination = output / slugify(name)
            print(f"Downloading {name}...")
            download(str(item["archive_download_url"]), archive, auth_token)
            files = extract_actions_artifact(archive, destination, args.keep_archives)
            executable_scripts = make_shell_scripts_executable(destination)
            if not args.keep_archives:
                archive.unlink(missing_ok=True)
            summary["artifacts"].append(
                {
                    "name": name,
                    "path": destination.as_posix(),
                    "files": files,
                    "executable_scripts": executable_scripts,
                }
            )
        archive_dir = output / "_archives"
        if archive_dir.exists() and not any(archive_dir.iterdir()):
            archive_dir.rmdir()
        (output / "artifacts.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
        print(f"Downloaded {len(selected)} artifacts to {output.as_posix()}")
        print(run_url)
    except (OSError, RuntimeError, ValueError, KeyError, json.JSONDecodeError, zipfile.BadZipFile) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
