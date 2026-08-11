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

Aleo should then replace the deterministic local provider behind the same interface.

## Workstream 0 — validate scaffold

1. Run `python3 -m py_compile tools/bootstrap.py tools/apply_patches.py`.
2. Review `components/privy_privacy` against Chromium C++ style/build rules.
3. Fix any compile/style issue before expanding it.
4. Do not weaken fail-closed behavior.

Acceptance:

- scripts parse;
- component GN target is internally consistent;
- policy unit tests are valid Chromium gtests.

## Workstream 1 — Chromium workspace

1. Install/use `depot_tools`.
2. Run `tools/bootstrap.py` in a clean workspace.
3. Generate an `out/Privy` GN directory.
4. Build `//components/privy_privacy:unit_tests` first.
5. Run tests and fix all failures.
6. Build Chromium `chrome` at the pinned revision before adding large patches.

Do not move to product integration until upstream + Privy core can build.

## Workstream 2 — profile service

Create under the Privy repo as a Chromium overlay/patch-backed integration:

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
- no Aleo SDK dependency here.

Tests:

- regular profile service creation;
- OTR behavior;
- provider unavailable -> behavioral request blocked;
- provider available -> `kComputePrivately` dispatches provider.

## Workstream 3 — privacy activity model

Add a bounded, profile-local activity store.

Initial event fields:

- timestamp;
- requester origin;
- requester type;
- privacy surface/capability;
- decision;
- count/type of private input consumed (not raw values);
- amount/type of data disclosed;
- success/failure reason.

Requirements:

- no raw browsing history in activity events;
- bounded memory/disk behavior;
- OTR activity is ephemeral;
- observer interface for UI.

## Workstream 4 — Privacy Center

Create an internal Chromium WebUI:

```text
chrome://privacy
```

First version should be functional, not decorative.

Show:

- protection status;
- current compute provider status;
- recent activity;
- counts for blocked / sanitized / privately computed requests;
- origin/request type;
- empty state.

Do not build a crypto dashboard. Do not show wallet balances/addresses.

## Workstream 5 — deterministic private compute provider

Before Aleo, implement a tiny in-process/test provider behind `PrivateComputeProvider` for the four typed capabilities.

Purpose: prove browser integration independently of Aleo SDK/toolchain complexity.

This provider is not the final feature, but it is genuine architecture testing rather than a fake UI demo.

Acceptance:

- `FrequencyCapInput{prior=2,max=3}` -> `true`;
- `prior=3,max=3` -> `false`;
- service logs only result metadata;
- disabling provider causes fail-closed block.

## Workstream 6 — first browser request surface

Create one controlled browser-native entry path for `frequency_cap`.

Do **not** start with arbitrary website JS access.

Recommended sequence:

1. browser-internal test page / WebUI action invokes capability;
2. browser integration test proves service/provider/UI event flow;
3. only then add a restricted site-facing API.

This keeps security boundaries testable while the core is young.

## Workstream 7 — Aleo provider

Read `docs/ALEO_RUNTIME.md` and `docs/REFERENCE_RESEARCH.md` first.

Before adding the Provable JS SDK as a distribution dependency, resolve the GPL-3.0 compatibility decision. Do not hide the issue.

Implementation target if using web SDK:

- browser-packaged internal compute context;
- dedicated worker;
- Aleo WASM loaded locally;
- no arbitrary remote JS;
- typed capability -> fixed Aleo program mapping;
- timeout/cancellation;
- bounded concurrent jobs;
- no automatic network transaction;
- no visible wallet requirement for local compute.

First Aleo-backed capability: `frequency_cap`.

Acceptance:

- private impression count is not returned to requester;
- result is boolean/minimal approved output;
- execution happens on machine running Privy;
- no Privy backend;
- no Aleo network transaction required for local mode;
- provider failure remains fail-closed.

## Workstream 8 — site-facing API

Only after the previous path is green.

Design a narrow named-capability API. Do not expose arbitrary programs or browser-private input selection.

Conceptual shape:

```js
await navigator.privateCompute.request({
  capability: "frequency-cap",
  publicInputs: { campaign: "..." }
});
```

Security requirements:

- feature flag while experimental;
- secure context requirement;
- origin captured by browser, never trusted from JS payload;
- browser selects private inputs;
- rate limiting;
- capability allowlist;
- no stable account identifier returned;
- tests for compromised/malformed renderer requests.

## Workstream 9 — extension privacy

After first Aleo vertical slice works:

1. identify stable Chromium extension access/egress hooks;
2. build local access ledger;
3. add per-site runtime control;
4. prototype behavioral-data egress classification;
5. expose private-compute API to extensions only after site/browser contract is stable.

Do not break standard Chrome extension installation/usage as a shortcut.

## Workstream 10 — cookies/storage

Do not replace Chromium's cookie jar.

Start with:

- third-party/cross-site persistence policy;
- privacy activity visibility;
- session-vs-persistent distinction;
- state partitioning behavior;
- tests proving ordinary first-party login/session cookies still work.

Then prototype one Privy-aware state pattern that removes the need for a tracking identifier.

## Workstream 11 — server mode

Only after a Linux Privy Browser binary launches correctly.

Use `docs/SERVER_MODE.md`.

Deliver:

- a reproducible single-user VPS setup;
- authenticated streaming endpoint;
- persistent profile volume;
- no public remote debugging port;
- Chromium sandbox retained;
- clear documentation that the VPS is the machine executing the browser/private compute.

## What not to spend time on yet

- user accounts;
- cloud sync;
- a Prime/Privy API backend;
- wallet UI;
- token balances;
- mobile builds;
- custom browser engine;
- redesigning every Chromium screen;
- Tor/VPN claims;
- broad tracker replacement before one private-compute flow works;
- hackathon-only mocked analytics screens.

## Immediate next command

On the development machine, after depot_tools is available:

```bash
python3 tools/bootstrap.py --workspace ~/privy-chromium
```

Then inspect/build the copied component before writing the first Chromium integration patch.
