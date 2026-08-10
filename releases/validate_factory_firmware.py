#!/usr/bin/env python3
"""Validate the checked-in factory firmware against its repository metadata."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path
from typing import Any


DEFAULT_CONFIG = Path("config/factory-firmware.json")
EXPECTED_SCHEMA = 1
EXPECTED_TARGET = "esp32s3"
ESP_IMAGE_HEADER = b"\xe9"
REQUIRED_FIELDS = {
    "schema_version",
    "name",
    "board",
    "hardware_variant",
    "target",
    "image",
    "offset",
    "size",
    "sha256",
    "sdcard_resources_included",
}


def resolve_inside(repo: Path, value: str | Path, description: str) -> Path:
    candidate = Path(value)
    resolved = candidate.resolve() if candidate.is_absolute() else (repo / candidate).resolve()
    try:
        resolved.relative_to(repo)
    except ValueError as exc:
        raise ValueError(f"{description} must be inside the repository") from exc
    return resolved


def parse_offset(value: object) -> int:
    if isinstance(value, bool) or not isinstance(value, (int, str)):
        raise ValueError("factory firmware offset must be an integer or numeric string")
    try:
        return value if isinstance(value, int) else int(value, 0)
    except ValueError as exc:
        raise ValueError("factory firmware offset is not a valid integer") from exc


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_factory_firmware(
    repo_value: str | Path = ".", config_value: str | Path = DEFAULT_CONFIG
) -> dict[str, Any]:
    repo = Path(repo_value).resolve()
    if not repo.is_dir():
        raise FileNotFoundError(f"repository directory not found: {repo}")

    config_path = resolve_inside(repo, config_value, "factory firmware configuration")
    if not config_path.is_file():
        raise FileNotFoundError(f"factory firmware configuration not found: {config_path}")
    config = json.loads(config_path.read_text(encoding="utf-8"))
    if not isinstance(config, dict):
        raise ValueError("factory firmware configuration must be a JSON object")

    missing = REQUIRED_FIELDS - config.keys()
    if missing:
        raise ValueError("factory firmware configuration is missing: " + ", ".join(sorted(missing)))
    if config["schema_version"] != EXPECTED_SCHEMA:
        raise ValueError(f"unsupported factory firmware schema: {config['schema_version']!r}")

    for field in ("name", "board", "hardware_variant", "target", "image"):
        if not isinstance(config[field], str) or not config[field].strip():
            raise ValueError(f"factory firmware field {field!r} must be a non-empty string")
    if config["target"] != EXPECTED_TARGET:
        raise ValueError(f"factory firmware target must be {EXPECTED_TARGET!r}")
    if parse_offset(config["offset"]) != 0:
        raise ValueError("factory firmware must be flashed at offset 0x0")
    if config["sdcard_resources_included"] is not False:
        raise ValueError("factory firmware must explicitly exclude SD-card runtime resources")

    expected_size = config["size"]
    if isinstance(expected_size, bool) or not isinstance(expected_size, int) or expected_size <= 0:
        raise ValueError("factory firmware size must be a positive integer")
    expected_sha = config["sha256"]
    if not isinstance(expected_sha, str) or not re.fullmatch(r"[0-9a-fA-F]{64}", expected_sha):
        raise ValueError("factory firmware sha256 must contain 64 hexadecimal characters")

    configured_image = Path(config["image"])
    image_path = resolve_inside(repo, config["image"], "factory firmware image")
    image_relative = image_path.relative_to(repo)
    if image_relative.parent != Path("firmware") or configured_image.suffix != ".bin":
        raise ValueError("factory firmware image must be a top-level firmware/*.bin file")
    if not image_path.is_file():
        raise FileNotFoundError(f"factory firmware image not found: {image_path}")
    actual_size = image_path.stat().st_size
    if actual_size != expected_size:
        raise ValueError(
            f"factory firmware size mismatch: expected {expected_size}, got {actual_size}"
        )
    actual_sha = sha256_file(image_path)
    if actual_sha.lower() != expected_sha.lower():
        raise ValueError(
            f"factory firmware SHA-256 mismatch: expected {expected_sha.lower()}, got {actual_sha}"
        )
    with image_path.open("rb") as image:
        if image.read(1) != ESP_IMAGE_HEADER:
            raise ValueError("factory firmware does not begin with an Espressif image header")

    return {
        "config": config_path.relative_to(repo).as_posix(),
        "image": image_relative.as_posix(),
        "size": actual_size,
        "sha256": actual_sha,
        "target": config["target"],
        "offset": "0x0",
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", default=".")
    parser.add_argument("--config", default=DEFAULT_CONFIG.as_posix())
    args = parser.parse_args()
    try:
        result = validate_factory_firmware(args.repo, args.config)
        print(
            f"valid: {result['image']} "
            f"({result['size']} bytes, sha256={result['sha256']})"
        )
    except (OSError, ValueError, KeyError, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
