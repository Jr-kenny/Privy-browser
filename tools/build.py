#!/usr/bin/env python3
"""Build Privy core tests and optionally the Chromium browser."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess


def run(args: list[str], cwd: Path) -> None:
    print("+ " + " ".join(args))
    subprocess.run(args, cwd=cwd, check=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--chromium-src", type=Path, required=True)
    parser.add_argument("--out", default="out/Privy")
    parser.add_argument(
        "--browser",
        action="store_true",
        help="also build the Chromium chrome target",
    )
    args = parser.parse_args()

    if shutil.which("autoninja") is None:
        raise SystemExit(
            "autoninja was not found in PATH; ensure depot_tools is configured"
        )

    src = args.chromium_src.expanduser().resolve()
    if not (src / ".git").exists():
        raise SystemExit(f"Not a Chromium checkout: {src}")

    targets = ["components/privy_privacy:unit_tests"]
    if args.browser:
        targets.append("chrome")

    run(["autoninja", "-C", args.out, *targets], cwd=src)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
