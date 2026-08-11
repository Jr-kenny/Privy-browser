#!/usr/bin/env python3
"""Prepare a pinned Chromium checkout for Privy Browser development.

This script intentionally uses Chromium's supported depot_tools workflow instead
of cloning the GitHub mirror. It does not modify the Privy repository itself.
"""

from __future__ import annotations

import argparse
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


def output(args: list[str], cwd: Path) -> str:
    return subprocess.run(
        args,
        cwd=cwd,
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()


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


def is_dirty(src: Path) -> bool:
    return bool(output(["git", "status", "--porcelain"], cwd=src))


def sync_overlays(src: Path) -> None:
    """Copy Privy-owned source overlays into the Chromium checkout."""
    mappings = [
        (
            REPO_ROOT / "components" / "privy_privacy",
            src / "components" / "privy_privacy",
        ),
        # Files in chromium_overlays/ are laid out relative to Chromium //.
        (REPO_ROOT / "chromium_overlays", src),
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

    fresh_checkout = not (src / ".git").exists()
    if fresh_checkout:
        if any(workspace.iterdir()):
            raise SystemExit(
                f"{workspace} is not empty and does not contain a Chromium src checkout."
            )
        run(["fetch", "--nohooks", "chromium"], cwd=workspace)

    # Resolve the requested tag before deciding whether an existing dirty tree
    # is safe to reuse. A dirty tree is expected after Privy patches/overlays.
    run(["git", "fetch", "--tags", "origin"], cwd=src)
    desired_commit = output(["git", "rev-parse", f"{version}^{{commit}}"], cwd=src)
    current_commit = output(["git", "rev-parse", "HEAD"], cwd=src)
    dirty = is_dirty(src)

    revision_changed = current_commit != desired_commit
    if revision_changed and dirty:
        raise SystemExit(
            "The pinned Chromium revision changed, but the existing checkout "
            "contains local/Privy changes. Rebase or reset that workspace "
            "explicitly before switching Chromium revisions."
        )

    if revision_changed:
        run(["git", "checkout", "--detach", version], cwd=src)
        dirty = False

    # `gclient sync` is needed for a fresh/revision-changed checkout. On an
    # already-prepared dirty checkout, rerunning it can fight the Privy patch
    # stack, so a normal bootstrap refresh leaves dependencies untouched.
    should_sync = not args.skip_sync and (fresh_checkout or revision_changed or not dirty)
    if should_sync:
        run(
            ["gclient", "sync", "--with_branch_heads", "--with_tags"],
            cwd=workspace,
        )
    elif not args.skip_sync and dirty:
        print("= existing Privy-patched checkout detected; skipping gclient sync")

    sync_overlays(src)

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
    print("Next: configure GN args, build Privy tests, then build the browser.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
