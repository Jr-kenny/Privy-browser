# Privy Browser

**A Chromium browser that replaces unnecessary data disclosure with private computation.**

Privy Browser is not a wallet wrapped in a browser and it is not a browser-shaped web app. It is a real Chromium-derived browser whose privacy model lives inside the browser itself.

The core idea is simple:

> The web often needs an answer about your data. It does not automatically need the data itself.

Instead of letting websites, analytics scripts, advertisers, and extensions collect raw behavioral data by default, Privy Browser introduces a local privacy runtime that can block, sanitize, isolate, or privately compute the requested result. Aleo is the first zero-knowledge execution engine behind that runtime.

## Product rules

1. **No mandatory backend.** Normal browsing and privacy enforcement run on the machine running the browser.
2. **Chromium is the browser foundation.** We keep Chromium's real tab strip, omnibox, renderer, extension platform, network stack, DevTools, downloads, profiles, and desktop UI.
3. **Privacy is behavioral, not identity-first.** Identity proofs may exist later, but the first-class problem is reducing surveillance of browsing behavior.
4. **Private computation beats raw disclosure.** When a site only needs a result, Privy should prefer computing that result locally over releasing the underlying data.
5. **Aleo stays behind an adapter.** Chromium code depends on a private-compute interface, not directly on Aleo SDK internals.
6. **Native and server-hosted modes use the same browser.** Server mode is simply Privy Browser running on a Linux machine and streamed to the user; it is not a separate demo implementation.
7. **No silent phone-home.** Privy Browser must remain useful when every Privy-operated service is unavailable.

## Initial privacy surfaces

- private web telemetry
- private conversion attribution
- private ad-frequency decisions
- cookie and storage mediation
- tracker and cross-site state control
- extension permission and data-egress controls
- fingerprinting reduction
- private personalization/recommendation decisions

## Repository model

This repository intentionally does **not** vendor the entire Chromium source tree. It owns Privy's code, patches, programs, documentation, and build tooling. A bootstrap step checks out a pinned upstream Chromium release and applies the Privy patch stack.

```text
Privy-browser/
├── AGENTS.md
├── config/
│   └── chromium.version
├── components/
│   └── privy_privacy/
├── aleo/
│   └── programs/
├── patches/
│   └── series
├── server/
├── tools/
│   ├── bootstrap.py
│   ├── apply_patches.py
│   ├── configure_build.py
│   └── build.py
└── docs/
    ├── ARCHITECTURE.md
    ├── PRIVACY_MODEL.md
    ├── THREAT_MODEL.md
    ├── ALEO_RUNTIME.md
    ├── CHROMIUM_INTEGRATION.md
    ├── REFERENCE_RESEARCH.md
    ├── CODEX_HANDOFF.md
    └── SERVER_MODE.md
```

## Upstream baseline

The initial baseline is Chromium **151.0.7922.108**. The exact Chromium release tag is pinned in `config/chromium.version`; upgrades are expected and the patch stack is deliberately kept separate to make rebasing routine.

## Bootstrap

Chromium uses `depot_tools`. With `fetch`, `gclient`, `gn`, and `autoninja` available in `PATH`:

```bash
python3 tools/bootstrap.py --workspace ~/privy-chromium
python3 tools/configure_build.py --chromium-src ~/privy-chromium/src
python3 tools/build.py --chromium-src ~/privy-chromium/src
```

The first build target is the Privy core/unit-test target. Add `--browser` to the final command only after that target is green.

## Current implementation

Already in the repository:

- reproducible pinned Chromium checkout and patch tooling;
- provider-neutral privacy request/decision types;
- deterministic, independently testable privacy policy engine;
- fail-closed handling for behavioral data when private compute is unavailable;
- typed `PrivateComputeProvider` boundary;
- deterministic private-compute provider for architecture/integration testing;
- unit tests for policy and compute behavior;
- architecture, privacy, threat-model, Chromium integration, Aleo runtime, server-mode, reference, and Codex handoff documents.

Next browser-level milestone:

- `PrivyPrivacyService` as a profile-keyed Chromium service;
- local privacy activity model;
- `chrome://privacy`;
- first real browser-owned `frequency_cap` request path;
- Aleo provider behind the existing private-compute interface.

See `AGENTS.md` for implementation constraints and `docs/CODEX_HANDOFF.md` for the ordered continuation plan.
