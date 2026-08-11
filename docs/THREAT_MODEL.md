# Threat Model

## Assets we protect

- browsing and navigation history;
- interaction/event history;
- local interest and personalization state;
- cookies and other persistent site state;
- extension-observed sensitive data;
- private-compute inputs;
- Aleo compute/account material used internally;
- privacy policy/preferences;
- local privacy activity records.

## Adversaries

### Tracking website or embedded third party

May attempt to correlate visits, create durable identifiers, collect high-resolution interaction events, fingerprint the browser, or request more information than a feature requires.

### Installed extension

May possess broad permissions and attempt to read or export page/browser data beyond what the user expects.

### Compromised page/renderer

A malicious page may exploit web APIs or a renderer vulnerability. Privy must not weaken Chromium process isolation or create a direct path from a renderer into browser-private state.

### Malicious private-compute request

A site may repeatedly request expensive computations, attempt to encode arbitrary queries, or use outputs as a high-entropy fingerprinting oracle.

### Network observer

Can potentially observe endpoints, timing, IP addresses, and other transport metadata depending on the network. Aleo private computation does not solve this class by itself.

### Host machine administrator/malware

Outside the protection boundary for ordinary native mode. If the OS or account running the browser is compromised, Privy cannot guarantee confidentiality of local state.

For server-hosted mode, the remote server is explicitly inside the user's trust boundary.

## Security invariants

1. **No raw fallback.** Failure of private computation cannot silently disclose the private input.
2. **No arbitrary private-data programs.** Sites request named capabilities, not arbitrary code execution against browser-private state.
3. **No site-controlled runtime code.** The Aleo execution host loads browser-packaged/verified resources only.
4. **No UI-thread proving.** Expensive compute cannot freeze the trusted browser UI path.
5. **No renderer direct access.** Browser-private state stays behind browser-process authorization.
6. **No weakened Chromium security.** Site Isolation, renderer sandboxing, TLS/certificate validation, and other security primitives remain intact.
7. **No hidden Privy telemetry.** Privy must not solve third-party surveillance by becoming another surveillance endpoint.
8. **Minimal proof outputs.** Proof/result APIs should avoid producing unnecessarily high-entropy stable values that become fingerprints.

## Abuse cases and mitigations

### Compute DoS

**Attack:** page floods private-compute requests.

**Mitigation:** per-origin/profile concurrency limits, capability-specific rate limits, cancellation, timeouts, and bounded inputs.

### Capability probing as fingerprint

**Attack:** site queries many capabilities/results to infer a unique profile.

**Mitigation:** coarse outputs, request budgets, per-site policy, entropy review, optional prompting for sensitive/high-entropy capabilities.

### Extension exfiltration

**Attack:** extension reads sensitive page/browser data and sends it remotely.

**Mitigation direction:** runtime access ledger, per-site grants, sensitive egress classification, user controls, and private-compute alternatives.

### Cross-site state resurrection

**Attack:** tracker recreates an identifier through multiple storage surfaces.

**Mitigation direction:** partitioning, coordinated storage policy, anti-fingerprinting, and clear-site-data semantics across Privy-owned state.

### Malicious Privy-aware website

**Attack:** website labels surveillance as a legitimate private-compute request.

**Mitigation:** fixed browser-defined capabilities and disclosure policy. A website's declared purpose is metadata, not trusted authorization.

### Proof replay/correlation

**Attack:** a proof becomes reusable as a stable cross-site identifier.

**Mitigation direction:** origin/campaign scoped public inputs, freshness/nonces where appropriate, avoid globally stable proof payloads, and do not expose internal account identifiers unnecessarily.

## Out of scope for Aleo layer

The initial Aleo privacy runtime does not claim to provide:

- IP anonymity;
- onion routing;
- VPN functionality;
- protection from a compromised OS;
- protection from a server that must see the plaintext content of a normal HTTPS request;
- magical anonymity for information the user explicitly posts to a website.

These may be addressed by separate browser/network features, but should never be attributed to ZK computation itself.

## Server-hosted mode

When Privy runs on a VPS, the VPS processes page content, browser profile state, and private-compute inputs. The server operator therefore has host-level power over that session unless additional confidential-compute infrastructure is explicitly added later.

Do not market remote streaming as end-to-end secrecy from the host.

## Review requirement

Any new privacy capability must answer:

1. What private inputs are used?
2. Who chooses them?
3. What is returned?
4. How much identifying entropy can the output carry?
5. Is the result linkable across origins/sessions?
6. What happens if compute fails?
7. Does it create network traffic?
8. Does it persist state?
9. How is incognito handled?
10. Can a site abuse it for resource exhaustion or tracking?
