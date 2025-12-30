## JackTrip network protocol (as implemented)

This document describes JackTrip’s **on-the-wire protocol** as implemented in the current source tree. It is intended for developers debugging or interoperating with JackTrip at the packet level.

### Scope and non-goals

- **In scope**: the real-time **UDP audio stream**, its headers and payload layout, the optional **UDP redundancy** framing, the small **UDP “stop” control packet**, and the **TCP handshake** used by hub/ping-server style deployments (including the authentication variant).
- **Out of scope**: local-only IPC (e.g. `QLocalSocket` “AudioSocket”), OSC control, and any higher-level application semantics outside packet exchange.

### Transports at a glance

- **UDP (audio)**: real-time audio is sent as UDP datagrams containing `PacketHeader` + raw audio payload.
- **UDP (control)**: a small fixed-size “stop” datagram is used to signal shutdown.
- **TCP (hub/ping-server handshake)**: a short-lived TCP connection is used to exchange UDP port information (and optionally do TLS + credentials).

---

## UDP audio datagrams

### High-level framing

Each UDP datagram carries one of:

- **Audio datagram**: one or more **full packets** (header + audio payload). When redundancy is disabled, there is exactly one full packet per UDP datagram. When redundancy is enabled, multiple full packets are concatenated into a single UDP datagram (see “UDP redundancy”).
- **Stop/control datagram**: exactly 63 bytes of `0xFF` (see “UDP stop/control datagram”).

### Packet header types

The header is selected by `DataProtocol::packetHeaderTypeT`:

- **DEFAULT**: `DefaultHeaderStruct` (the standard JackTrip header).
- **JAMLINK**: `JamLinkHeaderStuct` (JamLink compatibility).
- **EMPTY**: no header (payload only).

See `src/PacketHeader.h` and `src/PacketHeader.cpp`.

### Default header (`DEFAULT`)

On-wire layout is the in-memory `DefaultHeaderStruct` copied with `memcpy()` (no explicit endian conversions).

Fields (in order):

| Field | Type | Meaning |
|------:|------|---------|
| `TimeStamp` | `uint64_t` | Timestamp in microseconds since Unix epoch (see `PacketHeader::usecTime()`). |
| `SeqNumber` | `uint16_t` | Sequence number; increments once per audio period and wraps at 16 bits. |
| `BufferSize` | `uint16_t` | Audio period size \(N\) in **samples per channel**. |
| `SamplingRate` | `uint8_t` | Encoded sample-rate enum value (`AudioInterface::samplingRateT`), **not** Hz. |
| `BitResolution` | `uint8_t` | Bits per sample (8/16/24/32). |
| `NumIncomingChannelsFromNet` | `uint8_t` | Channel count expected from the peer “from network” direction (see notes below). |
| `NumOutgoingChannelsToNet` | `uint8_t` | Channel count the sender is placing into the payload (see notes below). |

#### Important interoperability notes

- **Endianness / ABI**: this header is serialized by raw `memcpy()` of a C struct. In practice this assumes:
  - both sides are using compatible ABI/layout for the struct, and
  - both sides are on the same endianness (typically **little-endian** on modern desktop platforms).
- **Channel fields are asymmetric**: the implementation uses these fields to convey “incoming vs outgoing” channel counts, including a couple of sentinel behaviors:
  - `NumIncomingChannelsFromNet` is populated from local *output* channel count.
  - `NumOutgoingChannelsToNet` may be set to `0` when in/out channel counts match, or to `0xFF` when there are zero input channels.

These behaviors come from `DefaultHeader::fillHeaderCommonFromAudio()` in `src/PacketHeader.cpp`.

### JamLink header (`JAMLINK`)

JamLink uses a compact header:

| Field | Type | Meaning |
|------:|------|---------|
| `Common` | `uint16_t` | Bitfield describing mono/stereo, bit depth, sample rate, and samples-per-packet (JamLink “streamType”). |
| `SeqNumber` | `uint16_t` | Sequence number. |
| `TimeStamp` | `uint32_t` | Timestamp. |

The current implementation primarily fills this for JamLink constraints (mono, 48kHz, 64-sample buffers). See `JamLinkHeader::fillHeaderCommonFromAudio()` in `src/PacketHeader.cpp`.

### Empty header (`EMPTY`)

No header; the UDP payload is raw audio data only.

---

## UDP audio payload

### Size

For a single full packet (no redundancy), the UDP payload length is:

\[
\text{headerBytes} + (N \times C \times \text{bytesPerSample})
\]

Where:

- \(N\) is `BufferSize` (samples per channel)
- \(C\) is the number of channels present in the payload
- `bytesPerSample` is `BitResolution / 8`

### Channel/sample ordering (planar / non-interleaved)

On the wire, the payload is **planar** (non-interleaved) by channel:

- First \(N\) samples for channel 0
- Then \(N\) samples for channel 1
- …

This is explicit in `UdpDataProtocol` which converts between:

- **Internal**: interleaved layout \([n][c]\)
- **Network**: planar layout \([c][n]\)

See `UdpDataProtocol::sendPacketRedundancy()` and `UdpDataProtocol::receivePacketRedundancy()` in `src/UdpDataProtocol.cpp`.

### Sample encoding (bit resolution)

JackTrip processes audio internally as `float` (`sample_t`), but the network payload uses the selected bit resolution via `AudioInterface::fromSampleToBitConversion()` / `fromBitToSampleConversion()`.

Behavior by bit resolution (`AudioInterface::audioBitResolutionT`):

- **8-bit (`BIT8`)**: signed 8-bit integer, scaled from float in \([-1, 1]\).
- **16-bit (`BIT16`)**: signed 16-bit integer, written **little-endian**.
- **24-bit (`BIT24`)**: a **non-standard 3-byte format**: a 16-bit signed integer plus an 8-bit unsigned “remainder” byte.
- **32-bit (`BIT32`)**: raw 32-bit float bytes (`memcpy` of `float`), which implicitly assumes IEEE-754 and matching endianness.

See `src/AudioInterface.cpp`.

---

## UDP redundancy (optional)

JackTrip can send redundant audio packets to reduce audible artifacts from packet loss.

### Framing

With redundancy factor \(R\), each UDP datagram contains **R full packets** concatenated:

- The newest packet is first (`UDP[n]`), followed by older packets (`UDP[n-1]`, …).
- Total UDP payload length becomes `R * full_packet_size`.

The sender implements this by shifting a buffer and prepending the newest full packet each period.

See `UdpDataProtocol::sendPacketRedundancy()` and the explanatory comment block in `src/UdpDataProtocol.cpp`.

### Receiver behavior

Upon receiving a redundant datagram, the receiver:

- Reads the first packet’s `SeqNumber`.
- If it is not the next expected sequence, scans forward through the concatenated packets looking for the expected next one.
- May “revive” and deliver multiple packets from the redundant datagram in order.
- Treats large negative or implausibly large sequence jumps as **out-of-order** and ignores them.

See `UdpDataProtocol::receivePacketRedundancy()` in `src/UdpDataProtocol.cpp`.

---

## UDP stop/control datagram

JackTrip uses a special fixed-size UDP datagram to signal shutdown:

- **Length**: 63 bytes
- **Contents**: every byte is `0xFF`

The receiver checks for this exact pattern and treats it as “Peer Stopped”.

See `UdpDataProtocol::processControlPacket()` and the shutdown path in `UdpDataProtocol::run()` in `src/UdpDataProtocol.cpp`.

---

## Connection setup and “handshake”

JackTrip supports multiple deployment styles. The relevant “protocol” differs depending on mode.

### P2P server mode (UDP-only)

In P2P server mode, there is **no TCP handshake**. Instead:

- The server binds a UDP socket on its configured receive port.
- It waits for the first UDP datagram.
- It uses the datagram’s source address/port as the peer endpoint for subsequent UDP send/receive.

This supports basic NAT traversal by responding to the client’s observed source port.

See `JackTrip::serverStart()` and `JackTrip::receivedDataUDP()` in `src/JackTrip.cpp`.

### Hub / ping-server mode (TCP handshake + UDP audio)

When connecting to a hub/ping-server style endpoint, JackTrip uses a short-lived TCP connection to exchange UDP port information.

#### Unauthenticated handshake (no TLS)

Client → server (TCP):

- `int32` little-endian: the client’s UDP receive/bind port
- `gMaxRemoteNameLength` bytes: optional UTF-8 “remote client name” (null-terminated, padded with zeros)

Server → client (TCP):

- `int32` little-endian: the server-assigned UDP port the client should use as its peer port

The TCP connection is then closed.

Client-side send/receive logic: `JackTrip::receivedConnectionTCP()` and `JackTrip::receivedDataTCP()` in `src/JackTrip.cpp`  
Server-side receive/send logic: `UdpHubListener::readClientUdpPort()` and `UdpHubListener::sendUdpPort()` in `src/UdpHubListener.cpp`

#### Authentication / TLS handshake (optional)

This is an extension of the same TCP handshake using values above 65535 as “auth response” codes.

High-level flow:

1. Client connects TCP and sends an `int32` little-endian value of `Auth::OK` to request authentication.
2. Server replies with an `int32` auth response (e.g. `Auth::OK`, `Auth::NOTREQUIRED`, `Auth::REQUIRED`, …).
3. If both sides proceed, TLS is established on the same TCP socket.
4. Client then sends:
   - `int32` LE: UDP receive/bind port
   - `gMaxRemoteNameLength` bytes: client name
   - `int32` LE: username length (excluding null terminator)
   - `int32` LE: password length (excluding null terminator)
   - `username` bytes + `\0`
   - `password` bytes + `\0`
5. Server validates credentials and replies with either:
   - `int32` LE UDP port (<= 65535) on success, or
   - `int32` LE auth error code (> 65535) on failure

Client-side: `JackTrip::receivedConnectionTCP()`, `JackTrip::connectionSecured()`, and `JackTrip::receivedDataTCP()` in `src/JackTrip.cpp`  
Server-side: `UdpHubListener::receivedClientInfo()`, `UdpHubListener::checkAuthAndReadPort()`, and `UdpHubListener::sendUdpPort()` in `src/UdpHubListener.cpp`

---

## QoS marking (best-effort)

On supported platforms, JackTrip attempts to mark UDP packets as “voice” traffic:

- Linux/Unix: sets DSCP to 56 (`IP_TOS` / `IPV6_TCLASS` set to `0xE0`), and sets `SO_PRIORITY` to 6.
- Windows: uses QOS APIs with `QOSTrafficTypeVoice`.
- macOS: uses `SO_NET_SERVICE_TYPE` with `NET_SERVICE_TYPE_VO` (best-effort).

See `src/UdpDataProtocol.cpp`.


