# JackTrip Hub Server Connection Types

## Overview

The JackTrip Hub Server supports three different connection types for clients, each with different characteristics suited to different deployment scenarios:

1. **UDP** - Traditional low-latency UDP transport with TCP signaling
2. **WebRTC** - Browser-compatible connection with NAT traversal using WebRTC data channels
3. **WebTransport** - Modern HTTP/3 transport using QUIC with built-in encryption

All three connection types use the same audio packet format (see [NetworkProtocol.md](NetworkProtocol.md)) and share the same worker pool allocation mechanism.

## Connection Type Comparison

| Feature | UDP | WebRTC | WebTransport |
|---------|-----|--------|--------------|
| **Transport** | UDP datagrams | WebRTC data channels over UDP | QUIC datagrams over UDP |
| **Signaling** | TCP port 4464 | WebSocket over TCP 4464 | HTTP/3 over UDP 4464 |
| **NAT Traversal** | No | Yes (ICE/STUN/TURN) | Yes (QUIC connection migration) |
| **Browser Support** | No | Yes (all modern browsers) | Yes (Chrome 97+, Edge 97+) |
| **Encryption** | Optional (TLS) | Mandatory (DTLS) | Mandatory (TLS 1.3) |
| **Setup Complexity** | Simple | Complex (ICE negotiation) | Medium (HTTP/3 CONNECT) |
| **Connection Time** | Fastest | Medium (ICE gathering) | Fast (0-RTT after first) |
| **Audio Transport Port** | UDP (61002 + worker_id) | ICE-negotiated UDP ports | UDP 4464 (QUIC) |
| **Library Required** | None (Qt Network) | libdatachannel | msquic |
| **Build Option** | Always available | `-Dlibdatachannel=enabled` | `-Dmsquic=enabled` |

## 1. UDP Connections

### Overview

Traditional UDP connections use a simple TCP-based signaling handshake followed by direct UDP audio transport. This is the lowest-latency option but requires open firewall ports and doesn't work behind NAT without port forwarding.

### Connection Handshake

1. Client connects to TCP port 4464
2. Client sends UDP port number (4 bytes, little-endian) and optional client name (64 bytes)
3. Server allocates a worker slot and responds with server UDP port (`mBasePort + worker_id`)
4. TCP connection closes
5. Audio exchange begins over UDP using the negotiated ports

For complete details on the UDP handshake protocol, packet format, and authentication flow, see [NetworkProtocol.md](NetworkProtocol.md).

### Port Requirements

- **TCP 4464**: Signaling handshake
- **UDP 61002 + worker_id**: Audio transport (base port configurable with `--udpbaseport`)
  - Example: First client uses 61002, second uses 61003, etc.

### Authentication

Optional TLS authentication is supported. When enabled:
- Client sends special value (`65536`) instead of port to initiate SSL handshake
- Credentials (username/password) are exchanged over encrypted connection
- Server validates and responds with port assignment or error code

## 2. WebRTC Connections

### Overview

WebRTC connections use WebSocket-based signaling followed by ICE-negotiated data channels. This provides excellent NAT traversal and works from web browsers, making it ideal for browser-based clients.

### Connection Handshake

1. **WebSocket Upgrade**: Client sends HTTP upgrade request to TCP port 4464
   ```http
   GET / HTTP/1.1
   Upgrade: websocket
   Connection: Upgrade
   Sec-WebSocket-Key: <key>
   ```

2. **SDP Exchange**: Client sends SDP offer, server responds with answer
   ```json
   {
       "type": "offer",
       "sdp": "v=0\r\no=- 4611731400430051336 2 IN IP4 127.0.0.1\r\n..."
   }
   ```

3. **ICE Candidates**: Both sides exchange ICE candidates for connectivity
   ```json
   {
       "type": "ice",
       "candidate": "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host",
       "sdpMid": "data",
       "sdpMLineIndex": 0
   }
   ```

4. **Connection Establishment**: ICE performs connectivity checks, DTLS establishes encryption, SCTP creates data channel association

5. **Audio Exchange**: Audio packets are sent over the data channel using the same packet format as UDP

### Data Channel Configuration

The data channel is configured for low-latency, unreliable delivery similar to UDP:

```cpp
rtc::DataChannelInit config;
config.ordered = false;       // Don't wait for in-order delivery
config.maxRetransmits = 0;    // No retransmissions (like UDP)
```

### ICE Server Configuration

The server can be configured with STUN/TURN servers for NAT traversal:

```bash
jacktrip -S --iceservers "stun:stun.l.google.com:19302"
```

### Implementation

- **Library**: libdatachannel
- **Classes**: `WebRtcPeerConnection`, `WebRtcDataProtocol`, `WebRtcSignalingProtocol`
- **Detection**: Server detects WebSocket upgrade by checking for "GET" in initial TCP data

## 3. WebTransport Connections

### Overview

WebTransport provides modern, low-latency transport using HTTP/3 over QUIC. Unlike WebRTC, it requires no ICE negotiation and provides a simpler connection model with built-in NAT traversal. All connections use unreliable QUIC datagrams for audio transport.

**Important**: WebTransport uses UDP (not TCP) for the entire connection, including signaling.

### Connection Handshake

1. **QUIC Connection**: Client initiates QUIC connection to UDP port 4464
   - TLS 1.3 handshake (mandatory, built into QUIC)
   - 0-RTT capable after first connection

2. **HTTP/3 CONNECT**: Client sends HTTP/3 CONNECT request over QUIC
   ```
   :method = CONNECT
   :protocol = webtransport
   :path = /webtransport
   :authority = server.example.com:4464
   ```

3. **Session Established**: Server responds with 200 OK
   ```
   :status = 200
   sec-webtransport-http3-draft = draft02
   ```

4. **Audio Exchange**: Audio packets are sent as QUIC DATAGRAM frames (RFC 9221)

### QUIC Datagram Transport

Audio packets are sent as unreliable QUIC datagrams:

```
┌──────────┬────────────────────┐
│ JackTrip │   Audio Samples    │
│ Header   │                    │
│ (16B)    │   (variable)       │
└──────────┴────────────────────┘
```

**Key properties:**
- Unreliable (no retransmissions, like UDP)
- Unordered (can arrive out of sequence)
- Encrypted (TLS 1.3 via QUIC)
- Preserve datagram boundaries
- Typical size limit: 1200 bytes (path MTU)

### TLS Certificate Requirements

WebTransport requires TLS 1.3 certificates - encryption is mandatory and cannot be disabled.

**Development (self-signed):**
```bash
openssl genpkey -algorithm RSA -out webtransport.key -pkeyopt rsa_keygen_bits:2048
openssl req -new -x509 -key webtransport.key -out webtransport.crt -days 365 \
  -subj "/CN=jacktrip.example.com"

jacktrip -S --certfile webtransport.crt --keyfile webtransport.key
```

**Production (Let's Encrypt):**
```bash
sudo certbot certonly --standalone -d jacktrip.example.com

jacktrip -S \
  --certfile /etc/letsencrypt/live/jacktrip.example.com/fullchain.pem \
  --keyfile /etc/letsencrypt/live/jacktrip.example.com/privkey.pem
```

**Note**: Browsers will reject self-signed certificates unless explicitly trusted.

### Implementation

- **Library**: msquic (only supported QUIC library)
- **Classes**: `WebTransportSession`, `WebTransportDataProtocol`
- **Detection**: Server detects QUIC packets on UDP 4464 by examining packet header flags
- **Port**: Single UDP port (4464) for both signaling and audio

### Why QUIC?

QUIC provides several advantages over TCP for real-time audio:

- **True unreliable datagrams**: Native support for unreliable delivery (no head-of-line blocking)
- **Lower latency**: 1-RTT connection setup (0-RTT after first connection)
- **Connection migration**: Survives IP address changes (WiFi ↔ Cellular)
- **Single port operation**: All communication over one UDP port
- **No framing overhead**: QUIC datagrams preserve packet boundaries

## Connection Type Detection

The server automatically detects the connection type:

```
┌─────────────────────┐
│ Incoming Connection │
└──────────┬──────────┘
           │
    ┌──────┴──────┐
    │             │
TCP 4464      UDP 4464
    │             │
    │             └──> QUIC packet → WebTransport
    │
    └──> Peek first bytes
          │
          ├──> "GET" → WebRTC (WebSocket)
          │
          └──> Binary (4 bytes) → UDP
```

### Detection Logic

**TCP port 4464** (UDP and WebRTC):
```cpp
QByteArray peekData = clientConnection->peek(512);

if (peekData.startsWith("GET")) {
    // WebRTC connection (WebSocket signaling)
    createWebRtcWorker(clientConnection, "webrtc");
} else {
    // Binary data - legacy UDP client
    readClientUdpPort(clientConnection, clientName);
}
```

**UDP port 4464** (WebTransport):
```cpp
// QUIC packets have distinctive header format
uint8_t first_byte = datagram[0];
bool is_long_header = (first_byte & 0x80) != 0;

if (is_long_header) {
    // QUIC Initial or Handshake packet
    handleQuicConnection(datagram, sender, senderPort);
}
```

## Audio Packet Format

All three connection types use the same audio packet format. See [NetworkProtocol.md](NetworkProtocol.md) for complete details on:

- Packet header structure (16 bytes)
- Audio payload format (planar/non-interleaved)
- Sample encoding (8/16/24/32-bit)
- Special field encodings

## Worker Pool Management

All connection types share the same worker pool:

- **Slot allocation**: First available slot from 0 to `gMaxThreads-1`
- **Audio ports**: All create identical JACK/RtAudio ports (`receive_N`, `send_N` where N = worker_id + 1)
- **Port assignment for UDP**: Each UDP client is assigned `mBasePort + worker_id` (typically 61002 + worker_id) for audio transport

## Building with Connection Type Support

### WebRTC Support

```bash
# Auto-detect libdatachannel (default)
meson setup build

# Explicitly enable (error if not available)
meson setup build -Dlibdatachannel=enabled

# Explicitly disable
meson setup build -Dlibdatachannel=disabled
```

When enabled, defines `WEBRTC_SUPPORT` macro.

### WebTransport Support

```bash
# Auto-detect msquic (default)
meson setup build

# Explicitly enable (error if not available)
meson setup build -Dmsquic=enabled

# Explicitly disable
meson setup build -Dmsquic=disabled
```

When enabled, defines `WEBTRANSPORT_SUPPORT` macro.

## Server Configuration

### Starting the Server

```bash
# Start hub server (binds to both TCP and UDP port 4464)
jacktrip -S

# Specify custom server port
jacktrip -S -p 4464

# Specify custom UDP base port for legacy UDP audio
jacktrip -S --udpbaseport 61002

# Configure ICE servers for WebRTC
jacktrip -S --iceservers "stun:stun.l.google.com:19302"

# Enable TLS for WebTransport (and optionally UDP auth)
jacktrip -S --certfile server.crt --keyfile server.key
```

### Firewall Configuration

```bash
# TCP for UDP and WebRTC signaling
sudo ufw allow 4464/tcp

# UDP for WebTransport and legacy audio
sudo ufw allow 4464/udp

# UDP port range for legacy UDP audio streams
sudo ufw allow 61002:62000/udp
```

### Port Summary

| Connection Type | Port | Protocol | Purpose |
|----------------|------|----------|---------|
| UDP | 4464 | TCP | Signaling handshake |
| UDP | 61002 + worker_id | UDP | Audio transport |
| WebRTC | 4464 | TCP | WebSocket signaling |
| WebRTC | ICE-negotiated | UDP | Audio transport (data channels) |
| WebTransport | 4464 | UDP | QUIC (signaling + audio) |

## Error Handling

### UDP Errors

- **Port already bound**: Worker slot exhausted or port conflict
- **Authentication failed**: Invalid credentials (if auth enabled)
- **Timeout**: Client doesn't send UDP packets after handshake

### WebRTC Errors

- **ICE failed**: No connectivity path found
- **DTLS handshake failed**: Certificate or crypto mismatch
- **Data channel failed**: SCTP association error

### WebTransport Errors

- **Handshake failed**: Invalid HTTP/3 CONNECT request
- **Certificate verification failed**: Invalid or untrusted TLS certificate
- **Session closed**: QUIC connection terminated

## References

### Source Files

- **UDP**: `src/UdpHubListener.cpp`, `src/UdpDataProtocol.cpp`
- **WebRTC**: `src/webrtc/WebRtcPeerConnection.cpp`, `src/webrtc/WebRtcDataProtocol.cpp`, `src/webrtc/WebRtcSignalingProtocol.cpp`
- **WebTransport**: `src/webtransport/WebTransportSession.cpp`, `src/webtransport/WebTransportDataProtocol.cpp`
- **Worker**: `src/JackTripWorker.cpp`

### External Documentation

- [NetworkProtocol.md](NetworkProtocol.md) - Detailed packet format and UDP protocol
- [WebRTC Specification](https://www.w3.org/TR/webrtc/)
- [WebTransport Specification](https://www.w3.org/TR/webtransport/)
- [RFC 9221 - QUIC Datagrams](https://www.rfc-editor.org/rfc/rfc9221.html)
- [libdatachannel](https://github.com/paullouisageneau/libdatachannel)
- [MsQuic](https://github.com/microsoft/msquic)
