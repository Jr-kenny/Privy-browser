# Codex Handoff

Read `AGENTS.md`, `docs/ARCHITECTURE.md`, `docs/PRIVACY_MODEL.md`, and `docs/CHROMIUM_INTEGRATION.md` before editing Chromium integration code.

Do not replace the architecture with an Electron app, React browser mock, extension-only implementation, wallet dashboard, or mandatory backend.

## Mission

Produce the first genuine vertical slice of Privy Browser:

```text
real Chromium browser
    ↓
profile-scoped Privy privacy service
    ↓
real browser-owned privacy request
    ↓
policy chooses compute_privately
    ↓
local private compute provider
    ↓
minimal result
    ↓
local privacy activity visible in browser UI
```

The provider-neutral policy/compute core and deterministic provider already exist. Aleo should replace the deterministic provider behind the same interface after the Chromium path is proven.

## Already implemented — validate, do not reinvent

- pinned Chromium bootstrap/patch tooling;
- `PrivacyRequest` / `PrivacyDecision` model;
- deterministic `PrivacyPolicyEngine`;
- fail-closed behavioral-data rule;
- typed `PrivateComputeProvider` interface;
- `DeterministicPrivateComputeProvider` for all four initial capabilities;
- directly runnable `privy_privacy_unittests` target;
- architecture/privacy/threat/Aleo/server documents.

## Workstream 0 — validate scaffold

1. Run:
   ```bash
   python3 -m py_compile tools/bootstrap.py tools/apply_patches.py tools/configure_build.py tools/build.py
   ```
2. Bootstrap the pinned Chromium workspace.
3. Generate `out/Privy`.
4. Build and run `//components/privy_privacy:privy_privacy_unittests`.
5. Fix compile/style failures before expanding the browser integration.
6. Build Chromium `chrome` at the pinned revision before adding large patches.

Preferred commands:

```bash
python3 tools/bootstrap.py --workspace ~/privy-chromium
python3 tools/configure_build.py --chromium-src ~/privy-chromium/src
python3 tools/build.py --chromium-src ~/privy-chromium/src
python3 tools/build.py --chromium-src ~/privy-chromium/src --browser
```

Do not weaken fail-closed behavior to make tests pass.

## Workstream 1 — profile service

Create Chromium integration under:

```text
//chrome/browser/privy/privacy/
  privy_privacy_service.h
  privy_privacy_service.cc
  privy_privacy_service_factory.h
  privy_privacy_service_factory.cc
  BUILD.gn
```

Requirements:

- `PrivyPrivacyService` derives from `KeyedService`;
- factory uses `ProfileKeyedServiceFactory`;
- regular and OTR/incognito semantics are explicit;
- OTR must not reuse durable behavioral state from the regular profile;
- service owns `PrivacyPolicyEngine`;
- provider is injected/replaceable for tests;
- use `DeterministicPrivateComputeProvider` first;
- no Aleo SDK dependency in the service.

Tests:

- regular profile service creation;
- OTR behavior;
- provider unavailable -> behavioral request blocked;
- provider available -> `kComputePrivately` dispatches provider;
- requester receives only the typed minimal result.

## Workstream 2 — privacy activity model

Add a bounded, profile-local activity store with:

- timestamp;
- requester origin/type;
- privacy surface/capability;
- decision;
- type/count of private input consumed, never raw values;
- amount/type of information disclosed;
- success/failure reason.

Requirements:

- no raw browsing history in activity events;
- bounded memory/disk behavior;
- OTR activity ephemeral;
- observer interface for UI.

## Workstream 3 — Privacy Center

Create a functional internal Chromium WebUI:

```text
chrome://privacy
```

Show:

- protection status;
- compute-provider status;
- recent activity;
- blocked / sanitized / privately-computed counts;
- origin/request type;
- empty state.

Do not build a crypto dashboard. No wallet balances/addresses.

## Workstream 4 — first browser-owned request path

Create one controlled browser-native entry path for `frequency_cap`.

Do **not** begin with arbitrary website JavaScript.

Sequence:

1. browser-internal test/WebUI action invokes capability;
2. service classifies the request;
3. policy selects `kComputePrivately`;
4. deterministic provider executes;
5. requester receives only boolean result;
6. activity event appears in `chrome://privacy`;
7. browser integration test covers the whole path.

This is the first vertical slice.

## Workstream 5 — Aleo provider

Read `docs/ALEO_RUNTIME.md` and `docs/REFERENCE_RESEARCH.md` first.

Before adding the Provable JS SDK as a distribution dependency, resolve its GPL-3.0 compatibility with Privy's intended distribution. Do not hide or bypass that decision.

If using the web SDK, target:

- browser-packaged internal compute context;
- dedicated worker;
- Aleo WASM loaded locally;
- no arbitrary remote JavaScript;
- typed capability -> fixed Aleo program mapping;
- timeout/cancellation;
- bounded concurrent jobs;
- no automatic network transaction;
- no visible wallet requirement for local compute.

First Aleo-backed capability: `frequency_cap`.

Acceptance:

- private impression count never returned to requester;
- result is boolean/minimal approved output;
- execution happens on the machine running Privy;
- no Privy backend;
- no Aleo network transaction required for local mode;
- provider failure remains fail-closed;
- swapping deterministic provider for Aleo does not change browser callers.

## Workstream 6 — restricted site-facing API

Only after the internal path is green.

Design a narrow named-capability API. Do not expose arbitrary Aleo programs or allow page code to select browser-private inputs.

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
- per-origin rate limiting;
- fixed capability allowlist;
- no stable account identifier returned;
- malformed/compromised renderer request tests.

## Workstream 7 — extension privacy

After the first Aleo vertical slice works:

1. identify stable Chromium extension access/egress hooks;
2. build local access ledger;
3. add per-site runtime control;
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

Then prototype one Privy-aware state pattern that removes the need for a tracking identifier.

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
- broad tracker replacement before one private-compute flow works;
- hackathon-only mocked analytics screens.
