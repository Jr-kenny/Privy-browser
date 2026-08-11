# AGENTS.md

This repository is for a real Chromium-derived browser. Treat every change as product code unless a file is explicitly marked experimental.

## Product thesis

Privy Browser reduces how much behavioral data the web receives by replacing unnecessary disclosure with browser-local policy and private computation.

The primary privacy problem is **what the user does**, not proving who the user is.

Good first-class use cases:

- private telemetry;
- private attribution;
- private frequency capping;
- private personalization;
- cookie/storage minimization;
- extension data-egress control;
- anti-fingerprinting;
- privacy-preserving anti-abuse.

Identity, age, KYC, balances, wallet connection, and credential proofs are secondary. Do not turn the product into a wallet or identity browser.

## Non-negotiable architecture constraints

1. **Do not build a fake browser frontend.** Use Chromium's real desktop UI and browser primitives.
2. **Do not introduce a mandatory Privy backend.** The browser must work locally without a Privy account, Privy API, hosted database, or telemetry service.
3. **Do not send browsing history, cookies, extension data, private-compute inputs, or behavioral state to Privy-operated infrastructure.**
4. **Keep Aleo behind an adapter boundary.** Core browser policy code must not depend directly on Aleo SDK types.
5. **Prefer profile-scoped browser services over global singletons.** Privacy decisions and state belong to a Chromium profile unless there is a strong reason otherwise.
6. **Keep site code outside the trust boundary.** A webpage must never gain direct access to private browser state merely because it can call a Privy API.
7. **Keep expensive or third-party cryptography away from the main browser UI thread.**
8. **Do not weaken Chromium sandboxing, Site Isolation, Safe Browsing/security boundaries, certificate validation, or renderer isolation to make integration easier.**
9. **Do not blindly copy patches from reference browsers.** Study their architecture, then implement Privy's behavior independently unless a copied component is deliberately accepted with its license.
10. **No security/privacy claim without a corresponding implementation or documented limitation.**

## Repository strategy

Do not vendor Chromium into this repository. The repository owns a patch stack and Privy-specific source overlays.

Expected flow:

```text
pinned Chromium
    + Privy source overlays
    + Privy patch series
    + Privy Aleo programs/runtime assets
    = Privy Browser
```

Keep upstream-touching changes narrow. Prefer adding isolated directories/components and a small number of integration patches over editing broad Chromium surfaces everywhere.

## Target Chromium integration points

These are starting points, not permission to hack them indiscriminately:

- `chrome/browser/ui/` and `chrome/browser/ui/views/` for browser UI;
- `chrome/browser/ui/webui/` for `chrome://` privacy surfaces;
- profile keyed services for `PrivyPrivacyService`;
- content/browser navigation and request throttles for policy interception;
- content settings / cookie settings for cookie and storage policy;
- extensions permission/network surfaces for extension privacy;
- Blink + Mojo only when exposing a browser-native private-compute capability to sites;
- a Privy-owned isolated compute host for Aleo JS/WASM execution.

## Privacy request model

Every privacy-sensitive request should conceptually pass through:

```text
requester -> classify -> policy -> decision
                              |
                              +-> allow
                              +-> block
                              +-> sanitize
                              +-> prompt
                              +-> compute privately
```

The policy engine should remain deterministic and testable without Aleo.

A private-compute request should contain only the minimum typed inputs required for the computation. Raw browsing history should not be serialized into generic JSON and handed to arbitrary site or extension code.

## Aleo integration

Aleo is the first implementation of `PrivateComputeProvider`.

The browser should ask for operations such as:

- `frequency_cap`;
- `conversion_attribution`;
- `telemetry_bucket`;
- `personalization_match`.

The provider handles program loading, execution, proof material, and optional network submission.

For local-only operations, do not create an onchain transaction merely to say that Aleo was used. Onchain state is only justified when the product semantics require shared/verifiable state, record consumption, issuance, revocation, payment, or another network-consensus property.

## UI rules

The browser should feel like a polished Chromium browser, not a crypto dashboard.

Avoid wallet jargon in normal browsing UI. Users should see concepts such as:

- blocked;
- isolated;
- computed privately;
- disclosed;
- site request;
- extension access;
- privacy activity.

The Privacy Center should explain what happened, not expose cryptographic internals by default.

## Server-hosted mode

Server mode is the same Linux browser binary running on a remote machine, streamed to a browser client. It must not become a separate web implementation of Privy Browser.

The server is inside the user's trust boundary. Do not claim that server-hosted mode hides page contents from the server that runs Chromium.

## Testing requirements

For every privacy rule, add tests that cover at least:

- expected allow path;
- expected block/private-compute path;
- cross-site behavior;
- private/incognito profile behavior where relevant;
- extension vs website request origin where relevant;
- failure behavior when private compute is unavailable.

A private-compute failure must fail safely. Do not silently fall back to releasing raw sensitive input.

## Work priority

1. reproducible Chromium checkout/build;
2. Privy core component and profile service;
3. privacy activity model and UI surface;
4. one real request interception path;
5. Aleo local compute provider;
6. end-to-end private telemetry/frequency-cap flow;
7. extension privacy controls;
8. server-hosted packaging;
9. additional privacy primitives.

## Definition of done for the first vertical slice

A fresh machine can follow repository instructions to build and launch Privy Browser. A test page triggers a browser-owned privacy request. Privy Browser classifies it, keeps the sensitive input inside the browser, executes the configured Aleo-backed private computation locally, returns only the permitted output, records the event in the Privacy Center, and does not require a Privy backend.
