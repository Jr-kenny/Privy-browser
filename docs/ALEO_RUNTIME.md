# Aleo Runtime

## Role

Aleo is Privy Browser's first private-compute provider. It is not the browser identity system, not a mandatory wallet, and not a reason to put ordinary browsing activity onchain.

Privy core code depends on provider-neutral typed capabilities. The Aleo adapter maps those capabilities to Aleo programs and returns only approved outputs/proof material.

## Why local execution

The Aleo SDK supports executing Aleo programs client-side through WebAssembly. That matches Privy's central requirement: sensitive browser inputs should remain on the machine running the browser when network consensus is not required.

Aleo program execution is computationally expensive, so it must run away from Chromium's UI thread. The Aleo SDK's own browser guidance uses Web Workers for this class of work.

## v1 execution shape

```text
PrivyPrivacyService (browser process)
        ↓ typed request
AleoPrivateComputeProvider
        ↓
Privy-owned internal execution context
        ↓
dedicated worker
        ↓
Aleo JS/WASM SDK
        ↓
Aleo program
        ↓
minimal result + optional proof material
```

The internal execution context must not be exposed as a normal website origin. It must use only packaged/trusted resources and must not load arbitrary remote JavaScript.

## Why an internal browser execution context first

The current Aleo web SDK already targets browser WebAssembly. Reusing that supported execution environment is substantially smaller and less fragile than immediately embedding a separate JavaScript runtime or creating a custom C++/Rust FFI layer inside Chromium.

The provider interface intentionally allows a later migration to a dedicated utility/native process without changing the browser privacy policy architecture.

## No wallet-first UX

Local private computation must not require users to connect a wallet just to browse.

Where Aleo SDK internals require account/compute material for local execution, the provider may use purpose-scoped local/ephemeral material that is not presented as a user wallet.

A visible signing flow is reserved for operations whose semantics genuinely require user authorization or an Aleo network transaction.

## When the Aleo network is justified

Use a network transaction only when the feature needs a shared/verifiable state transition such as:

- creating/consuming a private record;
- credential/state issuance or revocation;
- a private payment;
- globally enforceable one-time state;
- another operation that requires network consensus rather than a browser-local answer.

Do not create transactions for ordinary page views, tracker blocking, local personalization, or local frequency decisions solely to demonstrate blockchain usage.

## Initial capabilities

### `frequency_cap`

Private inputs:

- prior local impression count;
- configured maximum.

Public/request context:

- campaign commitment/identifier approved for disclosure.

Output:

- boolean: eligible to show again.

### `conversion_attribution`

Private inputs:

- locally retained exposure state;
- conversion condition.

Output:

- minimal attribution result/proof.

The surrounding navigation trail is not returned.

### `telemetry_bucket`

Private inputs:

- local event count or measurement.

Output:

- approved bucket/count representation.

### `personalization_match`

Private inputs:

- local interest/segment state.

Output:

- match/no-match or another coarse approved result.

## Program packaging

Aleo programs used by Privy should live under `aleo/programs/` and be versioned with the browser code.

The browser must know exactly which program/version implements each capability. Remote sites must not be able to swap in arbitrary Aleo source and ask it to run against browser-private data.

## Proving keys and caches

Large proving resources need explicit lifecycle management:

- package trusted static resources when feasible;
- cache only by known program/version;
- verify integrity before use;
- keep caches separate from page-controlled storage;
- expose cache size/cleanup in browser settings if it becomes material.

## Timeouts and resource limits

A site must not be able to turn private computation into a browser DoS primitive.

Each capability must define:

- maximum input size;
- maximum concurrent jobs per profile/origin;
- execution timeout;
- cancellation behavior;
- memory budget where practical;
- user-visible fallback when computation cannot finish.

Failure remains privacy-safe: no raw input fallback.

## Network submission

Local execution and network submission are separate operations.

If a capability later submits an execution/record transition to Aleo, the adapter must expose that as an explicit mode and the browser UI must make any required authorization understandable. Network submission must never be an accidental side effect of asking for a local result.
