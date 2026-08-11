#!/usr/bin/env python3
"""Prepare a pinned Chromium checkout for Privy Browser development.

This script intentionally uses Chromium's supported depot_tools workflow instead
of cloning the GitHub mirror. It does not modify the Privy repository itself.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = REPO_ROOT / "config" / "chromium.version"


def run(args: list[str], cwd: Path | None = None) -> None:
    printable = " ".join(args)
    print(f"+ {printable}")
    subprocess.run(args, cwd=cwd, check=True)


def require_tool(name: str) -> None:
    if shutil.which(name) is None:
        raise SystemExit(
            f"Required tool '{name}' was not found in PATH. "
            "Install Chromium depot_tools and place it in PATH first."
        )


def read_version() -> str:
    version = VERSION_FILE.read_text(encoding="utf-8").strip()
    if not version:
        raise SystemExit(f"{VERSION_FILE} is empty")
    return version


def assert_clean_checkout(src: Path) -> None:
    status = subprocess.run(
        ["git", "status", "--porcelain"],
        cwd=src,
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()
    if status:
        raise SystemExit(
            "Chromium checkout contains local changes. Refusing to change the "
            "upstream revision. Commit/stash them or use a clean workspace."
        )


def sync_overlay(src: Path) -> None:
    """Copy Privy-owned Chromium source overlays into the checkout.

    Only explicitly mapped directories are copied. We never mirror the whole
    repository into Chromium.
    """
    mappings = [
        (REPO_ROOT / "components" / "privy_privacy", src / "components" / "privy_privacy"),
    ]

    for source, destination in mappings:
        if not source.exists():
            continue
        print(f"+ overlay {source.relative_to(REPO_ROOT)} -> {destination}")
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copytree(source, destination, dirs_exist_ok=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--workspace",
        type=Path,
        default=Path.home() / "privy-chromium",
        help="directory that will contain Chromium's src checkout",
    )
    parser.add_argument(
        "--skip-sync",
        action="store_true",
        help="do not run gclient sync after selecting the pinned revision",
    )
    parser.add_argument(
        "--skip-patches",
        action="store_true",
        help="copy Privy source overlays but do not apply the patch series",
    )
    args = parser.parse_args()

    for tool in ("git", "fetch", "gclient"):
        require_tool(tool)

    version = read_version()
    workspace = args.workspace.expanduser().resolve()
    src = workspace / "src"
    workspace.mkdir(parents=True, exist_ok=True)

    if not (src / ".git").exists():
        if any(workspace.iterdir()):
            raise SystemExit(
                f"{workspace} is not empty and does not contain a Chromium src checkout."
            )
        run(["fetch", "--nohooks", "chromium"], cwd=workspace)
    else:
        assert_clean_checkout(src)

    # Stable Chrome/Chromium releases are tagged with their dotted version.
    run(["git", "fetch", "--tags", "origin"], cwd=src)
    run(["git", "checkout", "--detach", version], cwd=src)

    if not args.skip_sync:
        run(["gclient", "sync", "--with_branch_heads", "--with_tags"], cwd=workspace)

    sync_overlay(src)

    if not args.skip_patches:
        run(
            [
                sys.executable,
                str(REPO_ROOT / "tools" / "apply_patches.py"),
                "--chromium-src",
                str(src),
            ],
            cwd=REPO_ROOT,
        )

    print()
    print(f"Privy Chromium workspace prepared at: {src}")
    print(f"Pinned Chromium version: {version}")
    print("Next: configure GN args, build chrome, then launch the Privy binary.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
