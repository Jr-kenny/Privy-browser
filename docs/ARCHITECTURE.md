# Architecture

## 1. System shape

Privy Browser is a Chromium derivative. Chromium remains responsible for normal browser behavior; Privy adds a profile-scoped privacy control plane and an isolated private-compute plane.

```text
┌──────────────────────────────────────────────────────────────┐
│                         Privy Browser                        │
├──────────────────────────────────────────────────────────────┤
│ Chromium UI                                                  │
│ tabs · omnibox · settings · downloads · extensions · DevTools│
├──────────────────────────────────────────────────────────────┤
│ Privy UI                                                     │
│ privacy activity · site decisions · extension access         │
├──────────────────────────────────────────────────────────────┤
│ PrivyPrivacyService (profile scoped)                         │
│                                                              │
│  classify -> policy -> allow/block/sanitize/prompt/private    │
│                              compute                          │
├──────────────────────────────────────────────────────────────┤
│ Privacy interception surfaces                               │
│ navigation · network · cookies/storage · extensions · Blink  │
├──────────────────────────────────────────────────────────────┤
│ PrivateComputeProvider                                      │
│                 │                                            │
│                 └── AleoPrivateComputeProvider               │
│                     local JS/WASM execution                   │
├──────────────────────────────────────────────────────────────┤
│ Chromium                                                     │
│ Browser process · renderers · network service · storage       │
└──────────────────────────────────────────────────────────────┘
```

There is no required Privy server in this diagram.

## 2. Trust boundaries

### Browser process

Trusted coordinator. Owns policy, user decisions, profile configuration, privacy activity, and routing into private compute.

It must not run expensive proving synchronously on the UI thread.

### Renderer processes

Untrusted with respect to browser-private state. A page can request an operation but cannot directly read the data used to satisfy it.

A site-facing API, if exposed, must cross a typed Mojo boundary and be authorized by the browser process.

### Extension processes/workers

Extensions are not automatically trusted because they are installed. Their declared permissions remain relevant, but Privy can add narrower runtime policy and observable egress controls.

### Private compute host

Privileged enough to receive the minimum private inputs selected by `PrivyPrivacyService`, but isolated from arbitrary web content. It executes the configured provider and returns a typed result/proof package.

### Privy-operated infrastructure

Outside the core trust model. The normal browser must remain functional without it.

## 3. Core browser service

`PrivyPrivacyService` should be implemented as a profile-keyed service.

Responsibilities:

- receive privacy-sensitive requests from integration surfaces;
- normalize them into `PrivacyRequest`;
- evaluate `PrivacyPolicyEngine`;
- dispatch approved private computations;
- maintain profile-local privacy activity;
- expose observable state to browser UI;
- store policy/preferences through Chromium preference infrastructure;
- ensure failure is privacy-safe.

It should not:

- own Aleo-specific SDK objects;
- directly render UI;
- accept arbitrary script from websites;
- upload activity to a Privy server.

## 4. Request/decision model

```text
PrivacyRequest
├── requester origin
├── requester type: site | extension | browser
├── surface
├── operation
├── first/cross-site context
├── persistence requirement
├── sensitivity flags
├── private-compute capability
└── purpose/capability identifier

PrivacyDecision
├── allow
├── block
├── sanitize
├── prompt
└── compute_privately
```

The decision engine must be deterministic and independently unit-testable.

## 5. Private computation

Core code depends on a provider-neutral interface:

```text
PrivateComputeProvider
    Execute(request, callback)
    Supports(capability)
```

The first provider is Aleo.

This design deliberately allows later providers or a native implementation without rewriting Chromium integration surfaces.

### v1 Aleo execution host

The shortest maintainable path for the current Aleo JavaScript/WASM SDK is a Privy-owned internal Chromium execution context that sites cannot navigate to or script against directly. It loads only browser-packaged resources and communicates with `PrivyPrivacyService` through a narrow typed interface.

Candidate implementation:

- internal `chrome://privy-compute` or restricted equivalent;
- dedicated worker for Aleo WASM;
- no normal web origin privileges;
- no arbitrary remote script loading;
- execution off the UI thread;
- explicit resource limits/timeouts;
- browser process validates every request and result.

Longer term, this provider may move to a dedicated utility service/native Rust boundary if that improves isolation and performance. The rest of the browser must not care.

## 6. Browser integration surfaces

### Network and telemetry

Start above the lowest-level network implementation wherever possible. A request throttle/interceptor owned by the browser profile is preferable to invasive changes across Chromium's network service.

Goals:

- identify known telemetry/tracking requests;
- detect Privy-aware requests;
- block/sanitize requests according to policy;
- replace supported telemetry flows with private-compute outputs.

### Cookies and storage

Integrate with Chromium content/cookie settings rather than maintaining an independent cookie jar.

Goals:

- preserve necessary first-party sessions;
- partition/isolate cross-site state;
- minimize persistence where possible;
- surface storage behavior to users;
- provide private-state alternatives for supported Privy-aware sites.

### Extensions

Do not break Chromium's extension platform. Add Privy policy around sensitive access and egress.

Goals:

- clear runtime access ledger;
- per-site grants;
- temporary grants;
- sensitive-data egress decisions;
- private-compute capability where raw browser data is not required.

### Blink/site API

A browser-native API is useful only after the browser side exists. Proposed shape:

```js
const result = await navigator.privateCompute.request({
  capability: 'frequency-cap',
  publicInputs: { campaign: 'summer' }
});
```

The page does not provide or receive the private browser state. It requests a named capability. The browser chooses private inputs, policy, provider, and disclosure.

Do not expose a generic "run arbitrary Aleo program against browser data" API.

## 7. Privacy activity model

Every meaningful privacy decision should emit a local activity event:

```text
PrivacyActivity
├── timestamp
├── origin
├── requester type
├── capability/surface
├── decision
├── disclosed fields/count
├── privately consumed fields/count
└── optional proof/result metadata
```

This powers the Privacy Center and debugging. Raw private inputs are never stored in the activity log merely for display.

## 8. Native and server-hosted distribution

### Native

```text
macOS / Windows / Linux
        ↓
Privy Browser binary
        ↓
local profile + privacy runtime
```

### Server-hosted

```text
client browser
    ↓ WebRTC/stream transport
user-controlled Linux server
    ↓
Privy Browser binary
    ↓
remote profile + privacy runtime
```

The server-hosted browser is the same product binary. Streaming is distribution/access infrastructure, not the privacy engine.

## 9. Update strategy

Privy should follow a pinned Chromium release and carry a narrow patch series.

Rules:

- keep Privy-owned new files/components separate whenever possible;
- keep patches small and topic-specific;
- never allow a privacy feature to justify staying on an old Chromium security baseline;
- rebase frequently;
- document every patch's purpose and upstream touch points.

## 10. First vertical slice

The first complete path should be:

```text
test site requests frequency decision
        ↓
site-facing Privy capability
        ↓
PrivyPrivacyService
        ↓
PrivacyPolicyEngine => compute privately
        ↓
AleoPrivateComputeProvider
        ↓
local Aleo execution
        ↓
minimal boolean/result returned
        ↓
PrivacyActivity recorded
        ↓
Privacy Center shows what happened
```

This proves the architecture without making identity or wallet connectivity the product.
