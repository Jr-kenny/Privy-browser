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
│   └── apply_patches.py
└── docs/
    ├── ARCHITECTURE.md
    ├── PRIVACY_MODEL.md
    ├── THREAT_MODEL.md
    ├── ALEO_RUNTIME.md
    ├── CHROMIUM_INTEGRATION.md
    └── SERVER_MODE.md
```

## Upstream baseline

The initial baseline is Chromium **150.0.7871.186**, the latest broadly released Linux stable build verified when this repository was initialized. Upgrades are expected; the patch stack is deliberately kept separate to make rebasing routine.

## Current state

The repository is in architecture/bootstrap stage. The first implementation milestone is a locally buildable Chromium derivative with:

- Privy branding;
- a profile-scoped `PrivyPrivacyService`;
- a browser-native privacy decision engine;
- an internal private-compute bridge;
- one end-to-end Aleo-backed private telemetry primitive;
- a visible privacy activity surface in the browser UI.

See `AGENTS.md` for implementation constraints and `docs/ARCHITECTURE.md` for the system design.
