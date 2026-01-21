# JackTrip Connection Handshake Protocol

## Overview

This document describes the connection handshake processes used by clients connecting to the JackTrip Hub Server (`UdpHubListener`). The server supports three different connection types:

1. **UDP Connections** - Traditional UDP-based audio transport (legacy)
2. **WebRTC Data Channel Connections** - Browser-compatible WebRTC with NAT traversal
3. **WebTransport Connections** - Modern HTTP/3-based low-latency transport

All three connection types share the same TCP signaling port (default: 4464) and use the same worker pool allocation mechanism.

---

## 1. UDP Connection Handshake (Legacy)

### Overview

The traditional UDP handshake is a simple TCP-based signaling exchange followed by UDP audio transport.

### Connection Flow

```
┌─────────────┐                           ┌────────────────────┐
│ UDP Client  │                           │   Hub Server       │
│             │                           │   (UdpHubListener) │
└─────┬───────┘                           └─────────┬──────────┘
      │                                             │
      │  1. TCP Connect (port 4464)                 │
      │─────────────────────────────────────────────▶
      │                                             │
      │  2. Send UDP port (32-bit LE integer)       │
      │─────────────────────────────────────────────▶
      │     Optional: Client name (64 bytes)        │
      │                                             │
      │                                             │  Allocate server UDP port
      │                                             │  (mBasePort + worker_id)
      │                                             │
      │  3. Receive server UDP port (32-bit LE)     │
      │◀─────────────────────────────────────────────
      │                                             │
      │  4. TCP Close                               │
      │─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
      │                                             │
      │  5. UDP Audio Exchange (bidirectional)      │
      │◀════════════════════════════════════════════▶
      │                                             │
```

### Protocol Details

#### Step 1: TCP Connection
- Client connects to server on TCP port 4464 (default `gDefaultPort`)
- Connection can be plain TCP or SSL/TLS (if authentication is enabled)

#### Step 2: Send Client UDP Port
The client sends:

```
Byte Offset | Length | Type   | Description
------------|--------|--------|---------------------------
0           | 4      | int32  | Client UDP port (little-endian)
4           | 64     | char[] | Client name (optional, null-terminated)
```

**Note:** For authentication, the client sends a special value (`Auth::OK = 65536`) instead of the actual port to initiate SSL handshake.

#### Step 3: Receive Server UDP Port
The server responds with:

```
Byte Offset | Length | Type   | Description
------------|--------|--------|---------------------------
0           | 4      | int32  | Server UDP port (little-endian)
```

If authentication is required or fails, the server may send error codes:
- `Auth::REQUIRED` - Authentication required but not provided
- `Auth::NOTREQUIRED` - Authentication not needed
- `Auth::BADCREDENTIALS` - Invalid credentials

#### Step 4: TCP Connection Close
The TCP connection is closed after the port exchange is complete.

#### Step 5: UDP Audio Transport
Audio packets are exchanged over UDP using the JackTrip audio packet format:

```cpp
struct DefaultHeaderStruct {
    uint64_t TimeStamp;                  // 8 bytes
    uint16_t SeqNumber;                  // 2 bytes
    uint16_t BufferSize;                 // 2 bytes (in samples)
    uint8_t  SamplingRate;               // 1 byte
    uint8_t  BitResolution;              // 1 byte
    uint8_t  NumIncomingChannelsFromNet; // 1 byte
    uint8_t  NumOutgoingChannelsToNet;   // 1 byte
};
// Total header: 16 bytes, followed by audio samples
```

### Authentication (Optional)

When authentication is enabled:

1. Client sends `Auth::OK` (65536) as port value
2. Server responds with `Auth::OK` to confirm
3. SSL handshake is performed
4. Client sends credentials:
   - UDP port (4 bytes)
   - Client name (64 bytes)
   - Username length (4 bytes)
   - Password length (4 bytes)
   - Username (variable + null terminator)
   - Password (variable + null terminator)
5. Server validates and responds with actual port or error code

### Key Implementation Details

- **Port allocation**: Server assigns `mBasePort + worker_id` where worker_id is 0-based
- **NAT handling**: Server waits for first UDP packet to determine actual client port
- **Duplicate detection**: Multiple connections from same address/port are detected and handled
- **Default ports**: Server TCP port defaults to 4464, UDP base port to 61002

---

## 2. WebRTC Data Channel Connection Handshake

### Overview

WebRTC connections use WebSocket-based signaling over the same TCP port as UDP clients, followed by WebRTC data channel setup with ICE/DTLS negotiation.

### Connection Flow

```
┌─────────────┐                           ┌────────────────────┐
│  WebRTC     │                           │   Hub Server       │
│  Client     │                           │   (UdpHubListener) │
└─────┬───────┘                           └─────────┬──────────┘
      │                                             │
      │  1. WebSocket Upgrade (HTTP/1.1)            │
      │     GET / HTTP/1.1                          │
      │     Upgrade: websocket                      │
      │─────────────────────────────────────────────▶
      │                                             │
      │  2. WebSocket Upgrade Response              │
      │     HTTP/1.1 101 Switching Protocols        │
      │◀─────────────────────────────────────────────
      │                                             │
      │  3. Send SDP Offer (JSON over WebSocket)    │
      │     { "type": "offer", "sdp": "..." }       │
      │─────────────────────────────────────────────▶
      │                                             │  Create RTCPeerConnection
      │                                             │  Allocate worker slot
      │                                             │  Generate SDP Answer
      │                                             │
      │  4. Receive SDP Answer (JSON)               │
      │     { "type": "answer", "sdp": "..." }      │
      │◀─────────────────────────────────────────────
      │                                             │
      │  5. Exchange ICE Candidates (multiple)      │
      │     { "type": "ice", "candidate": "..." }   │
      │◀────────────────────────────────────────────▶
      │                                             │
      │  6. ICE/DTLS Connection Establishment       │
      │─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
      │                                             │
      │  7. Data Channel Open                       │
      │◀────────────────────────────────────────────▶
      │                                             │
      │  8. Audio Exchange (via Data Channel)       │
      │◀════════════════════════════════════════════▶
      │                                             │
```

### Protocol Details

#### Step 1-2: WebSocket Upgrade

The client initiates a WebSocket connection by sending an HTTP upgrade request:

```http
GET / HTTP/1.1
Host: server.example.com:4464
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 13
```

The server detects the HTTP request (starts with "GET") and responds with:

```http
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
```

**Detection Logic** (in `UdpHubListener::readyRead`):
```cpp
// Check for HTTP-based upgrade requests (starts with "GET" or "CONNECT")
QByteArray peekData = clientConnection->peek(512);
if (peekData.startsWith("GET") || peekData.startsWith("CONNECT")) {
    // This is a WebRTC client
    createWebRtcWorker(clientConnection, "webrtc");
}
```

#### Step 3: SDP Offer

Client sends an SDP offer as JSON over WebSocket:

```json
{
    "type": "offer",
    "sdp": "v=0\r\no=- 4611731400430051336 2 IN IP4 127.0.0.1\r\n..."
}
```

The SDP offer includes:
- Media description for data channel
- ICE credentials (ufrag, pwd)
- DTLS fingerprint
- Codec information

#### Step 4: SDP Answer

Server creates a WebRTC peer connection and generates an answer:

```json
{
    "type": "answer",
    "sdp": "v=0\r\no=- 1234567890 2 IN IP4 0.0.0.0\r\n..."
}
```

#### Step 5: ICE Candidate Exchange

Both sides exchange ICE candidates as they're discovered:

```json
{
    "type": "ice",
    "candidate": "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host",
    "sdpMid": "data",
    "sdpMLineIndex": 0
}
```

ICE candidates can be:
- **Host candidates**: Local network interfaces
- **Server reflexive candidates**: Public IP from STUN server
- **Relay candidates**: TURN server allocations (if configured)

#### Step 6: ICE/DTLS Connection

- ICE performs connectivity checks between candidates
- DTLS handshake encrypts the data channel
- SCTP association is established for data channel

#### Step 7-8: Data Channel Open & Audio

Once the data channel is open:
- Worker is configured with `WebRtcDataProtocol`
- Audio packets use the same format as UDP
- Packets are sent unreliably (no retransmissions)

### Signaling Message Format

All signaling messages use JSON with the following structure:

```json
{
    "type": "offer" | "answer" | "ice" | "hangup" | "error",
    "sdp": "string (for offer/answer)",
    "candidate": "string (for ice)",
    "sdpMid": "string (for ice)",
    "sdpMLineIndex": number (for ice),
    "errorMessage": "string (for error)"
}
```

### Data Channel Configuration

The data channel is configured for low-latency audio:

```cpp
rtc::DataChannelInit config;
config.ordered = false;       // Don't wait for in-order delivery
config.maxRetransmits = 0;    // No retransmissions (like UDP)
config.negotiated = false;    // Standard negotiation
```

### ICE Servers

The server can be configured with STUN/TURN servers:

```cpp
// Example ICE server configuration
mIceServers = {
    "stun:stun.l.google.com:19302",
    "turn:turn.example.com:3478?transport=udp"
};
```

### Key Implementation Details

- **Library**: Uses `libdatachannel` for WebRTC implementation
- **Worker creation**: `createWebRtcWorker()` allocates slot and handles signaling
- **Port allocation**: Same pool as UDP (conceptual port for slot ID)
- **Protocol class**: `WebRtcDataProtocol` implements `DataProtocol` interface
- **Connection management**: `WebRtcPeerConnection` wraps peer connection and data channel

---

## 3. WebTransport Connection Handshake

### Overview

WebTransport provides a modern, low-latency transport using HTTP/3 over QUIC with native unreliable datagrams. Unlike WebRTC, it requires no ICE negotiation and provides a simpler connection model with built-in NAT traversal through QUIC.

**Important:** WebTransport uses **UDP** for the QUIC connection, not TCP. The server must bind to UDP port 4464 (in addition to TCP 4464) to accept WebTransport connections.

> **Implementation Note:** This section describes the proper QUIC-based implementation of WebTransport. Earlier implementations may have used a WebSocket fallback approach (TCP-based), but this design exclusively uses HTTP/3 with QUIC unreliable datagrams as intended by the WebTransport specification.

### Connection Flow

```
┌─────────────┐                           ┌────────────────────┐
│ WebTransport│                           │   Hub Server       │
│   Client    │                           │   (UdpHubListener) │
└─────┬───────┘                           └─────────┬──────────┘
      │                                             │
      │  1. QUIC Connection Initiation (UDP 4464)   │
      │     Initial QUIC packet with ClientHello    │
      │─────────────────────────────────────────────▶
      │                                             │
      │  2. QUIC Handshake (TLS 1.3)                │
      │     ServerHello, encryption keys            │
      │◀────────────────────────────────────────────▶
      │                                             │
      │  3. HTTP/3 CONNECT Request                  │
      │     :method = CONNECT                       │
      │     :protocol = webtransport                │
      │     :path = /webtransport                   │
      │─────────────────────────────────────────────▶
      │                                             │
      │                                             │  Detect WebTransport
      │                                             │  Allocate worker slot
      │                                             │  Create WebTransportSession
      │                                             │
      │  4. HTTP/3 Response (200 OK)                │
      │◀─────────────────────────────────────────────
      │                                             │
      │  5. Optional: Client Info Datagram          │
      │     Client name, version, config            │
      │─────────────────────────────────────────────▶
      │                                             │
      │  6. Session Established                     │
      │◀─────────────────────────────────────────────
      │                                             │
      │  7. Unreliable Datagram Audio Exchange      │
      │     (QUIC datagrams, not streams)           │
      │◀════════════════════════════════════════════▶
      │                                             │
```

### Protocol Details

#### Step 1: QUIC Connection Initiation

The client initiates a QUIC connection to UDP port 4464:

- **Transport:** QUIC over UDP (not TCP)
- **TLS:** TLS 1.3 is mandatory (built into QUIC)
- **Initial packet:** Contains ClientHello and QUIC connection parameters
- **SNI:** Server Name Indication for certificate validation

```
QUIC Initial Packet Structure:
┌─────────────────┬──────────────────┬─────────────────┐
│ QUIC Header     │ TLS ClientHello  │ Connection IDs  │
│ (long format)   │ (encrypted)      │                 │
└─────────────────┴──────────────────┴─────────────────┘
```

**Server Detection Logic** (in `UdpHubListener::receivedDatagramOnUdpSocket`):
```cpp
// Detect QUIC packet by header flags (first byte)
QByteArray datagram = udpSocket->readDatagram();
if (isQuicPacket(datagram)) {
    // Handle as potential WebTransport connection
    handleQuicConnection(datagram, sender, senderPort);
}
```

#### Step 2: QUIC/TLS Handshake

QUIC and TLS handshakes happen simultaneously:

1. **Client → Server:** Initial packet with ClientHello
2. **Server → Client:** ServerHello, certificate, encryption keys
3. **Client → Server:** Certificate verification, final handshake data
4. **Result:** Encrypted QUIC connection established

**Key Features:**
- **0-RTT support:** After first connection, subsequent connections can send data immediately
- **Connection migration:** Client can change IP/port without reconnecting
- **Built-in encryption:** All data is encrypted (no plaintext option)

#### Step 3: HTTP/3 CONNECT Request

Once QUIC is established, client sends an HTTP/3 CONNECT request:

```
HTTP/3 CONNECT Request (QPACK encoded):
:method = CONNECT
:protocol = webtransport
:scheme = https
:path = /webtransport?name=MyClient
:authority = server.example.com:4464
origin = https://example.com
```

**Path Parameters:**
- `/webtransport` - Required path for WebTransport sessions
- `?name=ClientName` - Optional client name query parameter
- `?version=1` - Optional protocol version

**Detection in Server:**
```cpp
// HTTP/3 request arrives on QUIC stream 0
if (request.method == "CONNECT" && request.protocol == "webtransport") {
    createWebTransportWorker(quicConnection, clientName);
}
```

#### Step 4: HTTP/3 Response

Server accepts the WebTransport session:

```
HTTP/3 Response:
:status = 200
sec-webtransport-http3-draft = draft02
```

**Status Codes:**
- `200 OK` - Session accepted
- `404 Not Found` - WebTransport not supported
- `429 Too Many Requests` - No available worker slots
- `503 Service Unavailable` - Server shutting down

#### Step 5: Client Info Datagram (Optional)

Client can send initial configuration via first datagram:

```json
{
    "type": "client_info",
    "clientName": "MyClient",
    "version": 1,
    "audioChannels": 2,
    "sampleRate": 48000
}
```

**Note:** This is optional. The server can extract client name from the CONNECT path.

#### Step 6: Session Established

Server confirms session is ready and allocates resources:

```cpp
// Server-side
emit sessionEstablished();

// Worker configuration
worker->setJackTrip(id, peerAddress, conceptualPort, 0, 
                    m_connectDefaultAudioPorts);
worker->createWebTransportSession(quicConnection);
worker->start();
```

#### Step 7: QUIC Datagram Audio Exchange

Audio packets are sent as **QUIC DATAGRAM frames** (RFC 9221):

```
QUIC Datagram Frame Format:
┌──────────┬──────────────────────┬────────────────────┐
│ Type     │ JackTrip Header      │   Audio Samples    │
│ (1 byte) │ (16 bytes)           │   (variable)       │
└──────────┴──────────────────────┴────────────────────┘
```

**QUIC Datagram Properties:**
- **Unreliable:** No retransmissions (like UDP)
- **Unordered:** Can arrive out of sequence
- **Size limit:** Typically 1200 bytes (path MTU)
- **Encapsulated:** Wrapped in QUIC encryption
- **Flow controlled:** Still respects QUIC connection limits

**Sending Audio:**
```cpp
// Client or server sends audio via QUIC datagram
int WebTransportDataProtocol::sendPacket(const char* buf, const size_t n)
{
    if (!mQuicConnection || !mQuicConnection->isConnected()) {
        return -1;
    }
    
    // Send as QUIC datagram (unreliable, no retransmissions)
    return mQuicConnection->sendDatagram(buf, n);
}
```

**Receiving Audio:**
```cpp
// Datagram arrives via callback
void WebTransportSession::onDatagramReceived(const uint8_t* data, size_t len)
{
    // No framing needed - QUIC provides datagram boundaries
    std::vector<std::byte> packet(len);
    std::memcpy(packet.data(), data, len);
    
    // Process directly as JackTrip audio packet
    emit datagramReceived(packet);
}
```

### Key Implementation Details

#### Server Setup
```cpp
// UdpHubListener must bind to BOTH TCP and UDP on port 4464
void UdpHubListener::start()
{
    // Existing TCP listener for UDP and WebRTC clients
    mTcpServer.listen(QHostAddress::Any, mServerPort);
    
    // NEW: UDP listener for HTTP/3/WebTransport clients
    mQuicSocket = new QUdpSocket(this);
    mQuicSocket->bind(QHostAddress::Any, mServerPort, QUdpSocket::ShareAddress);
    connect(mQuicSocket, &QUdpSocket::readyRead, this,
            &UdpHubListener::handleQuicDatagram);
}
```

#### QUIC Connection Handling
```cpp
void UdpHubListener::handleQuicDatagram()
{
    while (mQuicSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;
        
        datagram.resize(mQuicSocket->pendingDatagramSize());
        mQuicSocket->readDatagram(datagram.data(), datagram.size(),
                                  &sender, &senderPort);
        
        // Check if this is a QUIC packet (examine header flags)
        if (isQuicPacket(datagram)) {
            handleQuicConnection(datagram, sender, senderPort);
        }
    }
}
```

#### Core Classes
- **Session class**: `WebTransportSession` manages the QUIC connection and HTTP/3 layer
- **Protocol class**: `WebTransportDataProtocol` implements `DataProtocol` interface
- **QUIC library**: Requires external library (e.g., `quiche`, `lsquic`, or `msquic`)
- **Datagram API**: Native QUIC datagrams (RFC 9221) for audio transport
- **Worker creation**: `createWebTransportWorker()` allocates slot from same pool as UDP/WebRTC

#### Port Allocation
- **UDP 4464**: Accepts QUIC connections for WebTransport
- **TCP 4464**: Still accepts legacy UDP and WebRTC clients
- **No separate ports**: Audio travels over the same QUIC connection
- **Worker slots**: Same port assignment scheme (`mBasePort + id`) for consistency

### WebTransport vs WebRTC

| Feature | WebTransport | WebRTC |
|---------|-------------|--------|
| **NAT Traversal** | Good (QUIC built-in) | Excellent (ICE/STUN/TURN) |
| **Browser Support** | Modern browsers only (Chrome 97+, Edge 97+) | Wide support |
| **Setup Complexity** | Simpler (no ICE negotiation) | More complex (SDP/ICE) |
| **Connection Time** | Faster (0-RTT capable) | Slower (ICE gathering) |
| **Transport** | QUIC datagrams over UDP | SCTP data channel over DTLS/UDP |
| **Encryption** | TLS 1.3 (mandatory) | DTLS (mandatory) |
| **Firewall Traversal** | UDP 4464 must be open | Uses ICE to find path |

---

## QUIC Library Requirements for WebTransport

WebTransport requires a QUIC library to handle HTTP/3 connections. Several options are available:

### Option 1: quiche (Cloudflare)

**Recommended** - Well-maintained, good performance, pure Rust with C bindings

```bash
# Add as subproject or system dependency
# Provides both QUIC and HTTP/3 support
```

**Features:**
- RFC 9000 compliant QUIC
- RFC 9114 HTTP/3 support
- RFC 9221 QUIC datagrams
- TLS 1.3 via BoringSSL
- C API via FFI

**Integration:**
```cpp
#include <quiche.h>

// Initialize QUIC connection
quiche_config* config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
quiche_config_enable_dgram(config, true, 1200, 1200);  // Enable datagrams

quiche_conn* conn = quiche_connect(server_name, scid, scid_len,
                                   local_addr, local_addr_len,
                                   peer_addr, peer_addr_len,
                                   config);
```

### Option 2: lsquic (LiteSpeed)

High-performance QUIC library from LiteSpeed Technologies

**Features:**
- Production-tested at scale
- HTTP/3 and QPACK support
- QUIC datagram support
- C API

### Option 3: msquic (Microsoft)

Microsoft's cross-platform QUIC library

**Features:**
- Cross-platform (Windows, Linux, macOS)
- MsQuic API
- Good Windows integration
- Used in Windows kernel

### Build Configuration

```bash
# Enable WebTransport with QUIC support
meson setup build -Dwebtransport=enabled -Dquic_library=quiche

# Specify custom QUIC library path
meson setup build -Dquic_library_path=/usr/local/lib/libquiche.so
```

### TLS Certificate Requirements

WebTransport requires TLS 1.3 certificates. Unlike legacy UDP, encryption is **mandatory** and cannot be disabled.

#### Generate Self-Signed Certificate (Development)

```bash
# Generate private key
openssl genpkey -algorithm RSA -out webtransport.key -pkeyopt rsa_keygen_bits:2048

# Generate self-signed certificate
openssl req -new -x509 -key webtransport.key -out webtransport.crt -days 365 \
  -subj "/CN=jacktrip.example.com"

# Start server with certificate (enables both auth and WebTransport)
jacktrip -S --certfile webtransport.crt \
         --keyfile webtransport.key
```

#### Production Certificate (Let's Encrypt)

```bash
# Use certbot to obtain valid certificate
sudo certbot certonly --standalone -d jacktrip.example.com

# Certificates will be in:
# /etc/letsencrypt/live/jacktrip.example.com/fullchain.pem
# /etc/letsencrypt/live/jacktrip.example.com/privkey.pem

jacktrip -S \
  --certfile /etc/letsencrypt/live/jacktrip.example.com/fullchain.pem \
  --keyfile /etc/letsencrypt/live/jacktrip.example.com/privkey.pem
```

#### Browser Certificate Verification

For WebTransport from browsers:
1. **Valid certificate** - Must be signed by a trusted CA
2. **Correct hostname** - Must match the domain in the connection URL
3. **Not expired** - Certificate must be current

Self-signed certificates will be **rejected** by browsers unless explicitly trusted.

---

## Connection Type Detection

The server automatically detects the connection type by examining the transport layer and initial data:

### UDP Port 4464: QUIC/HTTP/3 Detection

```cpp
// UdpHubListener::handleQuicDatagram()
// Detect QUIC packets on UDP port 4464

bool UdpHubListener::isQuicPacket(const QByteArray& datagram)
{
    if (datagram.size() < 1) {
        return false;
    }
    
    // QUIC packets have distinctive header format
    uint8_t first_byte = static_cast<uint8_t>(datagram[0]);
    
    // Long header: first bit is 1
    // Short header: first bit is 0
    bool is_long_header = (first_byte & 0x80) != 0;
    
    if (is_long_header) {
        // Check for QUIC version field (bytes 1-4)
        // QUIC v1 = 0x00000001
        // QUIC v2 = 0x6b3343cf
        if (datagram.size() < 5) {
            return false;
        }
        
        // This is likely a QUIC Initial or Handshake packet
        return true;
    } else {
        // Short header packets (after handshake)
        // Need to track existing QUIC connections
        return isExistingQuicConnection(sender, senderPort);
    }
}

void UdpHubListener::handleQuicDatagram()
{
    while (mQuicSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress sender;
        quint16 senderPort;
        
        datagram.resize(mQuicSocket->pendingDatagramSize());
        mQuicSocket->readDatagram(datagram.data(), datagram.size(),
                                  &sender, &senderPort);
        
        if (isQuicPacket(datagram)) {
            // This is a WebTransport (HTTP/3) client
            handleQuicConnection(datagram, sender, senderPort);
        }
    }
}
```

### TCP Port 4464: UDP/WebRTC Detection

```cpp
// UdpHubListener::readyRead()
// Detect connection type on TCP port 4464

void UdpHubListener::readyRead(QSslSocket* clientConnection)
{
    // Peek at initial data without consuming it
    QByteArray peekData = clientConnection->peek(512);

    if (peekData.startsWith("GET") || peekData.startsWith("CONNECT")) {
        // HTTP-based request - must be WebRTC
        // (WebTransport uses UDP, not TCP)
        
        // WebRTC connection (WebSocket for signaling)
        createWebRtcWorker(clientConnection, "webrtc");
    } else {
        // Binary data - legacy UDP client
        // Read 32-bit UDP port and proceed with UDP handshake
        readClientUdpPort(clientConnection, clientName);
    }
}
```

### Summary of Detection Logic

```
┌─────────────────────────────────────────────────┐
│         Incoming Connection                      │
└────────────────┬────────────────────────────────┘
                 │
      ┌──────────┴──────────┐
      │                     │
  TCP 4464              UDP 4464
      │                     │
      │                     └──> QUIC packet?
      │                           │
      │                           └──> WebTransport
      │
      └──> Peek first bytes
            │
            ├──> "GET" / "CONNECT" → WebRTC (WebSocket)
            │
            └──> Binary (4 bytes) → UDP legacy
```

---

## Worker Pool Management

All three connection types share the same worker pool:

### Port/Slot Allocation

```cpp
int createWorker(QString& clientName)
{
    // Find first empty slot (0 to gMaxThreads-1)
    int id = -1;
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) == nullptr) {
            id = i;
            break;
        }
    }
    
    if (id < 0) {
        return -1;  // No available slots
    }
    
    // Create worker with assigned slot
    JackTripWorker* worker = new JackTripWorker(...);
    mJTWorkers->replace(id, worker);
    
    return id;
}
```

### Port Assignment

- **UDP**: `mBasePort + worker_id` (actual UDP port)
- **WebRTC**: `mBasePort + worker_id` (conceptual, for consistency)
- **WebTransport**: `mBasePort + worker_id` (conceptual, for consistency)

### Audio Port Creation

All connection types create identical JACK/RtAudio ports:
- `receive_N` - Audio input from network
- `send_N` - Audio output to network

Where N = worker_id + 1 (1-indexed for user display)

---

## Packet Format (All Transports)

All three transport types use the same audio packet format:

### Header Structure

```cpp
struct DefaultHeaderStruct {
    uint64_t TimeStamp;                  // 8 bytes - Packet timestamp
    uint16_t SeqNumber;                  // 2 bytes - Sequence number
    uint16_t BufferSize;                 // 2 bytes - Buffer size in samples
    uint8_t  SamplingRate;               // 1 byte - Sample rate code
    uint8_t  BitResolution;              // 1 byte - Bit depth code
    uint8_t  NumIncomingChannelsFromNet; // 1 byte - Input channels
    uint8_t  NumOutgoingChannelsToNet;   // 1 byte - Output channels (special encoding)
};
// Total: 16 bytes
```

### Special Encoding for NumOutgoingChannelsToNet

- **Value = 0**: Symmetric (outgoing = incoming)
- **Value = 1-254**: Explicit channel count
- **Value = 255**: Zero input channels (receive-only)

### Audio Data

Following the header:
- Audio samples in interleaved format
- Sample size determined by `BitResolution`
- Total samples = `BufferSize × NumOutgoingChannelsToNet`

### Example Packet

```
Offset | Size | Field           | Value
-------|------|-----------------|------------------
0      | 8    | TimeStamp       | 0x0000012345ABCDEF
8      | 2    | SeqNumber       | 0x0042
10     | 2    | BufferSize      | 0x0080 (128 samples)
12     | 1    | SamplingRate    | 0x02 (48kHz)
13     | 1    | BitResolution   | 0x02 (16-bit)
14     | 1    | NumIncoming     | 0x02 (2 channels)
15     | 1    | NumOutgoing     | 0x00 (symmetric)
16     | 512  | Audio data      | (128 samples × 2 ch × 2 bytes)
```

---

## Comparison Summary

| Feature | UDP | WebRTC | WebTransport |
|---------|-----|--------|--------------|
| **NAT Traversal** | No | Yes (ICE/STUN/TURN) | Yes (QUIC) |
| **Browser Support** | No | Yes | Modern only (Chrome 97+) |
| **Encryption** | Optional | Mandatory (DTLS) | Mandatory (TLS 1.3) |
| **Setup Time** | Fastest | Medium | Fast (0-RTT after first) |
| **Latency** | Lowest | Low | Low |
| **Reliability** | Unreliable | Unreliable (configurable) | Unreliable (QUIC datagrams) |
| **Signaling Port** | TCP 4464 | TCP 4464 (WebSocket) | UDP 4464 (HTTP/3) |
| **Transport Port** | UDP dynamic | UDP (ICE-selected) | Same as signaling (UDP 4464) |
| **Port Range** | 61002+ | N/A (ICE) | N/A (single port) |
| **Authentication** | Optional | N/A (via signaling) | TLS certificates |
| **Library** | Qt Network | libdatachannel | QUIC library (quiche/lsquic/msquic) |

---

## Error Handling

### UDP Connection Errors

- **Port already bound**: Worker slot exhausted or port conflict
- **Authentication failed**: Invalid credentials
- **Timeout**: Client doesn't send UDP packets after handshake

### WebRTC Connection Errors

- **ICE failed**: No connectivity path found
- **DTLS handshake failed**: Certificate or crypto mismatch
- **Data channel failed**: SCTP association error

### WebTransport Connection Errors

- **Handshake failed**: Invalid HTTP upgrade request
- **Session closed**: Transport layer disconnected
- **Frame decode error**: Malformed frame data

---

## Configuration

### Server Configuration

```bash
# Start hub server with default settings
# Binds to BOTH TCP and UDP port 4464
jacktrip -S

# Specify server port (both TCP and UDP)
jacktrip -S -p 4464

# Specify UDP base port for legacy UDP audio streams
jacktrip -S --udpbaseport 61002

# Enable authentication (for legacy UDP clients)
jacktrip -S --auth \
  --certfile server.crt \
  --keyfile server.key \
  --credsfile credentials.txt

# TLS certificate for WebTransport (mandatory)
# Uses same certificate as authentication
jacktrip -S --certfile server.crt \
         --keyfile server.key

# Configure ICE servers for WebRTC
jacktrip -S --iceservers "stun:stun.l.google.com:19302"
```

### Port Requirements

| Connection Type | Port | Protocol | Purpose |
|----------------|------|----------|---------|
| **UDP Legacy** | 4464 | TCP | Signaling handshake |
| **UDP Legacy** | 61002+ | UDP | Audio transport |
| **WebRTC** | 4464 | TCP | WebSocket signaling |
| **WebRTC** | Dynamic | UDP | ICE-negotiated data channel |
| **WebTransport** | 4464 | UDP | QUIC connection (signaling + audio) |

**Important:** The server must bind to **both TCP and UDP** on port 4464 to support all connection types.

### Firewall Configuration

```bash
# Allow TCP for legacy UDP and WebRTC signaling
sudo ufw allow 4464/tcp

# Allow UDP for WebTransport (HTTP/3) and legacy audio
sudo ufw allow 4464/udp

# Allow UDP port range for legacy UDP audio streams
sudo ufw allow 61002:62000/udp
```

### Build Options

```bash
# Enable all transports (default)
meson setup build

# Disable WebRTC
meson setup build -Dwebrtc=disabled

# Disable WebTransport
meson setup build -Dnowebtransport=true

# Specify QUIC library for WebTransport
meson setup build -Dquic_library=quiche
```

---

## References

### Source Files

- **UDP**: `src/UdpHubListener.cpp`, `src/UdpDataProtocol.cpp`
- **WebRTC**: `src/webrtc/WebRtcPeerConnection.cpp`, `src/webrtc/WebRtcDataProtocol.cpp`, `src/webrtc/WebRtcSignalingProtocol.cpp`
- **WebTransport**: `src/webtransport/WebTransportSession.cpp`, `src/webtransport/WebTransportDataProtocol.cpp`
- **Worker**: `src/JackTripWorker.cpp`

### External Documentation

- [WebRTC Specification](https://www.w3.org/TR/webrtc/)
- [WebTransport Specification](https://www.w3.org/TR/webtransport/)
- [libdatachannel Documentation](https://github.com/paullouisageneau/libdatachannel)
- [JackTrip WebRTC Design Doc](WebRTC_DataChannel_Design.md)

---

## Why QUIC for WebTransport?

QUIC (Quick UDP Internet Connections) provides several advantages over TCP for real-time audio:

### 1. True Unreliable Datagrams

Unlike TCP (which WebSocket uses), QUIC supports native unreliable datagrams:

```cpp
// No framing overhead - send JackTrip packet directly
connection->sendDatagram(audioPacket, packetSize);

// QUIC handles encryption, not the application
// Datagram boundaries are preserved (unlike TCP streams)
```

### 2. Lower Latency

| Metric | TCP/WebSocket | QUIC |
|--------|---------------|------|
| **Head-of-line blocking** | Yes (lost packets block all data) | No (per-stream) |
| **Connection setup** | 3 RTTs (TCP + TLS) | 1 RTT (0-RTT after first) |
| **Retransmission** | Entire stream affected | Only affected streams |
| **Datagram mode** | Not supported | Native support |

### 3. Better NAT Traversal

```
TCP/WebSocket:
- Requires public IP or reverse proxy
- Firewall must allow TCP connections
- No built-in connection migration

QUIC:
- Connection IDs allow NAT rebinding
- Can migrate between networks (WiFi ↔ Cellular)
- Survives IP address changes
```

### 4. Single Port Operation

```
WebSocket (Old Way):
1. TCP 4464 for signaling
2. Establish WebSocket
3. Send framed audio over TCP (reliable, higher latency)

QUIC (New Way):
1. UDP 4464 for QUIC handshake
2. HTTP/3 CONNECT establishes session
3. Send audio as QUIC datagrams on same connection (unreliable, low latency)
```

### 5. No Framing Overhead

**WebSocket Approach:**
```
┌─────┬───────┬──────────┬────────────┐
│ WS  │ Frame │ JackTrip │   Audio    │
│Frame│ Len   │ Header   │  Samples   │
│ 2B  │ 4B    │ 16B      │  ~512B     │
└─────┴───────┴──────────┴────────────┘
Total overhead: 22 bytes per packet
```

**QUIC Datagram Approach:**
```
┌──────────┬────────────┐
│ JackTrip │   Audio    │
│ Header   │  Samples   │
│ 16B      │  ~512B     │
└──────────┴────────────┘
Total overhead: 16 bytes per packet
(QUIC encryption is at UDP layer, transparent to app)
```

---

## Appendix: Message Sequence Examples

### Complete UDP Handshake (Hex Dump)

```
Client → Server (TCP)
  00 00 7B 3A              # Port 15104 (little-endian)
  4D 79 43 6C 69 65 6E 74  # "MyClient" (+ padding to 64 bytes)
  00 00 00 ...

Server → Client (TCP)
  62 EE 00 00              # Port 61026 (little-endian)
```

### Complete WebRTC Signaling (JSON)

```json
// Client → Server
{
    "type": "offer",
    "sdp": "v=0\r\no=- 123456 2 IN IP4 127.0.0.1\r\ns=-\r\n..."
}

// Server → Client
{
    "type": "answer",
    "sdp": "v=0\r\no=- 789012 2 IN IP4 0.0.0.0\r\ns=-\r\n..."
}

// Bidirectional (multiple)
{
    "type": "ice",
    "candidate": "candidate:1 1 UDP 2130706431 192.168.1.100 54321 typ host",
    "sdpMid": "data",
    "sdpMLineIndex": 0
}
```

### WebTransport (QUIC) Packet Example

```
QUIC Initial Packet (Client → Server):
  C0                       # Header: Long form, Initial type
  00 00 00 01              # Version: QUIC v1
  08                       # DCID length: 8 bytes
  A1 B2 C3 D4 E5 F6 G7 H8  # Destination Connection ID
  08                       # SCID length: 8 bytes
  12 34 56 78 9A BC DE F0  # Source Connection ID
  00                       # Token length: 0
  41 XX                    # Payload length (variable-length)
  <encrypted TLS ClientHello>

HTTP/3 CONNECT Request (over QUIC stream 0):
  # QPACK encoded headers
  00 00                    # Required Insert Count = 0, Base = 0
  51 84 49                 # :method = CONNECT (indexed)
  7F 0A                    # :protocol = webtransport (literal)
  77 65 62 74 72 61 6E 73 70 6F 72 74
  52 07                    # :path = /webtransport (literal)
  2F 77 65 62 74 72 61 6E 73 70 6F 72 74

HTTP/3 Response (Server → Client):
  00 00                    # QPACK header
  61 C8                    # :status = 200

QUIC Datagram with Audio (bidirectional):
  # QUIC Datagram Frame
  30                       # Frame type: DATAGRAM (0x30)
  82 02                    # Length: 258 bytes (var-length)
  
  # JackTrip audio packet (no additional framing!)
  00 01 02 03 04 05 06 07  # Timestamp (8 bytes)
  00 42                    # Sequence number: 66
  00 80                    # Buffer size: 128 samples
  02                       # Sample rate: 48kHz
  02                       # Bit depth: 16-bit
  02                       # Input channels: 2
  00                       # Output channels: symmetric
  <512 bytes of audio>     # 128 samples × 2 ch × 2 bytes
```

**Key Observation:** QUIC datagrams carry the JackTrip packet directly with no additional framing at the application layer. The datagram boundaries are preserved by QUIC itself.

---

## Design Principles Summary

### WebTransport Architecture Decision

**Critical Design Requirement:** WebTransport MUST use HTTP/3 with QUIC unreliable datagrams over UDP, NOT WebSocket/TCP.

#### Why This Matters

1. **Latency**: TCP's reliable delivery and head-of-line blocking add latency that's unacceptable for real-time audio
2. **Protocol Intent**: WebTransport was designed specifically to provide unreliable datagram transport
3. **NAT Traversal**: QUIC includes connection migration and NAT rebinding
4. **Single Port**: QUIC multiplexes signaling and audio on one UDP port
5. **Encryption**: TLS 1.3 is built into QUIC, no additional layer needed

#### Implementation Checklist

- [ ] `UdpHubListener` binds to UDP port 4464 (in addition to TCP)
- [ ] QUIC library integrated (quiche, lsquic, or msquic)
- [ ] `isQuicPacket()` detects QUIC Initial packets
- [ ] `handleQuicConnection()` processes HTTP/3 CONNECT requests
- [ ] `WebTransportSession` wraps QUIC connection (not WebSocket)
- [ ] `sendDatagram()` uses QUIC datagram frames (RFC 9221)
- [ ] `onDatagramReceived()` receives QUIC datagrams directly
- [ ] No application-level framing over QUIC datagrams
- [ ] TLS 1.3 certificates configured for QUIC
- [ ] Audio packets sent as unreliable QUIC datagrams

#### What NOT to Do

❌ **Do NOT** use WebSocket as a fallback for WebTransport  
❌ **Do NOT** send audio over TCP  
❌ **Do NOT** add custom framing over QUIC datagrams  
❌ **Do NOT** implement reliability/retransmission in the application  
❌ **Do NOT** accept HTTP/1.1 Upgrade requests for WebTransport  

#### Correct Flow

```
Client                          Server
  │                               │
  │  QUIC Initial (UDP 4464)      │
  ├──────────────────────────────>│  (QUIC handshake)
  │                               │
  │  HTTP/3 CONNECT               │
  ├──────────────────────────────>│  (establish session)
  │                               │
  │  QUIC Datagram (audio)        │
  ├<─────────────────────────────>│  (unreliable, low latency)
  │                               │
```

---

**Document Version**: 1.1  
**Last Updated**: January 16, 2026  
**Author**: JackTrip Development Team
