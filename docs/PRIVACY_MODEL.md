# Privacy Model

## Principle

Privy Browser treats raw behavioral data as a high-cost disclosure.

A requester should receive the minimum information required to complete a legitimate operation. When the operation can be answered locally, Privy should prefer a local decision or private computation over releasing the underlying data.

## What counts as behavioral data

Examples include:

- browsing/navigation history;
- page and content interactions;
- search and query history;
- ad impressions and clicks;
- conversion journeys;
- session timing and frequency;
- inferred interests or segments;
- purchase/product interaction patterns;
- cross-site correlations;
- extension-observed page data;
- browser state that can become a durable tracking signal.

Identity data can also be sensitive, but identity is not the organizing concept of Privy Browser.

## Disclosure ladder

For a privacy-sensitive request, prefer the highest applicable level in this order:

1. **Block** — requester does not need the operation or the operation is disallowed.
2. **Local decision** — browser can answer internally without exposing anything.
3. **Sanitized result** — browser returns a coarse/non-identifying result.
4. **Private computation** — browser computes/proves the allowed result using sensitive local inputs.
5. **Prompted disclosure** — user knowingly authorizes a specific raw disclosure.
6. **Raw disclosure** — only when required for the requested functionality and allowed by policy/user choice.

A compute-provider failure must never silently move a request downward into raw disclosure.

## Example: frequency capping

A site wants to know whether campaign X may be shown again.

Traditional approach:

```text
stable identifier + impression log -> ad/analytics service -> decision
```

Privy approach:

```text
campaign request
      ↓
local private impression state
      ↓
private computation: prior_impressions < limit
      ↓
allowed = true/false
```

The exact impression history stays inside the browser.

## Example: conversion attribution

A site wants evidence that a conversion followed an eligible campaign exposure.

Traditional approach links the exposure and conversion through an identifier and usually preserves a user journey.

Privy instead aims to retain the exposure state locally and return a minimal attribution result/proof. The website should not automatically receive the surrounding browsing history or a reusable cross-site identifier.

## Example: telemetry

Telemetry is not inherently malicious. Websites need operational/product information. The privacy problem is collecting far more user-level event data than the metric requires.

Privy-aware telemetry should support requests such as:

```text
Was this a valid page view?
Which coarse duration bucket applies?
How many local events fall into this approved bucket?
Did an approved conversion condition occur?
```

rather than exporting the complete event trail.

## Cookies and storage

Privy does not claim that all cookies are bad or replaceable by zero knowledge.

Useful first-party session state should continue to work. Privy focuses on:

- cross-site persistence;
- tracking identifiers;
- unnecessary durability;
- behavioral/profile data encoded into client state;
- cases where the website needs a result rather than the underlying stored history.

## Extensions

Installed extensions are software with browser privileges, not automatically trusted recipients of all browser data.

Privy should make a distinction between:

- permission to perform an operation locally;
- permission to read sensitive browser data;
- permission to transmit derived/raw data outside the browser.

An extension may eventually be able to ask a private-compute question without receiving the raw browsing state used to answer it.

## Private state

Privy-owned private state should be:

- profile scoped;
- purpose scoped;
- origin/capability scoped where applicable;
- encrypted at rest using platform/browser storage facilities where feasible;
- excluded from sync or cloud features unless a separately designed encrypted sync model exists;
- never copied into ordinary diagnostic logs.

## Incognito/private profiles

Private windows must not silently share durable Privy behavioral state with the normal profile. Capabilities that require durable state must define explicit incognito semantics.

Default direction: ephemeral state that disappears with the private profile.

## Network privacy boundary

Private computation does not hide the network connection itself.

Privy Browser does not claim that Aleo/ZK automatically hides:

- IP address;
- DNS queries;
- TLS endpoints;
- the fact that a request reached a server;
- page contents from a server the user intentionally visits.

Network anonymity is a separate layer.

## Success condition

A privacy feature is meaningful when a website or extension can still perform a useful operation while learning materially less about the user's underlying behavior.
