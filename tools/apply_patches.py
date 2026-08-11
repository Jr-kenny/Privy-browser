#!/usr/bin/env python3
"""Apply the ordered Privy Chromium patch series."""

from __future__ import annotations

import argparse
from pathlib import Path
import subprocess

REPO_ROOT = Path(__file__).resolve().parents[1]
SERIES = REPO_ROOT / "patches" / "series"


def check(args: list[str], cwd: Path) -> bool:
    return subprocess.run(
        args,
        cwd=cwd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0


def run(args: list[str], cwd: Path) -> None:
    print("+ " + " ".join(args))
    subprocess.run(args, cwd=cwd, check=True)


def patch_paths() -> list[Path]:
    if not SERIES.exists():
        return []

    result: list[Path] = []
    for raw in SERIES.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        path = (REPO_ROOT / "patches" / line).resolve()
        if not path.is_file():
            raise SystemExit(f"Patch listed in series does not exist: {line}")
        result.append(path)
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--chromium-src", type=Path, required=True)
    args = parser.parse_args()

    src = args.chromium_src.expanduser().resolve()
    if not (src / ".git").exists():
        raise SystemExit(f"Not a Chromium git checkout: {src}")

    patches = patch_paths()
    if not patches:
        print("No Privy Chromium patches are listed yet.")
        return 0

    for patch in patches:
        rel = patch.relative_to(REPO_ROOT / "patches")
        if check(["git", "apply", "--reverse", "--check", str(patch)], src):
            print(f"= already applied: {rel}")
            continue
        if not check(["git", "apply", "--check", str(patch)], src):
            raise SystemExit(
                f"Patch does not apply cleanly: {rel}\n"
                "Stop and rebase the patch against the pinned Chromium version."
            )
        run(["git", "apply", str(patch)], src)
        print(f"✓ applied: {rel}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
