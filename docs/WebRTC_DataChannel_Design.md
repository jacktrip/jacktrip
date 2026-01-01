# WebRTC Data Channel Support for JackTrip Hub Server

## Executive Summary

This document outlines the architecture for adding WebRTC data channel support to JackTrip's hub server, enabling it to handle both traditional UDP and WebRTC data channel clients on the same infrastructure.

## Goals

1. **Reuse existing TCP signaling port** for both UDP and WebRTC clients
2. **Reuse the same port allocation pool** for both connection types
3. **Create input/output audio ports per peer** identical to UDP behavior
4. **Maintain compatibility** with existing UDP clients
5. **Enable NAT traversal** via WebRTC's ICE framework

## Current Architecture Overview

### Connection Flow (UDP)

```
┌─────────────┐                           ┌────────────────────┐
│   Client    │                           │   Hub Server       │
│             │                           │   (UdpHubListener) │
└─────┬───────┘                           └─────────┬──────────┘
      │                                             │
      │  1. TCP Connect (port 4464)                 │
      │─────────────────────────────────────────────▶
      │                                             │
      │  2. Send client UDP port                    │
      │─────────────────────────────────────────────▶
      │                                             │  Allocate server UDP port
      │  3. Receive server UDP port                 │  (mBasePort + id)
      │◀─────────────────────────────────────────────
      │                                             │
      │  4. TCP Close                               │
      │─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
      │                                             │
      │  5. UDP Audio Exchange (bidirectional)      │
      │◀════════════════════════════════════════════▶
      │                                             │
```

### Key Classes

| Class | Responsibility |
|-------|---------------|
| `UdpHubListener` | TCP server for signaling, manages worker pool |
| `JackTripWorker` | Per-client connection handler |
| `JackTrip` | Main coordinator (mediator pattern) |
| `DataProtocol` | Abstract base for data transport |
| `UdpDataProtocol` | UDP implementation |

### Packet Format

```cpp
struct DefaultHeaderStruct {
    uint64_t TimeStamp;                  // 8 bytes
    uint16_t SeqNumber;                  // 2 bytes
    uint16_t BufferSize;                 // 2 bytes (in samples)
    uint8_t  SamplingRate;               // 1 byte
    uint8_t  BitResolution;              // 1 byte
    uint8_t  NumIncomingChannelsFromNet; // 1 byte
    uint8_t  NumOutgoingChannelsToNet;   // 1 byte (see special encoding below)
};
// Total header: 16 bytes, followed by audio samples
```

#### Special Encoding for `NumOutgoingChannelsToNet`

The `NumOutgoingChannelsToNet` field uses a space-optimized encoding:

- **Value = 0**: Outgoing channels equals incoming channels (symmetric configuration)
  - This is the most common case and avoids redundant data
  - Example: If `NumIncomingChannelsFromNet = 2`, then outgoing is also 2
- **Value = 1-254**: Explicit outgoing channel count (asymmetric configuration)
  - Used when sender has different input/output channel counts
- **Value = 255 (0xFF)**: Special case indicating zero input channels
  - Sender is receive-only (no outgoing audio)

This encoding is implemented in `PacketHeader.cpp::fillHeaderCommonFromAudio()`:

```cpp
if (getNumInputChannels() == getNumOutputChannels()) {
    NumOutgoingChannelsToNet = 0;  // Symmetric case (bandwidth optimization)
} else if (getNumInputChannels() == 0) {
    NumOutgoingChannelsToNet = 255;  // No input channels
} else {
    NumOutgoingChannelsToNet = getNumInputChannels();  // Explicit count
}
```

---

## Proposed WebRTC Architecture

### Connection Flow (WebRTC)

```
┌─────────────┐                           ┌────────────────────┐
│  WebRTC     │                           │   Hub Server       │
│  Client     │                           │   (UdpHubListener) │
└─────┬───────┘                           └─────────┬──────────┘
      │                                             │
      │  1. TCP/WS Connect (port 4464)              │
      │─────────────────────────────────────────────▶
      │                                             │
      │  2. Send protocol=webrtc + SDP Offer        │
      │─────────────────────────────────────────────▶
      │                                             │  Create RTCPeerConnection
      │  3. Receive SDP Answer                      │  Allocate worker slot
      │◀─────────────────────────────────────────────
      │                                             │
      │  4. Exchange ICE Candidates (multiple)      │
      │◀────────────────────────────────────────────▶
      │                                             │
      │  5. Data Channel Connected                  │
      │─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
      │                                             │
      │  6. Audio Exchange (via Data Channel)       │
      │◀════════════════════════════════════════════▶
      │                                             │
```

### New Classes

#### 1. `WebRtcDataProtocol`

Extends `DataProtocol` to use WebRTC data channels instead of UDP sockets.

```cpp
class WebRtcDataProtocol : public DataProtocol
{
    Q_OBJECT;

public:
    WebRtcDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                       rtc::scoped_refptr<webrtc::DataChannelInterface> dataChannel);
    
    virtual ~WebRtcDataProtocol();
    
    virtual void run() override;
    virtual void stop() override;
    
    // Required virtual methods
    virtual void setPeerAddress(const char* peerHostOrIP) override;
    virtual void setPeerPort(int port) override;
    virtual void setSocket(int& socket) override { /* N/A for WebRTC */ }
    
    virtual int receivePacket(char* buf, const size_t n);
    virtual int sendPacket(const char* buf, const size_t n);
    
private:
    rtc::scoped_refptr<webrtc::DataChannelInterface> mDataChannel;
    std::queue<std::vector<uint8_t>> mReceiveQueue;
    QMutex mQueueMutex;
    QWaitCondition mDataAvailable;
};
```

#### 2. `WebRtcSignalingProtocol`

Handles WebRTC signaling protocol over TCP/WebSocket.

```cpp
class WebRtcSignalingProtocol : public QObject
{
    Q_OBJECT;

public:
    // Message types for signaling protocol
    enum MessageType {
        OFFER = 1,
        ANSWER = 2,
        ICE_CANDIDATE = 3,
        HANGUP = 4
    };
    
    struct SignalingMessage {
        MessageType type;
        QString sdp;           // For OFFER/ANSWER
        QString candidate;     // For ICE_CANDIDATE
        QString sdpMid;
        int sdpMLineIndex;
    };
    
    static QByteArray encodeMessage(const SignalingMessage& msg);
    static SignalingMessage decodeMessage(const QByteArray& data);
    
signals:
    void offerReceived(const QString& sdp);
    void answerReceived(const QString& sdp);
    void iceCandidateReceived(const QString& candidate, const QString& sdpMid, int sdpMLineIndex);
    void hangupReceived();
};
```

#### 3. `WebRtcPeerConnection`

Wraps the WebRTC peer connection and manages the data channel.

```cpp
class WebRtcPeerConnection : public QObject,
                             public webrtc::PeerConnectionObserver,
                             public webrtc::DataChannelObserver
{
    Q_OBJECT;

public:
    WebRtcPeerConnection(QObject* parent = nullptr);
    virtual ~WebRtcPeerConnection();
    
    // Initialize as server (create answer)
    void setRemoteOffer(const QString& sdp);
    QString createAnswer();
    
    // ICE handling
    void addIceCandidate(const QString& candidate, const QString& sdpMid, int sdpMLineIndex);
    
    // Data channel access
    rtc::scoped_refptr<webrtc::DataChannelInterface> getDataChannel();
    
    // PeerConnectionObserver overrides
    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) override;
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnConnectionChange(webrtc::PeerConnectionInterface::PeerConnectionState state) override;
    
    // DataChannelObserver overrides
    void OnStateChange() override;
    void OnMessage(const webrtc::DataBuffer& buffer) override;
    
signals:
    void localIceCandidate(const QString& candidate, const QString& sdpMid, int sdpMLineIndex);
    void dataChannelOpen();
    void dataChannelClosed();
    void dataReceived(const QByteArray& data);
    void connectionFailed(const QString& reason);

private:
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> mPeerConnectionFactory;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> mPeerConnection;
    rtc::scoped_refptr<webrtc::DataChannelInterface> mDataChannel;
};
```

### Modified Classes

#### `UdpHubListener` Modifications

```cpp
class UdpHubListener : public QObject
{
    // ... existing code ...

private:
    // New: Detect client protocol from initial message
    enum ClientProtocol {
        PROTOCOL_UDP,      // Traditional UDP client
        PROTOCOL_WEBRTC    // WebRTC data channel client
    };
    
    ClientProtocol detectClientProtocol(QSslSocket* clientConnection);
    
    // New: WebRTC-specific handling
    void handleWebRtcClient(QSslSocket* clientConnection, const QByteArray& initialData);
    void processWebRtcSignaling(QSslSocket* connection, WebRtcSignalingProtocol::SignalingMessage& msg);
    
    // Store WebRTC peer connections (indexed same as mJTWorkers)
    QVector<WebRtcPeerConnection*> mWebRtcConnections;
};
```

#### `JackTrip` Modifications

```cpp
class JackTrip : public QObject
{
    // ... existing code ...

public:
    enum dataProtocolT {
        UDP,     // Existing
        TCP,     // Existing (not implemented)
        SCTP,    // Existing (not implemented)
        WEBRTC   // NEW: WebRTC Data Channel
    };
    
    // New: Set WebRTC data channel for transport
    void setWebRtcDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel);
    
    // Modified: Setup appropriate data protocol based on type
    virtual void setupDataProtocol() override;
};
```

### Signaling Protocol Messages

Signaling is handled via **WebSocket** on the same TCP port as the JackTrip server (e.g., 4464).
The server auto-detects WebSocket upgrade requests by looking for HTTP `GET` requests with
`Upgrade: websocket` headers. Legacy UDP clients send binary data directly and are handled
as before.

The signaling messages are JSON-encoded text messages sent over the WebSocket connection:

```json
// Protocol detection (first message from client)
{
    "protocol": "webrtc",  // or "udp" for legacy
    "version": 1,
    "client_name": "MyClient"
}

// SDP Offer (client -> server)
{
    "type": "offer",
    "sdp": "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\n..."
}

// SDP Answer (server -> client)
{
    "type": "answer",
    "sdp": "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\n..."
}

// ICE Candidate (bidirectional)
{
    "type": "ice",
    "candidate": "candidate:1 1 UDP 2130706431 192.168.1.1 12345 typ host",
    "sdpMid": "data",
    "sdpMLineIndex": 0
}

// Hangup (bidirectional)
{
    "type": "hangup"
}
```

### Data Channel Configuration

For low-latency audio, the data channel should be configured for unordered, unreliable delivery (similar to UDP):

```cpp
webrtc::DataChannelInit config;
config.ordered = false;           // Don't wait for in-order delivery
config.maxRetransmits = 0;        // No retransmissions (like UDP)
// OR alternatively:
// config.maxPacketLifeTime = 50; // Max 50ms lifetime for packets
```

---

## Implementation Plan

### Phase 1: Infrastructure (Week 1-2)

1. **Add WebRTC library dependency** (libwebrtc or libdatachannel)
2. **Create `WebRtcPeerConnection` class**
3. **Create `WebRtcSignalingProtocol` class**
4. **Create unit tests for WebRTC components**

### Phase 2: Data Protocol (Week 2-3)

1. **Create `WebRtcDataProtocol` class**
2. **Modify `JackTrip::setupDataProtocol()` to support WebRTC**
3. **Test audio transmission over data channel**

### Phase 3: Server Integration (Week 3-4)

1. **Modify `UdpHubListener` for protocol detection**
2. **Implement WebRTC signaling flow in hub listener**
3. **Modify `JackTripWorker` to support WebRTC peers**
4. **Integration testing with mixed UDP/WebRTC clients**

### Phase 4: Client Support (Week 4-5)

1. **Add WebRTC client mode to JackTrip**
2. **Create browser-based JavaScript client (optional)**
3. **End-to-end testing**

---

## Library Options

### libdatachannel (Used)

- Lightweight, C++17
- Focused specifically on data channels
- MIT license
- Easy to integrate
- https://github.com/paullouisageneau/libdatachannel

The library is configured as a meson subproject and will be automatically downloaded
and built if not found on the system. See `subprojects/libdatachannel.wrap`.

```cpp
// Example usage with libdatachannel
#include <rtc/rtc.hpp>

rtc::Configuration config;
config.iceServers.emplace_back("stun:stun.l.google.com:19302");

auto pc = std::make_shared<rtc::PeerConnection>(config);

pc->onLocalDescription([](rtc::Description description) {
    // Send to peer via signaling
});

pc->onLocalCandidate([](rtc::Candidate candidate) {
    // Send to peer via signaling
});

pc->onDataChannel([](std::shared_ptr<rtc::DataChannel> dc) {
    dc->onMessage([](rtc::message_variant data) {
        // Handle audio packet
    });
});
```

---

## Building

WebRTC support is auto-detected. If libdatachannel is available (either as a system
package or via the subproject), WebRTC support is enabled automatically.

```bash
# Standard build (WebRTC auto-detected)
meson setup builddir
meson compile -C builddir

# Explicitly disable libdatachannel/WebRTC
meson setup builddir -Dlibdatachannel=disabled

# Force libdatachannel/WebRTC (error if not available)
meson setup builddir -Dlibdatachannel=enabled
```

The build summary will show whether WebRTC Data Channels are enabled.

### Alternative: libwebrtc (Google's Implementation)

- Full WebRTC implementation
- Larger binary size
- More complex build process
- Used by Chrome and other browsers

Not used due to complexity. libdatachannel provides all needed functionality.

---

## Port Allocation Strategy

To reuse the same port range for both UDP and WebRTC clients:

```cpp
// In UdpHubListener::getJackTripWorker()
int getJackTripWorker(const QString& address, uint16_t port, 
                      QString& clientName, ClientProtocol protocol)
{
    QMutexLocker lock(&mMutex);
    
    // Find empty slot (same logic for both protocols)
    int id = -1;
    for (int i = 0; i < gMaxThreads; i++) {
        if (mJTWorkers->at(i) == nullptr) {
            id = i;
            break;
        }
    }
    
    if (id >= 0) {
        mTotalRunningThreads++;
        
        if (protocol == PROTOCOL_WEBRTC) {
            // Create WebRTC-specific worker
            // Port number is conceptual for WebRTC (slot ID)
            mJTWorkers->replace(id,
                new JackTripWorker(this, mBufferQueueLength, mUnderRunMode,
                                   mAudioBitResolution, clientName));
            mJTWorkers->at(id)->setProtocol(JackTrip::WEBRTC);
            mJTWorkers->at(id)->setJackTrip(id, address, mBasePort + id, 0, 
                                            m_connectDefaultAudioPorts);
        } else {
            // Existing UDP logic
            mJTWorkers->replace(id,
                new JackTripWorker(this, mBufferQueueLength, mUnderRunMode,
                                   mAudioBitResolution, clientName));
            mJTWorkers->at(id)->setJackTrip(id, address, mBasePort + id, 0,
                                            m_connectDefaultAudioPorts);
        }
    }
    
    return id;
}
```

---

## Backward Compatibility

The design maintains full backward compatibility:

1. **Protocol Detection**: First bytes of client message determine protocol
   - Legacy clients send 32-bit port number directly
   - WebRTC clients send JSON with `"protocol": "webrtc"`

2. **Port Behavior**: 
   - UDP clients get actual UDP port numbers
   - WebRTC clients get slot IDs (same numbering, different meaning)

3. **Audio Interface**: 
   - Both protocols create identical Jack/RtAudio ports
   - Hub patching works identically for both

---

## Testing Strategy

### Unit Tests
- WebRTC signaling message encoding/decoding
- Data channel configuration
- Packet header serialization

### Integration Tests
- Single WebRTC client to hub server
- Mixed UDP + WebRTC clients
- Client reconnection handling
- ICE restart scenarios

### Performance Tests
- Latency comparison (UDP vs WebRTC data channel)
- CPU usage under load
- Memory usage with many connections

---

## Appendix: Alternative Approaches Considered

### 1. WebSocket Transport
- **Pros**: Simpler than WebRTC, works through proxies
- **Cons**: TCP-based (higher latency), no P2P capability

### 2. SCTP over UDP
- **Pros**: Similar to WebRTC data channels
- **Cons**: Limited NAT traversal, less tooling

### 3. QUIC
- **Pros**: Low latency, good congestion control
- **Cons**: Less browser support, newer protocol

**Decision**: WebRTC data channels chosen for NAT traversal, browser compatibility, and proven reliability.

