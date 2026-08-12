# Codex Checkpoint

Captured on 2026-08-12 after pausing the first full Chromium build to preserve
Mac battery.

## Repository state

- Chromium target: `151.0.7922.108`
- Chromium checkout used locally: `/Users/user/privy-chromium/src`
- Chromium checkout is detached at the pinned revision.
- The Privy repository contains the current source overlays and patch series.
- The local `out/Privy` directory is generated build output and is not part of
  this repository. A new machine should regenerate it.

## Completed evidence

- Python syntax checks passed for `tools/bootstrap.py`,
  `tools/apply_patches.py`, `tools/configure_build.py`, and `tools/build.py`.
- JavaScript syntax check passed for the Privacy Center `app.js`.
- The original Privy core tests passed: 11/11.
- The original Privy service tests passed: 4/4.
- The core and service tests covered fail-closed provider failure, typed
  inputs, cross-site policy, first-party cookie behavior, extension egress,
  fingerprint sanitization, and off-the-record behavior.
- `git diff --check` and patch applicability checks passed before this
  checkpoint.

## Current implementation in this checkpoint

- M151 compatibility includes the required callback headers and the split
  service/factory GN targets.
- The browser profile lifecycle registers
  `PrivyPrivacyServiceFactory` through a small patch in `patches/core/`.
- Regular and off-the-record profiles receive distinct service instances.
- `PrivyPrivacyService` owns a bounded profile-local activity store with an
  observer interface and explicit clearing.
- Activity entries retain requester metadata, decision metadata, typed input
  counts, disclosure type/count, and failure reason. They never retain raw
  private-compute inputs or browsing history.
- `chrome://privacy` is a real browser WebUI with browser-packaged local
  resources, provider status, activity counts, recent activity, clear
  activity, and a browser-owned frequency-cap action.
- A focused browser test covers the frequency-cap action, activity event, and
  clear operation. It still needs to compile and run against the synced M151
  checkout.
- The frequency-cap browser request is classified as `compute_privately` and
  uses the existing deterministic provider. Provider failure remains blocked.

## Paused build

The first full command was:

```bash
env PATH="/Users/user/depot_tools:$PATH" python3 tools/build.py \
  --chromium-src /Users/user/privy-chromium/src --browser
```

It was intentionally interrupted for battery preservation after reaching
`19287/60544` actions at about 1 hour 48 minutes. It was still compiling
Blink, V8 bindings, and Chromium libraries and had not reached linking or
browser launch. This build started before the newest activity and WebUI
overlays were copied into the Chromium checkout, so its result must not be
treated as validation of those newer files.

## Resume commands

From a machine with the repository and a Chromium workspace:

```bash
python3 -m py_compile tools/bootstrap.py tools/apply_patches.py \
  tools/configure_build.py tools/build.py
env PATH="/path/to/depot_tools:$PATH" python3 tools/bootstrap.py \
  --workspace /path/to/privy-chromium --skip-sync --no-history
env PATH="/path/to/depot_tools:$PATH" python3 tools/configure_build.py \
  --chromium-src /path/to/privy-chromium/src
env PATH="/path/to/depot_tools:$PATH" python3 tools/build.py \
  --chromium-src /path/to/privy-chromium/src
```

After the Privy targets pass, build the browser with `--browser`, launch the
normal binary, then build and run the focused `browser_tests` target for
`PrivyPrivacyUIBrowserTest.FrequencyCapResultAppearsInPrivacyActivity`.

The next implementation boundary is Workstream 5, the Aleo provider. It has
not started. The deterministic provider remains the local provider for the
first browser vertical slice until the Chromium integration tests are green.
