# Engineering References

These projects are references for architecture, build strategy, privacy surfaces, or remote delivery. They are **not** source donors by default.

The rule is: understand how a solved problem is structured, then implement the Privy requirement independently unless we deliberately accept the referenced code and its license obligations.

## Chromium

Repository: `chromium/chromium` / canonical source at `chromium.googlesource.com/chromium/src`

Use for:

- actual browser engine and desktop UI;
- tabs/omnibox/profiles/extensions/network/storage/DevTools;
- process sandbox and Site Isolation;
- WebUI and profile keyed service patterns.

Privy is a Chromium derivative, so Chromium's own licensing/notices remain part of distributed builds as required.

## Helium

Repository: `imputnet/helium`

Current license: GPL-3.0.

Use as a reference for:

- maintaining a Chromium derivative without vendoring the entire Chromium source tree;
- version pinning;
- ordered patch tooling;
- build/release automation;
- where a modern Chromium browser chooses to patch upstream.

Do **not** copy Helium patches into Privy unless we intentionally accept GPL-derived code and preserve the required licensing.

Observed useful pattern: Helium keeps a Chromium version file plus patch/build tooling around an upstream checkout. Privy adopts the architectural idea, not Helium's implementation.

## Cromite

Repository: `uazo/cromite`

Current license: GPL-3.0.

Use as a reference for:

- anti-fingerprinting surfaces;
- tracker/ad blocking integration points;
- privacy-oriented Chromium patch locations;
- Android/privacy behavior research.

Cromite/Bromite patches are not a default copy source.

## Thorium

Repository: `Alex313031/Thorium`

Use as a reference for:

- cross-platform Chromium derivative build/release practices;
- product branding/build flags;
- keeping a browser usable while rebasing Chromium.

Verify the exact license of any file before reuse; do not infer file-level permissions from project reputation.

## Kiwi Browser

Repository: `kiwibrowser/src.next` (archived)

Use as a reference for:

- Android Chromium browser UX;
- extension support on mobile;
- mobile-specific browser product decisions.

Do not use Kiwi as the desktop foundation.

## Selkies

Repository: `selkies-project/selkies`

License: MPL-2.0.

Use as a reference/possible dependency for:

- low-latency WebRTC Linux application/desktop streaming;
- containerized self-hosted remote browser delivery;
- HTML5 control surface for the server-hosted mode.

Selkies belongs to the optional server distribution layer, not Privy's privacy architecture.

## Provable / Aleo SDK

Repository: `ProvableHQ/sdk`

Current repository license: GPL-3.0.

Use for:

- browser-side Aleo program execution;
- WASM-backed private computation;
- worker-based proving/execution patterns;
- optional Aleo network interactions.

Important licensing decision: bundling GPL-licensed SDK code into a distributed browser has consequences for the combined distribution. Before the SDK is committed as a vendored/npm/build dependency, the project license/distribution strategy must be explicitly compatible.

## snarkVM

Repository: `ProvableHQ/snarkVM`

Current repository package metadata uses Apache-2.0 for `snarkvm`.

Use as research for a possible lower-level/native/WASM Aleo provider if the JavaScript SDK's licensing or integration model becomes undesirable.

This path is likely more engineering work than using the supported web SDK, so it is not the first hackathon implementation unless licensing forces the choice.

## Reference policy for agents

Before copying any non-trivial external code:

1. identify the exact source file/commit;
2. identify its file/project license;
3. determine whether that license is compatible with Privy's distribution;
4. preserve required notices/source availability;
5. document the imported code in a dedicated third-party notice;
6. avoid copying when the same behavior can be implemented cleanly from Chromium/public API understanding.

Studying architecture does not mean importing implementation.
