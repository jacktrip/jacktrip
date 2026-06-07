# Plan: Defer WebTransport worker creation until session is established

## Background / motivation

The HUB server currently allocates a full `JackTripWorker` for **every incoming QUIC
connection**, at `QUIC_LISTENER_EVENT_NEW_CONNECTION` — i.e. *before* the QUIC/TLS
handshake and HTTP/3 `CONNECT` complete.

Observed symptom: Firefox on Fedora opens **two** QUIC connections from the same client
IP (different source ports). One completes the WebTransport handshake and streams audio;
the other is abandoned and dies with `SHUTDOWN_INITIATED_BY_TRANSPORT` ("Transport
shutdown"). Because a worker is created up-front, the abandoned connection:

- transiently bumps `mTotalRunningThreads` (the misleading `Total Running Threads: 2`
  for a single client),
- allocates a heavyweight `JackTrip` instance (audio interface, ring buffers) that is
  immediately torn down,
- produces "session failed" / shutdown log spam.

This is benign today (the original wrong-slot cleanup bug was fixed by assigning the
worker's slot id in `createWorker`, see `UdpHubListener::createWorker` →
`worker->setID(id)`), but it is wasteful and noisy. It also makes the server fragile to
any client (or scanner/probe) that opens speculative or half-open QUIC connections, not
just Firefox.

**Goal:** only the lightweight `WebTransportSession` is created at `NEW_CONNECTION`. The
heavyweight `JackTripWorker` + slot allocation is deferred until the session actually
reaches `sessionEstablished` (HTTP/3 `CONNECT` accepted with status 200). Connections that
never establish never consume a worker slot or bump the thread count.

## Current flow (for reference)

1. `Http3Server` listener callback `QUIC_LISTENER_EVENT_NEW_CONNECTION`
   (`src/http3/Http3Server.cpp:239`) accepts the connection, sets its config, then invokes
   `mConnectionCallback(connection, addr, port)`.
2. That callback is `UdpHubListener::createWebTransportWorker(...)`
   (`src/UdpHubListener.cpp:991`), which:
   - calls `createWorker()` → finds a free slot, `new JackTripWorker`, `setID(id)`,
     `mTotalRunningThreads++`, stores in `mJTWorkers[id]`;
   - `worker->moveToThread(listenerThread)`;
   - connects `signalRemoveThread → handleWorkerRemoval`;
   - `new WebTransportSession(quicApi, connection, addr, port, nullptr)` — the session
     registers itself as the QUIC connection callback handler in its constructor
     (`src/webtransport/WebTransportSession.cpp:122`);
   - `session->moveToThread(listenerThread)`;
   - `worker->createWebTransportSession(session)` — reparents session to worker and wires
     `sessionEstablished/Closed/Failed` to the worker's slots
     (`src/JackTripWorker.cpp:607`).
3. On `sessionEstablished`, `JackTripWorker::onWebTransportSessionEstablished()` configures
   `JackTrip` (ports, channels, protocol) but does **not** start audio yet.
4. On the first datagram, `receivedFirstPacketWebTransport → processPeerSettings →
   startProcess` actually starts the audio pipeline (`mRunning = true`).

The key insight: **the `WebTransportSession` must exist at `NEW_CONNECTION`** (it is the
QUIC callback handler — without it, nothing services the handshake). But the
`JackTripWorker` is only needed once the session is established.

## Proposed design

### Ownership of the pending session

Introduce a "pending session" stage owned by `UdpHubListener` (it already owns the
`Http3Server` and is the connection delegate):

- Add a container, e.g. `QHash<WebTransportSession*, /*placeholder*/ bool>` or simply a
  `QSet<WebTransportSession*> mPendingWtSessions`, guarded by `mMutex`.
- At `NEW_CONNECTION` (replace `createWebTransportWorker`):
  1. `auto* session = new WebTransportSession(quicApi, connection, addr, port, nullptr);`
  2. `session->moveToThread(this->thread());`
  3. Connect, with `Qt::QueuedConnection`:
     - `session->sessionEstablished → UdpHubListener::onWebTransportSessionEstablished(session)`
     - `session->sessionFailed     → UdpHubListener::onPendingSessionGone(session)`
     - `session->sessionClosed     → UdpHubListener::onPendingSessionGone(session)`
  4. Insert into `mPendingWtSessions`.
  5. **Do not** call `createWorker()`, do not bump `mTotalRunningThreads`.

   Because `sessionEstablished` is connected via a queued connection, it is delivered on
   the listener thread's event loop, so worker creation happens on the right thread.

### Promotion to a worker (on establish)

`UdpHubListener::onWebTransportSessionEstablished(WebTransportSession* session)`:

1. Under `mMutex`, remove `session` from `mPendingWtSessions`. If it was not present
   (already failed/closed), bail out.
2. `int id = createWorker(tempName);` (this assigns slot + `setID(id)` +
   `mTotalRunningThreads++`).
   - If `id < 0` (no free slots), `session->close(); session->deleteLater();` and return —
     reject gracefully.
3. `JackTripWorker* worker = mJTWorkers->at(id);`
4. `worker->moveToThread(this->thread());`
5. `connect(worker, signalRemoveThread, this, handleWorkerRemoval, QueuedConnection);`
6. **Re-wire the session to the worker.** The session is already CONNECTED, so the
   worker's `createWebTransportSession()` "already connected → onWebTransportSessionEstablished"
   fast-path (`src/JackTripWorker.cpp:644`) will run and configure `JackTrip`. Verify the
   signal connections set up inside `createWebTransportSession` (sessionEstablished/Closed/
   Failed → worker slots) plus the datagram callback are correct given the session is
   already established. The `isConnected()` branch already exists for exactly this case.
7. Disconnect the temporary `UdpHubListener`-side session signal connections from step
   "pending" (so they don't double-fire alongside the worker's connections).

### Cleanup of a pending session that never establishes (on fail/close)

`UdpHubListener::onPendingSessionGone(WebTransportSession* session)`:

1. Under `mMutex`, if `session` is still in `mPendingWtSessions`, erase it and
   `session->deleteLater();`. No worker, no thread-count change, minimal logging
   (gate behind `gVerboseFlag`).
2. If it is **not** in the set, it was already promoted to a worker — ignore (the worker's
   own `onWebTransportSessionFailed/Closed` path handles teardown).

### Thread-safety notes

- The session is created on the msquic thread (the listener callback runs there), then
  `moveToThread(this->thread())`. All subsequent signal handling is queued onto the
  listener thread — same pattern as today.
- `mPendingWtSessions` mutations must be under `mMutex` (consistent with `mJTWorkers`
  access).
- There is an inherent race: `sessionFailed` and `sessionEstablished` could both be
  emitted. Using set membership as the single source of truth (whoever removes it first
  wins) resolves it: establish promotes, fail/close frees, and the second handler sees the
  session is no longer pending and no-ops.

## Files to change

| File | Change |
|------|--------|
| `src/UdpHubListener.h` | Add `QSet<WebTransportSession*> mPendingWtSessions;` member; declare `onWebTransportSessionEstablished(WebTransportSession*)` and `onPendingSessionGone(WebTransportSession*)` slots; keep/retire `createWebTransportWorker`. |
| `src/UdpHubListener.cpp` | Replace `createWebTransportWorker` body with "create pending session only"; add the two new slots; ensure the `Http3Server` connection callback (`src/UdpHubListener.cpp:289`) calls the new entry point. |
| `src/JackTripWorker.cpp` / `.h` | Likely no structural change — `createWebTransportSession()` already handles the "already connected" case. Confirm the established-on-attach path fully configures and that no second `sessionEstablished` is required. |
| `src/webtransport/WebTransportSession.*` | No change expected; it already emits `sessionEstablished/Failed/Closed` and self-registers as the QUIC handler. |

## Testing / verification

1. **Single Chrome/Safari client (baseline):** one QUIC connection → `Total Running
   Threads: 1`, audio streams. No regression.
2. **Firefox on Fedora (repro case):** two QUIC connections; the abandoned one should now
   produce **no** worker and **no** thread-count bump. Expect at most a single gated
   "pending session failed" verbose line, and `Total Running Threads: 1`.
3. **Slot exhaustion:** with `-p N` and N active clients, an additional establishing
   session should be rejected cleanly (`session->close()`), not crash.
4. **Rapid connect/disconnect:** client that connects and immediately drops mid-handshake
   — pending session is freed, no leak. Watch for the
   `WebTransportDataProtocol::stop: thread did not finish within 1s` warning (should not
   appear).
5. **Probe/scan:** point a QUIC scanner at the port — connections that never send a
   `CONNECT` should be reaped by msquic idle timeout with no worker allocation.
6. Run under `-V` (verbose) to confirm the gated diagnostics tell a coherent story:
   pending-created → established → worker N, or pending-created → gone.

## Out of scope / notes

- This does **not** address *why* Firefox-on-Fedora opens a second connection — that is a
  client/browser behavior (suspected Firefox QUIC connection racing) and cannot be fixed
  server-side. This plan only makes the server robust and quiet in the face of it.
- The same up-front-allocation pattern exists for **WebRTC** (`createWebRtcWorker`,
  `src/UdpHubListener.cpp:948`). If WebRTC shows similar speculative-connection churn, a
  parallel deferral could be applied, but it is not covered here.
- Keep the `mID`/slot-assignment fix (`createWorker` → `setID(id)`) regardless; it is
  independent and already correct.
