#!/usr/bin/env python3
"""Generate a practical local Chromium build directory for Privy development."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import subprocess


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--chromium-src", type=Path, required=True)
    parser.add_argument("--out", default="out/Privy")
    parser.add_argument(
        "--debug",
        action="store_true",
        help="generate a debug build (release-like build is the default)",
    )
    args = parser.parse_args()

    if shutil.which("gn") is None:
        raise SystemExit("gn was not found in PATH; ensure depot_tools is configured")

    src = args.chromium_src.expanduser().resolve()
    if not (src / ".git").exists():
        raise SystemExit(f"Not a Chromium checkout: {src}")

    gn_args = [
        f"is_debug={'true' if args.debug else 'false'}",
        "is_component_build=true",
        "symbol_level=1",
        "blink_symbol_level=0",
        "v8_symbol_level=0",
    ]

    command = ["gn", "gen", args.out, "--args=" + " ".join(gn_args)]
    print("+ " + " ".join(command))
    subprocess.run(command, cwd=src, check=True)

    print(f"Generated {src / args.out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
