# Codex Handoff

Read `AGENTS.md`, `docs/ARCHITECTURE.md`, `docs/PRIVACY_MODEL.md`, and `docs/CHROMIUM_INTEGRATION.md` before editing Chromium integration code.

Do not replace the architecture with an Electron app, React browser mock, extension-only implementation, wallet dashboard, or mandatory backend.

## Mission

Finish the first genuine vertical slice of Privy Browser:

```text
real Chromium browser
    ↓
profile-scoped Privy privacy service       [implemented]
    ↓
real browser-owned privacy request
    ↓
policy chooses compute_privately           [implemented]
    ↓
local private compute provider             [deterministic provider implemented]
    ↓
minimal result                             [implemented at service boundary]
    ↓
local privacy activity visible in browser UI
```

The next task is not to redesign the architecture. It is to validate the existing overlays against Chromium M151, then connect the browser-visible activity/request path.

## Already implemented — validate, do not reinvent

- pinned Chromium **151.0.7922.108** bootstrap/overlay/patch tooling;
- `PrivacyRequest` / `PrivacyDecision` model;
- deterministic `PrivacyPolicyEngine`;
- fail-closed behavioral-data rule;
- typed `PrivateComputeProvider` interface;
- `DeterministicPrivateComputeProvider` for all four initial capabilities;
- directly runnable `privy_privacy_unittests` target;
- Chromium overlay at `//chrome/browser/privy/privacy/`;
- `PrivyPrivacyService : KeyedService`;
- `PrivyPrivacyServiceFactory : ProfileKeyedServiceFactory`;
- separate off-the-record service instances instead of redirecting to regular profile state;
- service-derived compute availability (callers do not get to claim a provider exists);
- provider failure -> block, never raw behavioral-data fallback;
- directly runnable `privy_privacy_service_unittests` target;
- architecture/privacy/threat/Aleo/server/reference documents.

## Workstream 0 — compile and validate the existing foundation

Run:

```bash
python3 -m py_compile tools/bootstrap.py tools/apply_patches.py tools/configure_build.py tools/build.py
python3 tools/bootstrap.py --workspace ~/privy-chromium
python3 tools/configure_build.py --chromium-src ~/privy-chromium/src
python3 tools/build.py --chromium-src ~/privy-chromium/src
```

`tools/build.py` should compile and run both:

```text
//components/privy_privacy:privy_privacy_unittests
//chrome/browser/privy/privacy:privy_privacy_service_unittests
```

Fix all M151 compile/GN/style/API mismatches in the existing code before adding features. If a current Chromium API differs from the scaffold, adapt Privy to Chromium; do not downgrade Chromium and do not remove security behavior merely to make it compile.

After both Privy test targets are green:

```bash
python3 tools/build.py --chromium-src ~/privy-chromium/src --browser
```

Confirm a clean Chromium browser binary builds at the pinned revision.

## Workstream 1 — wire the service into normal browser lifecycle

The service/factory source already exists under `chromium_overlays/chrome/browser/privy/privacy/`.

Do the minimum Chromium-owned integration required so the factory is constructed/available through the normal profile lifecycle. Keep this as a small explicit patch in `patches/core/` and add it to `patches/series`.

Requirements:

- no global singleton privacy state;
- regular and OTR profiles resolve distinct service instances;
- OTR does not reuse durable behavioral state from the regular profile;
- browser shutdown does not leave callbacks/use-after-free paths;
- no Aleo dependency enters profile/service code.

Add integration/factory tests for regular vs OTR service resolution.

## Workstream 2 — privacy activity model

Implement a bounded profile-local activity store owned by the browser service or a closely related profile-scoped object.

Initial event fields:

- timestamp;
- requester origin/type;
- privacy surface/capability;
- decision;
- type/count of private input consumed, never raw values;
- amount/type of information disclosed;
- success/failure reason.

Requirements:

- no raw browsing history in activity events;
- no raw private-compute input values;
- bounded memory/disk behavior;
- OTR activity is ephemeral;
- observer interface for browser UI;
- clearing browser/site data has explicit semantics for Privy-owned state.

Tests must cover event creation for allow/block/private-compute/provider-failure and OTR isolation.

## Workstream 3 — Privacy Center

Create a functional internal Chromium WebUI:

```text
chrome://privacy
```

First version should show real state from `PrivyPrivacyService`, not mocked cards.

Show:

- protection status;
- current private-compute provider status;
- recent privacy activity;
- counts for blocked / sanitized / privately-computed requests;
- requester origin/type;
- empty state;
- clear activity control.

Use Chromium WebUI patterns and browser-packaged resources. Do not load remote JavaScript. Do not build a crypto dashboard; no wallet balances/addresses.

## Workstream 4 — first browser-owned frequency-cap path

Create one controlled browser-native path that invokes the existing `kFrequencyCap` capability.

Do **not** begin with arbitrary website JavaScript.

Required end-to-end sequence:

1. browser-internal test/WebUI action constructs the request;
2. `PrivyPrivacyService` receives it;
3. policy selects `kComputePrivately`;
4. deterministic provider executes using local private inputs;
5. caller receives only the boolean result;
6. privacy activity records what happened without raw private inputs;
7. `chrome://privacy` displays the event;
8. browser/integration test covers the entire path.

This is the first complete browser vertical slice.

## Workstream 5 — Aleo provider

Read `docs/ALEO_RUNTIME.md` and `docs/REFERENCE_RESEARCH.md` first.

Before adding the Provable JS SDK as a distributed dependency, resolve its GPL-3.0 compatibility with Privy's intended licensing/distribution. Do not hide or bypass that decision.

If using the browser SDK path, target:

- browser-packaged internal compute context;
- dedicated worker;
- Aleo WASM loaded locally;
- no arbitrary remote JavaScript;
- typed capability -> fixed Aleo program mapping;
- timeout/cancellation;
- bounded concurrent jobs;
- no automatic Aleo network transaction;
- no visible wallet requirement for local compute.

First Aleo-backed capability: `frequency_cap`.

Acceptance:

- private impression count never returned to requester;
- result remains boolean/minimal approved output;
- execution happens on the machine running Privy;
- no Privy backend;
- no Aleo network transaction required for local mode;
- provider failure remains fail-closed;
- browser callers do not change when deterministic provider is replaced by Aleo.

## Workstream 6 — restricted site-facing API

Only after the internal path and Aleo provider are green.

Design a narrow named-capability API. Do not expose arbitrary Aleo programs or allow page code to select/read browser-private inputs.

Conceptual shape:

```js
await navigator.privateCompute.request({
  capability: "frequency-cap",
  publicInputs: { campaign: "..." }
});
```

Requirements:

- experimental feature flag;
- secure context;
- origin captured by browser, never trusted from JS payload;
- browser selects private inputs;
- per-origin request/entropy budget;
- fixed capability allowlist;
- no stable account identifier returned;
- malformed/compromised renderer request tests;
- no generic "run this program over my browser data" surface.

## Workstream 7 — extension privacy

After the first Aleo vertical slice works:

1. identify stable Chromium extension permission/access/egress hooks;
2. build local access ledger;
3. add per-site/runtime controls;
4. prototype behavioral-data egress classification;
5. expose private-compute capabilities only after the browser/site contract is stable.

Do not break normal Chrome extension compatibility as a shortcut.

## Workstream 8 — cookies/storage

Do not replace Chromium's cookie jar.

Start with:

- third-party/cross-site persistence policy;
- privacy activity visibility;
- session-vs-persistent distinction;
- state partitioning behavior;
- tests proving ordinary first-party login/session cookies still work.

Then prototype one Privy-aware state pattern that removes the need for a stable tracking identifier.

## Workstream 9 — server mode

Only after a Linux Privy Browser binary launches correctly.

Use `docs/SERVER_MODE.md`.

Deliver:

- reproducible single-user VPS setup;
- authenticated streaming endpoint;
- persistent profile volume;
- no public remote-debugging port;
- Chromium sandbox retained;
- documentation that the VPS is the machine executing the browser/private compute.

## Do not spend time on yet

- user accounts;
- cloud sync;
- a Privy API backend;
- wallet UI;
- token balances;
- mobile builds;
- custom browser engine;
- redesigning every Chromium screen;
- Tor/VPN claims;
- broad automatic tracker replacement before one private-compute flow works;
- hackathon-only mocked analytics screens.
