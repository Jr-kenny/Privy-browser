# Server-Hosted Mode

Server-hosted mode runs the **same Linux Privy Browser binary** on a remote machine and streams its graphical session to the user.

It is not a second web implementation of the browser.

## Shape

```text
User device
   ↓ HTTPS/WebRTC
streaming gateway
   ↓
Linux display/session
   ↓
Privy Browser
   ↓
Internet
```

The Privy profile, cookies, browsing state, extensions, and private-compute runtime live on the remote machine because that machine is the one actually running Chromium.

## Trust model

The remote machine is inside the user's trust boundary.

A host administrator with sufficient privilege can potentially inspect the browser process or its profile. Server-hosted mode therefore does not claim confidentiality from the VPS provider/administrator unless a future confidential-compute design explicitly adds that property.

## Why this mode exists

- let judges/users try the real browser without compiling Chromium;
- provide a self-hosted browser that can persist independently of the client device;
- support access from thin/temporary devices;
- allow the same browser profile to run in user-controlled infrastructure;
- create a deployment path for later managed/self-hosted offerings without changing the privacy engine.

## Streaming layer

Selkies is the current reference implementation for Linux application/desktop streaming because it provides an HTML5/WebRTC path and supports self-hosted/container deployments.

Privy should integrate it as replaceable transport infrastructure. Do not couple privacy logic to Selkies-specific APIs.

## Deployment target

The desired eventual UX is intentionally simple:

```bash
docker compose up -d
```

then visit a configured HTTPS URL and interact with the running Privy Browser session.

Do not publish a server image until the following are explicit:

- authentication;
- TLS termination;
- persistent profile volume;
- per-session isolation;
- clipboard/file transfer policy;
- browser sandbox requirements inside containers;
- WebRTC/TURN configuration;
- resource limits;
- teardown behavior;
- whether the deployment is single-user or multi-user.

## Judge/demo deployment

A hackathon-hosted instance may run one disposable profile/session behind authentication. The submission must state clearly:

- the browser is running on the submitted server;
- the web page is a stream/control surface for that browser;
- the same source builds a native local browser;
- privacy computation happens on the machine running Privy Browser (the server in this mode, the user's device in native mode).

## Security notes

Do not disable the Chromium sandbox merely because the browser runs inside a container.

If container restrictions make the Chromium sandbox difficult, solve the container configuration rather than normalizing `--no-sandbox` as production behavior.

Remote debugging ports must not be exposed publicly.

The streaming gateway must not become a second channel that logs keystrokes, URLs, clipboard contents, or frames for analytics.

## Phased implementation

### Phase 1

Single-user Linux VPS session:

- Privy Browser binary;
- X11/Wayland-compatible virtual display as required by the streamer;
- Selkies;
- authenticated HTTPS entry point;
- persistent profile volume;
- no public DevTools/debug port.

### Phase 2

Container image and reproducible compose deployment.

### Phase 3

Optional multi-session manager with one isolated profile/container per user/session.

A multi-user control plane is not required for Privy Browser itself and should remain a separate server product layer.
