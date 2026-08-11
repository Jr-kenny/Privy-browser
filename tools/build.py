#!/usr/bin/env python3
"""Build Privy core/service targets, run tests, optionally build Chromium."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess
import sys


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
    parser.add_argument(
        "--skip-tests",
        action="store_true",
        help="build the Privy unit test binary but do not execute it",
    )
    args = parser.parse_args()

    if shutil.which("autoninja") is None:
        raise SystemExit(
            "autoninja was not found in PATH; ensure depot_tools is configured"
        )

    src = args.chromium_src.expanduser().resolve()
    if not (src / ".git").exists():
        raise SystemExit(f"Not a Chromium checkout: {src}")

    test_target = "components/privy_privacy:privy_privacy_unittests"
    service_target = "chrome/browser/privy/privacy:privacy"
    targets = [test_target, service_target]
    if args.browser:
        targets.append("chrome")

    run(["autoninja", "-C", args.out, *targets], cwd=src)

    if not args.skip_tests:
        binary_name = (
            "privy_privacy_unittests.exe"
            if sys.platform == "win32"
            else "privy_privacy_unittests"
        )
        test_binary = src / args.out / binary_name
        if not test_binary.exists():
            raise SystemExit(
                f"Expected test binary was not produced: {test_binary}"
            )
        run([str(test_binary)], cwd=src)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
