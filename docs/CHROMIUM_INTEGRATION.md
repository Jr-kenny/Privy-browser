# Chromium Integration Map

This document maps Privy Browser features to Chromium-owned layers so implementation work stays narrow and upgradeable.

## Baseline

Privy Browser follows a pinned Chromium release instead of tracking `main` during normal feature work. The current pin lives in `config/chromium.version`.

Chromium's desktop browser UI already lives under `chrome/browser/ui/` and `chrome/browser/ui/views/`; Privy should modify that real UI rather than recreate tabs/omnibox/navigation in a separate frontend.

## Privy-owned component

Repository path:

```text
components/privy_privacy/
```

Bootstrap destination:

```text
//components/privy_privacy/
```

This component contains provider-neutral privacy concepts and must stay as low in the dependency graph as practical.

It should not depend on `//chrome/browser`.

## Profile-scoped service

Planned Chromium path:

```text
//chrome/browser/privy/privacy/
    privy_privacy_service.{h,cc}
    privy_privacy_service_factory.{h,cc}
```

Use Chromium's `ProfileKeyedServiceFactory` pattern so normal, guest, and off-the-record behavior is explicit.

Do not redirect incognito to the regular profile by accident. The initial design should create an isolated ephemeral service for OTR/private profiles or deliberately disable capabilities that require durable state.

Responsibilities:

- own `PrivacyPolicyEngine`;
- resolve registered private-compute providers;
- maintain local privacy activity;
- read/write Privy profile prefs;
- expose browser-facing methods for integration surfaces.

## Browser UI

### Privacy Center

Planned path:

```text
//chrome/browser/ui/webui/privy_privacy/
```

Expose a browser-owned WebUI such as:

```text
chrome://privacy
```

The page should show:

- recent blocked/sanitized/private-compute actions;
- per-site decisions;
- extension access/egress events;
- stored private-state categories;
- global privacy controls;
- compute provider health/status without wallet jargon.

WebUI is appropriate because Chromium uses it for privileged browser pages such as settings/history/downloads. Treat WebUI bindings as privileged code; do not load remote scripts into the page.

### Toolbar indicator

After the service and activity model work, add a lightweight toolbar/omnibox-adjacent affordance that summarizes the current site's privacy activity and opens a native bubble or Privacy Center route.

Do not redesign the entire Chromium toolbar for the first vertical slice.

## Request interception

### First vertical slice

Do not begin by patching the lowest-level network service globally.

Start with a narrow browser/profile-owned interception path for a Privy-aware test capability. The goal is to prove:

```text
request -> service -> policy -> private compute -> minimal result
```

before attempting broad automatic tracker replacement.

### Network layer later

Investigate Chromium request throttles/interceptors and content settings before modifying `//services/network` internals. The lower the patch surface, the easier Chromium upgrades remain.

Network-service changes are justified only for behavior that cannot be correctly enforced at a higher browser-owned layer.

## Cookies/storage

Reuse Chromium storage and content-setting infrastructure.

Do not create a second cookie implementation.

Privy additions should focus on policy and observability:

- cross-site persistence decisions;
- partitioning/isolation configuration;
- lifetime reduction;
- privacy activity;
- optional Privy-aware local state that replaces disclosure-heavy patterns.

## Extensions

Chromium's extension system remains enabled.

Privy should add policy without breaking extension compatibility:

1. observe sensitive extension accesses;
2. provide per-site/runtime controls;
3. classify sensitive extension network egress;
4. log meaningful access locally;
5. later expose approved private-compute capabilities to extensions.

Do not modify an extension's content silently in ways that create unpredictable behavior. Enforcement should occur at stable permission/request boundaries.

## Site-facing private compute API

Do this only after the browser service/provider path works.

The API must expose **named capabilities**, not arbitrary execution.

Conceptual IDL:

```text
navigator.privateCompute.request({ capability, publicInputs })
```

The renderer sends only site-controlled public request data. Browser-private inputs are selected in the browser process based on capability and origin.

A new Blink API implies:

- WebIDL definition;
- Blink implementation;
- Mojo interface;
- browser-side binder/authorization;
- feature flag/RuntimeEnabledFeature;
- WPT/browser tests;
- origin/permission policy decision.

Do not add the Blink surface until the internal path is testable without it.

## Aleo execution host

For v1, use a browser-packaged internal execution environment that can load the Aleo JS/WASM SDK and run work in a dedicated worker.

Rules:

- no arbitrary remote scripts;
- not navigable/usable as a normal website capability;
- browser validates typed request before execution;
- bounded concurrency and timeout;
- no network submission unless explicitly requested by a network-backed capability.

## Build integration patches

Privy-owned source files should be copied into the Chromium checkout by `tools/bootstrap.py`. Small patches then register them with Chromium-owned build/factory/UI lists.

Expected patch groups:

```text
patches/
├── branding/
├── core/
├── ui/
├── compute/
├── extensions/
└── privacy/
```

Keep each patch small enough that a failed Chromium rebase points to one understandable feature.

## Branding

Branding is separate from privacy functionality.

The browser must be visibly Privy Browser, but do not mix broad resource renaming with privacy patches. Keep product name/icons/resources in their own patch group so functional changes remain reviewable.

## Build order

1. bootstrap Chromium;
2. copy `components/privy_privacy` overlay;
3. build/run its unit tests;
4. add `PrivyPrivacyService` and factory;
5. add privacy activity storage/model;
6. add `chrome://privacy`;
7. add internal Aleo compute provider;
8. connect first capability end-to-end;
9. add site-facing bridge;
10. expand into extensions/cookies/network enforcement.
