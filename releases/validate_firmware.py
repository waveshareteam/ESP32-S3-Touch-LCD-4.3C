#!/usr/bin/env python3
"""Validate packaged firmware archives without extracting them."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timezone
from pathlib import Path, PurePosixPath
from typing import Any, Callable


REQUIRED_FILES = {"README.md", "SHA256SUMS", "flash.sh", "flash.bat", "flash_args.txt", "manifest.json"}


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def safe_member(name: str) -> PurePosixPath:
    if "\\" in name:
        raise ValueError(f"archive member uses a backslash: {name}")
    path = PurePosixPath(name)
    if path.is_absolute() or any(part in ("", ".", "..") for part in path.parts):
        raise ValueError(f"unsafe archive member: {name}")
    return path


def parse_offset(value: str | int) -> int:
    return value if isinstance(value, int) else int(value, 0)


def validate_timestamp(value: Any) -> None:
    if not isinstance(value, str) or not value.endswith("Z"):
        raise ValueError("timestamp_utc must be an ISO 8601 UTC timestamp ending in Z")
    try:
        parsed = datetime.fromisoformat(value[:-1] + "+00:00")
    except ValueError as exc:
        raise ValueError("timestamp_utc is not a valid ISO 8601 timestamp") from exc
    if parsed.tzinfo is None or parsed.utcoffset() != timezone.utc.utcoffset(parsed):
        raise ValueError("timestamp_utc must use UTC")


def validate_manifest(
    manifest: dict[str, Any], read_file: Callable[[str], bytes], available: set[str]
) -> None:
    if manifest.get("schema_version") != 1:
        raise ValueError("unsupported or missing manifest schema_version")
    for field in (
        "name",
        "framework",
        "framework_version",
        "target",
        "project_path",
        "git_sha",
        "timestamp_utc",
        "baud",
        "files",
        "flash_command",
        "combined_bin",
        "segments",
    ):
        if field not in manifest:
            raise ValueError(f"manifest is missing {field!r}")
    if manifest["framework"] not in ("esp-idf", "arduino"):
        raise ValueError(f"unsupported source-build framework: {manifest['framework']}")
    if manifest["target"] != "esp32s3":
        raise ValueError(f"unexpected firmware target: {manifest['target']}")
    project_path = str(manifest["project_path"])
    safe_member(project_path)
    git_sha = str(manifest["git_sha"])
    if git_sha and not re.fullmatch(r"[0-9a-f]{7,64}", git_sha):
        raise ValueError("git_sha must be empty or a hexadecimal commit identifier")
    validate_timestamp(manifest["timestamp_utc"])
    if not str(manifest["baud"]).isdigit() or int(manifest["baud"]) <= 0:
        raise ValueError("baud must be a positive integer")
    if not isinstance(manifest["flash_command"], str) or "<PORT>" not in manifest["flash_command"]:
        raise ValueError("flash_command must contain the <PORT> placeholder")
    runtime = manifest.get("runtime_resources", {})
    if runtime.get("sdcard_included") is not False:
        raise ValueError("manifest must explicitly state that SD-card resources are excluded")

    records = manifest["files"]
    segments = manifest["segments"]
    if not isinstance(records, list) or not isinstance(segments, list):
        raise ValueError("files and segments must be lists")
    if not records:
        raise ValueError("manifest has no firmware records")
    by_path: dict[str, dict[str, Any]] = {}
    for record in records:
        for field in ("offset", "file", "size", "sha256"):
            if field not in record:
                raise ValueError(f"firmware record is missing {field!r}")
        relative = str(record["file"])
        safe_member(relative)
        if parse_offset(record["offset"]) < 0:
            raise ValueError(f"negative firmware offset for {relative}")
        if relative not in available:
            raise ValueError(f"manifest references a missing file: {relative}")
        data = read_file(relative)
        if len(data) != int(record["size"]):
            raise ValueError(f"size mismatch for {relative}")
        if sha256_bytes(data) != str(record["sha256"]).lower():
            raise ValueError(f"SHA-256 mismatch for {relative}")
        if relative in by_path:
            raise ValueError(f"duplicate firmware record for {relative}")
        by_path[relative] = record

    combined = str(manifest["combined_bin"])
    if combined not in by_path:
        raise ValueError("combined_bin is not listed in manifest files")
    if parse_offset(by_path[combined]["offset"]) != 0:
        raise ValueError("combined firmware must be flashed at offset 0x0")
    combined_data = read_file(combined)
    if not combined_data or combined_data[0] != 0xE9:
        raise ValueError("combined firmware does not begin with an Espressif image header")

    segment_paths: set[str] = set()
    position = 0
    for record in sorted(segments, key=lambda item: parse_offset(item["offset"])):
        for field in ("offset", "file", "size", "sha256"):
            if field not in record:
                raise ValueError(f"firmware segment is missing {field!r}")
        relative = str(record["file"])
        if relative in segment_paths:
            raise ValueError(f"duplicate firmware segment for {relative}")
        segment_paths.add(relative)
        file_record = by_path.get(relative)
        if file_record is None:
            raise ValueError(f"segment is not listed in files: {relative}")
        for field in ("offset", "size", "sha256"):
            if str(file_record[field]).lower() != str(record[field]).lower():
                raise ValueError(f"segment metadata differs from files for {relative}")
        offset = parse_offset(record["offset"])
        if offset < position:
            raise ValueError(f"overlapping firmware segment: {record['file']}")
        position = offset + int(record["size"])
    if not segments:
        raise ValueError("manifest has no source firmware segments")
    if segment_paths != set(by_path) - {combined}:
        raise ValueError("files must contain exactly the combined image and source segments")
    if len(combined_data) != position:
        raise ValueError(
            f"combined firmware length {len(combined_data)} does not match segment layout {position}"
        )

    binary_members = {name for name in available if name.lower().endswith(".bin")}
    if binary_members != set(by_path):
        untracked = sorted(binary_members - set(by_path))
        missing = sorted(set(by_path) - binary_members)
        raise ValueError(f"binary manifest mismatch; untracked={untracked}, non-binary={missing}")

    checksum_lines = read_file("SHA256SUMS").decode("utf-8").splitlines()
    checksums: dict[str, str] = {}
    for line in checksum_lines:
        if not line:
            continue
        try:
            digest, relative = line.split("  ", 1)
        except ValueError as exc:
            raise ValueError("invalid SHA256SUMS line") from exc
        checksums[relative] = digest.lower()
    expected_checksums = {path: str(record["sha256"]).lower() for path, record in by_path.items()}
    if checksums != expected_checksums:
        raise ValueError("SHA256SUMS does not match the manifest")


def validate_zip(path: Path) -> None:
    if not path.is_file():
        raise FileNotFoundError(path)
    with zipfile.ZipFile(path) as archive:
        infos = archive.infolist()
        names = [info.filename for info in infos if not info.is_dir()]
        if len(names) != len(set(names)):
            raise ValueError("archive contains duplicate member names")
        safe_paths = [safe_member(name) for name in names]
        roots = {member.parts[0] for member in safe_paths}
        if len(roots) != 1:
            raise ValueError("archive must contain exactly one top-level package directory")
        root = next(iter(roots))
        relative_names = {
            PurePosixPath(*member.parts[1:]).as_posix() for member in safe_paths if len(member.parts) > 1
        }
        missing = REQUIRED_FILES - relative_names
        if missing:
            raise ValueError("archive is missing required files: " + ", ".join(sorted(missing)))
        for relative in relative_names:
            parts = [part.lower() for part in PurePosixPath(relative).parts]
            if "sdcard" in parts:
                raise ValueError(f"archive contains an SD-card runtime resource: {relative}")

        def read_file(relative: str) -> bytes:
            return archive.read(f"{root}/{relative}")

        manifest = json.loads(read_file("manifest.json").decode("utf-8"))
        if manifest.get("name") != root:
            raise ValueError("manifest name does not match the ZIP top-level directory")
        validate_manifest(manifest, read_file, relative_names)


def validate_directory(path: Path) -> None:
    if not path.is_dir():
        raise FileNotFoundError(path)
    available = {
        item.relative_to(path).as_posix() for item in path.rglob("*") if item.is_file()
    }
    missing = REQUIRED_FILES - available
    if missing:
        raise ValueError("package is missing required files: " + ", ".join(sorted(missing)))
    if any("sdcard" in [part.lower() for part in PurePosixPath(name).parts] for name in available):
        raise ValueError("package contains SD-card runtime resources")

    def read_file(relative: str) -> bytes:
        return (path / relative).read_bytes()

    manifest = json.loads(read_file("manifest.json").decode("utf-8"))
    if manifest.get("name") != path.name:
        raise ValueError("manifest name does not match the package directory")
    validate_manifest(manifest, read_file, available)


def validate_path(path: Path) -> None:
    if path.suffix.lower() == ".zip":
        validate_zip(path)
    else:
        validate_directory(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("packages", nargs="+", help="Firmware ZIP archives or unpacked package directories.")
    args = parser.parse_args()
    try:
        for value in args.packages:
            path = Path(value)
            validate_path(path)
            digest = sha256_bytes(path.read_bytes()) if path.is_file() else "directory"
            print(f"valid: {path.as_posix()} ({digest})")
    except (OSError, ValueError, KeyError, json.JSONDecodeError, zipfile.BadZipFile) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
