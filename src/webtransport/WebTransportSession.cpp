//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file WebTransportSession.cpp
 * \author Mike Dickey + Claude AI
 * \date 2026
 */

#include "WebTransportSession.h"

#include <msquic.h>

#include <QMap>
#include <QUrl>
#include <QUrlQuery>
#include <cstring>
#include <iostream>
#include <thread>

#include "../http3/Http3Protocol.h"
#include "../jacktrip_globals.h"
#include "WebTransportDataProtocol.h"

using std::cerr;
using std::cout;
using std::endl;

// Default maximum datagram size (conservative estimate)
static constexpr size_t DEFAULT_MAX_DATAGRAM_SIZE = 1200;

//*******************************************************************************
// Static callback handlers for msquic
//*******************************************************************************

static QUIC_STATUS QUIC_API ConnectionCallback(HQUIC /* connection */, void* context,
                                               QUIC_CONNECTION_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        return session->handleConnectionEvent(event);
    }
    return QUIC_STATUS_INVALID_STATE;
}

static QUIC_STATUS QUIC_API StreamCallback(HQUIC stream, void* context,
                                           QUIC_STREAM_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        QUIC_STATUS result = session->handleStreamEvent(stream, event);
        return result;
    }
    return QUIC_STATUS_INVALID_STATE;
}

// Callback for server-initiated infrastructure streams (control, QPACK)
static QUIC_STATUS QUIC_API InfraStreamCallback(HQUIC stream, void* context,
                                                QUIC_STREAM_EVENT* event)
{
    WebTransportSession* session = static_cast<WebTransportSession*>(context);
    if (session) {
        QUIC_STATUS result = session->handleInfraStreamEvent(stream, event);
        return result;
    }
    return QUIC_STATUS_INVALID_STATE;
}

//*******************************************************************************
WebTransportSession::WebTransportSession(const QUIC_API_TABLE* api, HQUIC connection,
                                         const QHostAddress& peerAddress,
                                         quint16 peerPort, QObject* parent)
    : QObject(parent)
    , mApi(api)
    , mConnection(connection)
    , mControlStream(nullptr)
    , mQpackEncoderStream(nullptr)
    , mQpackDecoderStream(nullptr)
    , mConnectStream(nullptr)
    , mConnectStreamId(0)
    , mState(STATE_NEW)
    , mPeerAddress(peerAddress)
    , mPeerPort(peerPort)
    , mSessionAccepted(false)
    , mControlStreamReady(false)
    , mQpackEncoderStreamReady(false)
    , mQpackDecoderStreamReady(false)
    , mClientSettingsReceived(false)
    , mServerSettingsSent(false)
    , mMaxDatagramSize(DEFAULT_MAX_DATAGRAM_SIZE)
{
    if (mConnection && mApi) {
        // Set the callback context to this session
        mApi->SetCallbackHandler(mConnection, (void*)ConnectionCallback, this);

        // Enable datagram support
        QUIC_SETTINGS settings{};
        std::memset(&settings, 0, sizeof(settings));
        settings.DatagramReceiveEnabled       = TRUE;
        settings.IsSet.DatagramReceiveEnabled = TRUE;

        QUIC_STATUS status = mApi->SetParam(mConnection, QUIC_PARAM_CONN_SETTINGS,
                                            sizeof(settings), &settings);
        if (QUIC_FAILED(status)) {
            cerr << "WebTransportSession: Failed to enable datagram support, status: 0x"
                 << std::hex << status << std::dec << endl;
        } else {
            setState(STATE_CONNECTING);
        }
    } else {
        cerr << "WebTransportSession: ERROR - Connection or API is null!" << endl;
    }
}

//*******************************************************************************
WebTransportSession::~WebTransportSession()
{
    // If connection is still active, initiate shutdown and wait for it to complete
    std::unique_lock<std::mutex> lock(mMutex);

    if (mConnection && mApi && !mShutdownComplete) {
        // Mark as disconnected so SHUTDOWN_COMPLETE callback won't emit signals
        mState = STATE_DISCONNECTED;

        mApi->ConnectionShutdown(mConnection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);

        // Wait for SHUTDOWN_COMPLETE callback (releases mMutex while waiting)
        bool completed = false;
        do {
            completed = mShutdownCv.wait_for(lock, std::chrono::seconds(5), [this] {
                return mShutdownComplete;  // Protected by mMutex (held when predicate is
                                           // checked)
            });
        } while (!completed);
    }

    // Now safe to close handles - msquic is done with them (still holding mMutex)
    if (mControlStream && mApi) {
        mApi->StreamClose(mControlStream);
        mControlStream = nullptr;
    }

    if (mQpackEncoderStream && mApi) {
        mApi->StreamClose(mQpackEncoderStream);
        mQpackEncoderStream = nullptr;
    }

    if (mQpackDecoderStream && mApi) {
        mApi->StreamClose(mQpackDecoderStream);
        mQpackDecoderStream = nullptr;
    }

    if (mConnection && mApi) {
        mApi->ConnectionClose(mConnection);
        mConnection = nullptr;
    }
}

//*******************************************************************************
bool WebTransportSession::processConnectRequest(const QString& path)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mState != STATE_CONNECTING) {
        cerr << "WebTransportSession: Invalid state for CONNECT request (state=" << mState
             << ", expected=" << STATE_CONNECTING << ")" << endl;
        return false;
    }

    // Parse client name from path
    parseClientNameFromPath(path);

    // Mark session as accepted - will send response
    mSessionAccepted = true;
    return true;
}

//*******************************************************************************
bool WebTransportSession::sendConnectResponse(int statusCode)
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mConnection || !mApi) {
        cerr << "WebTransportSession: Cannot send response - connection or API is null"
             << endl;
        return false;
    }

    // Send the actual HTTP/3 response to the client
    if (!sendHttp3Response(statusCode)) {
        cerr << "WebTransportSession: Failed to send HTTP/3 response" << endl;
        setState(STATE_FAILED);
        emit sessionFailed(QStringLiteral("Failed to send HTTP/3 response"));
        return false;
    }

    if (statusCode == 200) {
        // Session accepted - transition to connected state
        setState(STATE_CONNECTED);
        emit sessionEstablished();
        return true;
    } else {
        // Session rejected
        cerr << "WebTransportSession: Rejecting session with status " << statusCode
             << endl;
        setState(STATE_FAILED);
        emit sessionFailed(
            QStringLiteral("CONNECT request rejected with status %1").arg(statusCode));
        return false;
    }
}

//*******************************************************************************
bool WebTransportSession::sendDatagram(uint8_t* buffer, size_t length,
                                       QUIC_BUFFER* quicBuf, void* owner)
{
    // Lock-free check - safe to call from real-time audio thread
    if (!isConnected() || !mConnection || !mApi || !buffer || !owner || !quicBuf) {
        return false;
    }

    // Calculate total size needed (varint prefix + payload)
    uint64_t quarterStreamId = mConnectStreamId / 4;
    uint8_t varintBuf[8];
    size_t varintLen = Http3::encodeQuicVarint(quarterStreamId, varintBuf);
    size_t totalLen  = varintLen + length;

    if (totalLen > mMaxDatagramSize) {
        return false;
    }

    // Shift data to make room for varint prefix
    // Move from right to left to avoid overwriting
    std::memmove(buffer + varintLen, buffer, length);

    // Write varint prefix at the beginning
    std::memcpy(buffer, varintBuf, varintLen);

    // Setup QUIC buffer in the persistent context (not on stack!)
    // This must stay alive until MsQuic's worker thread processes it
    quicBuf->Buffer = buffer;
    quicBuf->Length = static_cast<uint32_t>(totalLen);

    // Send the datagram - pass owner as context for cleanup in callback
    QUIC_STATUS status =
        mApi->DatagramSend(mConnection, quicBuf, 1, QUIC_SEND_FLAG_NONE, owner);

    if (QUIC_FAILED(status)) {
        // Shift data back on failure
        std::memmove(buffer, buffer + varintLen, length);
        return false;
    }

    return true;
}

//*******************************************************************************
void WebTransportSession::close()
{
    std::lock_guard<std::mutex> lock(mMutex);

    if (mState == STATE_SHUTTING_DOWN || mState == STATE_DISCONNECTED
        || mState == STATE_FAILED) {
        // STATE_FAILED means the transport already initiated shutdown; MsQuic will fire
        // SHUTDOWN_COMPLETE on its own — no need to call ConnectionShutdown again, and
        // transitioning to STATE_SHUTTING_DOWN here would cause SHUTDOWN_COMPLETE to
        // emit sessionClosed in addition to the already-emitted sessionFailed.
        return;
    }

    setState(STATE_SHUTTING_DOWN);

    if (mConnection && mApi && !mShutdownComplete) {
        // Initiate graceful shutdown - SHUTDOWN_COMPLETE callback will emit sessionClosed
        mApi->ConnectionShutdown(mConnection, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    }
}

//*******************************************************************************
void WebTransportSession::setState(SessionState state)
{
    if (mState != state) {
        mState = state;
        emit stateChanged(state);
    }
}

//*******************************************************************************
void WebTransportSession::parseClientNameFromPath(const QString& path)
{
    QUrl url(QStringLiteral("http://localhost") + path);
    QUrlQuery query(url);
    if (query.hasQueryItem(QStringLiteral("name"))) {
        mClientName = query.queryItemValue(QStringLiteral("name"));
    }
}

//*******************************************************************************
void WebTransportSession::createInfrastructureStreams()
{
    // Called from QUIC_CONNECTION_EVENT_CONNECTED
    // Create all three HTTP/3 infrastructure streams:
    // 1. Control stream (type 0x00) - required for HTTP/3
    // 2. QPACK encoder stream (type 0x02) - required by browsers
    // 3. QPACK decoder stream (type 0x03) - required by browsers

    if (!createInfraStream(H3_STREAM_CONTROL, &mControlStream)) {
        cerr << "WebTransportSession: Failed to create control stream" << endl;
        return;
    }

    if (!createInfraStream(H3_STREAM_QPACK_ENCODER, &mQpackEncoderStream)) {
        cerr << "WebTransportSession: Failed to create QPACK encoder stream" << endl;
        return;
    }

    if (!createInfraStream(H3_STREAM_QPACK_DECODER, &mQpackDecoderStream)) {
        cerr << "WebTransportSession: Failed to create QPACK decoder stream" << endl;
        return;
    }
}

//*******************************************************************************
bool WebTransportSession::createInfraStream(H3StreamType type, HQUIC* streamHandle)
{
    if (!mConnection || !mApi || !streamHandle) {
        return false;
    }

    const char* streamNames[] = {"control", "push", "QPACK encoder", "QPACK decoder"};

    // Open a unidirectional stream with our callback handler
    // The callback will be invoked when START_COMPLETE fires
    QUIC_STATUS status =
        mApi->StreamOpen(mConnection, QUIC_STREAM_OPEN_FLAG_UNIDIRECTIONAL,
                         InfraStreamCallback,  // Use infrastructure stream callback
                         this, streamHandle);

    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to open " << streamNames[type]
             << " stream, status: 0x" << std::hex << status << std::dec << endl;
        return false;
    }

    // Start the stream - data will be sent when START_COMPLETE fires
    status = mApi->StreamStart(*streamHandle, QUIC_STREAM_START_FLAG_NONE);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to start " << streamNames[type]
             << " stream, status: 0x" << std::hex << status << std::dec << endl;
        mApi->StreamClose(*streamHandle);
        *streamHandle = nullptr;
        return false;
    }

    return true;
}

//*******************************************************************************
void WebTransportSession::sendStreamType(HQUIC stream, H3StreamType type)
{
    if (!stream || !mApi) {
        return;
    }

    // Allocate buffer that will persist until SEND_COMPLETE
    // msquic requires the buffer to remain valid until the send completes
    uint8_t* typeData = new uint8_t[1];
    typeData[0]       = static_cast<uint8_t>(type);

    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer      = typeData;
    buffer->Length      = 1;

    // Send the stream type byte
    // Context is used to free the buffer in SEND_COMPLETE
    QUIC_STATUS status = mApi->StreamSend(stream, buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send stream type, status: 0x" << std::hex
             << status << std::dec << endl;
        delete[] typeData;
        delete buffer;
    }
}

//*******************************************************************************
std::vector<uint8_t> WebTransportSession::buildSettingsFrame()
{
    // Build HTTP/3 SETTINGS frame
    std::vector<uint8_t> frame;

    // Build SETTINGS payload first to calculate length
    std::vector<uint8_t> settingsPayload;

    // SETTINGS_QPACK_MAX_TABLE_CAPACITY (0x01) = 0 (we don't use dynamic table)
    settingsPayload.push_back(0x01);
    settingsPayload.push_back(0x00);

    // SETTINGS_QPACK_BLOCKED_STREAMS (0x07) = 0
    settingsPayload.push_back(0x07);
    settingsPayload.push_back(0x00);

    // SETTINGS_ENABLE_CONNECT_PROTOCOL (0x08) = 1
    // Required for extended CONNECT (WebTransport)
    settingsPayload.push_back(0x08);
    settingsPayload.push_back(0x01);

    // SETTINGS_H3_DATAGRAM (0x33 = 51 decimal) = 1
    // This enables datagrams (HTTP Datagrams RFC 9297)
    settingsPayload.push_back(0x33);
    settingsPayload.push_back(0x01);

    // SETTINGS_ENABLE_WEBTRANSPORT (0x2b603742 = 729713730 decimal) = 1
    // Encoded as 4-byte varint: top 2 bits = 0b10 (4 byte)
    settingsPayload.push_back(0xab);  // 0b10101011 (2-bit prefix 10 = 4 bytes)
    settingsPayload.push_back(0x60);
    settingsPayload.push_back(0x37);
    settingsPayload.push_back(0x42);
    settingsPayload.push_back(0x01);  // value = 1 (enabled)

    // Frame type: 0x04 (SETTINGS)
    frame.push_back(0x04);

    // Frame length (varint)
    size_t payloadLen = settingsPayload.size();
    if (payloadLen < 64) {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    } else {
        // Multi-byte varint (shouldn't happen for our small payload)
        frame.push_back(0x40 | static_cast<uint8_t>(payloadLen >> 8));
        frame.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    }

    // Append payload
    frame.insert(frame.end(), settingsPayload.begin(), settingsPayload.end());

    return frame;
}

//*******************************************************************************
void WebTransportSession::sendSettingsFrame()
{
    // Called after receiving client's SETTINGS and our control stream is ready
    if (mServerSettingsSent) {
        return;
    }

    if (!mControlStream || !mApi) {
        cerr << "WebTransportSession: Cannot send SETTINGS - control stream not available"
             << endl;
        return;
    }

    if (!mControlStreamReady) {
        return;
    }

    if (!mClientSettingsReceived) {
        return;
    }

    mServerSettingsSent = true;

    // Build the SETTINGS frame
    std::vector<uint8_t> settingsFrame = buildSettingsFrame();

    // Allocate buffer that persists until SEND_COMPLETE
    size_t dataSize    = settingsFrame.size();
    uint8_t* frameData = new uint8_t[dataSize];
    std::memcpy(frameData, settingsFrame.data(), dataSize);

    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer      = frameData;
    buffer->Length      = static_cast<uint32_t>(dataSize);

    QUIC_STATUS status =
        mApi->StreamSend(mControlStream, buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send SETTINGS, status: 0x" << std::hex
             << status << std::dec << endl;
        mServerSettingsSent = false;
        delete[] frameData;
        delete buffer;
        return;
    }
}

//*******************************************************************************
std::vector<uint8_t> WebTransportSession::buildResponseFrame(int statusCode)
{
    // Build HTTP/3 HEADERS frame with :status response
    // Using QPACK static table encoding
    std::vector<uint8_t> frame;

    // Build QPACK-encoded headers
    std::vector<uint8_t> qpackPayload;

    // QPACK prefix: Required Insert Count = 0, Delta Base = 0
    qpackPayload.push_back(0x00);  // Required Insert Count (8-bit prefix)
    qpackPayload.push_back(0x00);  // Delta Base with S=0 (7-bit prefix)

    // Encode :status header using QPACK static table
    // Format: Indexed Header Field (1T pattern, T=1 for static table)
    // QPACK static table indices for :status:
    //   25 = :status: 200
    //   63 = :status: 100
    //   64 = :status: 204
    //   67 = :status: 400
    //   68 = :status: 403
    //   27 = :status: 404
    //   71 = :status: 500
    //   28 = :status: 503

    int staticIndex = -1;
    switch (statusCode) {
    case 200:
        staticIndex = 25;
        break;
    case 100:
        staticIndex = 63;
        break;
    case 204:
        staticIndex = 64;
        break;
    case 400:
        staticIndex = 67;
        break;
    case 403:
        staticIndex = 68;
        break;
    case 404:
        staticIndex = 27;
        break;
    case 500:
        staticIndex = 71;
        break;
    case 503:
        staticIndex = 28;
        break;
    default:
        // For other status codes, use literal encoding
        staticIndex = -1;
        break;
    }

    if (staticIndex >= 0 && staticIndex < 64) {
        // Indexed Header Field: 1 1 index (6-bit prefix)
        // Pattern: 11xxxxxx for static table index
        qpackPayload.push_back(0xC0 | static_cast<uint8_t>(staticIndex));
    } else {
        // Literal Header Field With Name Reference
        // Use static table index 24 for :status name (":status", "103")
        // Then provide literal value
        // Pattern: 0101 NNNN for static table name reference
        qpackPayload.push_back(
            0x5F);  // 01011111 = literal with name ref, static, index needs continuation
        qpackPayload.push_back(0x09);  // Index 24 encoded with 4-bit prefix (24 - 15 = 9)

        // Status value as literal string
        std::string statusStr = std::to_string(statusCode);
        qpackPayload.push_back(
            static_cast<uint8_t>(statusStr.length()));  // Length (no huffman)
        for (char c : statusStr) {
            qpackPayload.push_back(static_cast<uint8_t>(c));
        }
    }

    // Add sec-webtransport-http3-draft header for WebTransport compatibility
    // This is a literal header with literal name and value
    // QPACK literal with literal name format: 0010 NHLL
    //   N = never index (1), H = huffman for name (0), LL = 3-bit length prefix
    if (statusCode == 200) {
        const char* headerName  = "sec-webtransport-http3-draft";
        const char* headerValue = "draft02";
        size_t nameLen          = strlen(headerName);
        size_t valueLen         = strlen(headerValue);

        // First byte: 0010 N H LL (N=1 never index, H=0 no huffman)
        // 0x2? where ? encodes the length with 3-bit prefix
        if (nameLen < 8) {
            qpackPayload.push_back(0x20 | static_cast<uint8_t>(nameLen));
        } else {
            // Length >= 8, need continuation
            qpackPayload.push_back(0x27);  // 0010 0111 = max 3-bit prefix value
            // Continuation: nameLen - 7 encoded as varint
            size_t remaining = nameLen - 7;
            while (remaining >= 128) {
                qpackPayload.push_back(0x80 | (remaining & 0x7F));
                remaining >>= 7;
            }
            qpackPayload.push_back(static_cast<uint8_t>(remaining));
        }

        // Name bytes (lowercase, no huffman)
        for (size_t i = 0; i < nameLen; i++) {
            qpackPayload.push_back(static_cast<uint8_t>(headerName[i]));
        }

        // Value length (7-bit prefix, H=0 no huffman)
        if (valueLen < 128) {
            qpackPayload.push_back(static_cast<uint8_t>(valueLen));
        } else {
            qpackPayload.push_back(0x7F);
            size_t remaining = valueLen - 127;
            while (remaining >= 128) {
                qpackPayload.push_back(0x80 | (remaining & 0x7F));
                remaining >>= 7;
            }
            qpackPayload.push_back(static_cast<uint8_t>(remaining));
        }

        // Value bytes
        for (size_t i = 0; i < valueLen; i++) {
            qpackPayload.push_back(static_cast<uint8_t>(headerValue[i]));
        }
    }

    // Build the HEADERS frame
    // Frame type: 0x01 (HEADERS)
    frame.push_back(0x01);

    // Frame length (varint)
    size_t payloadLen = qpackPayload.size();
    if (payloadLen < 64) {
        frame.push_back(static_cast<uint8_t>(payloadLen));
    } else {
        frame.push_back(0x40 | static_cast<uint8_t>(payloadLen >> 8));
        frame.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    }

    // Append QPACK payload
    frame.insert(frame.end(), qpackPayload.begin(), qpackPayload.end());

    return frame;
}

//*******************************************************************************
bool WebTransportSession::sendHttp3Response(int statusCode)
{
    if (!mConnectStream || !mApi) {
        cerr << "WebTransportSession: Cannot send HTTP/3 response - no CONNECT stream"
             << endl;
        return false;
    }

    // Build the response frame
    std::vector<uint8_t> responseFrame = buildResponseFrame(statusCode);

    // Allocate buffer that persists until SEND_COMPLETE
    size_t dataSize    = responseFrame.size();
    uint8_t* frameData = new uint8_t[dataSize];
    std::memcpy(frameData, responseFrame.data(), dataSize);

    QUIC_BUFFER* buffer = new QUIC_BUFFER;
    buffer->Buffer      = frameData;
    buffer->Length      = static_cast<uint32_t>(dataSize);

    // Send the response - use the peer's CONNECT stream
    // The buffer context is used to free memory in the stream callback
    QUIC_STATUS status =
        mApi->StreamSend(mConnectStream, buffer, 1, QUIC_SEND_FLAG_NONE, buffer);
    if (QUIC_FAILED(status)) {
        cerr << "WebTransportSession: Failed to send HTTP/3 response, status: 0x"
             << std::hex << status << std::dec << endl;
        delete[] frameData;
        delete buffer;
        return false;
    }

    return true;
}

//*******************************************************************************
unsigned int WebTransportSession::handleConnectionEvent(void* eventPtr)
{
    QUIC_CONNECTION_EVENT* event = static_cast<QUIC_CONNECTION_EVENT*>(eventPtr);

    switch (event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        if (gVerboseFlag) {
            cout << "WebTransportSession: QUIC connection established from "
                 << mPeerAddress.toString().toStdString() << ":" << mPeerPort << endl;
        }

        // Create HTTP/3 infrastructure streams immediately on connection
        // This follows the libwtf pattern - create control + QPACK streams right away
        // The streams will send their type byte when START_COMPLETE fires
        createInfrastructureStreams();

        // Connection is ready, but we wait for HTTP/3 CONNECT before declaring
        // session established
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
        // Common QUIC error codes for debugging:
        // 0x65 (101) = Connection refused or protocol error
        // 0x0A (10)  = No application protocol
        // 0x01 (1)   = Internal error
        setState(STATE_FAILED);
        // Cleanup will happen in SHUTDOWN_COMPLETE
        emit sessionFailed(QStringLiteral("Transport shutdown"));
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        // Peer initiated shutdown - transition to SHUTTING_DOWN and wait for
        // SHUTDOWN_COMPLETE to emit the sessionClosed signal
        if (mState == STATE_CONNECTED) {
            setState(STATE_SHUTTING_DOWN);
        }
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE: {
        bool shouldEmitClosed = false;
        {
            // Set shutdown complete flag, check/update state, all protected by mutex
            std::lock_guard<std::mutex> lock(mMutex);
            mShutdownComplete = true;

            // Emit sessionClosed signal only if we're in SHUTTING_DOWN state
            // (i.e., close() was called but destructor hasn't run yet)
            // If state is DISCONNECTED, the destructor is running and we shouldn't emit
            if (mState == STATE_SHUTTING_DOWN) {
                setState(STATE_DISCONNECTED);
                shouldEmitClosed = true;
            }
        }  // mMutex released here

        // Notify outside the lock (standard pattern - avoids waking waiting thread while
        // we hold lock)
        mShutdownCv.notify_all();

        // Emit signal outside the lock to avoid holding mutex during potentially slow
        // signal/slot calls
        if (shouldEmitClosed) {
            emit sessionClosed();
        }

        break;
    }

    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        // A new stream was started by the peer
        // This could be the HTTP/3 control stream or the CONNECT stream
        if (mApi) {
            mApi->SetCallbackHandler(event->PEER_STREAM_STARTED.Stream,
                                     (void*)StreamCallback, this);
        }
        break;

    case QUIC_CONNECTION_EVENT_DATAGRAM_RECEIVED: {
        // Received a QUIC datagram - strip the quarter stream ID prefix
        const QUIC_BUFFER* buffer = event->DATAGRAM_RECEIVED.Buffer;
        if (buffer && buffer->Length > 0) {
            const uint8_t* data = buffer->Buffer;
            size_t len          = buffer->Length;

            // Decode QUIC varint to get prefix length
            size_t varintLen = 0;
            if (len > 0) {
                uint8_t firstByte = data[0];
                uint8_t prefix    = firstByte >> 6;
                varintLen         = 1 << prefix;  // 1, 2, 4, or 8 bytes
            }

            // Skip the quarter stream ID prefix and deliver directly (zero-copy)
            if (len > varintLen) {
                const uint8_t* payload = data + varintLen;
                size_t payloadLen      = len - varintLen;

                // Direct callback invocation (audio hot path, no Qt overhead)
                if (mDatagramCallback) {
                    mDatagramCallback(payload, payloadLen);
                }
            }
        }
        break;
    }

    case QUIC_CONNECTION_EVENT_DATAGRAM_STATE_CHANGED:
        // Update maximum datagram size based on path MTU
        if (event->DATAGRAM_STATE_CHANGED.SendEnabled) {
            mMaxDatagramSize = event->DATAGRAM_STATE_CHANGED.MaxSendLength;
        } else {
            cerr << "WebTransportSession: WARNING - Datagrams NOT enabled by peer!"
                 << endl;
        }
        break;

    case QUIC_CONNECTION_EVENT_DATAGRAM_SEND_STATE_CHANGED:
        // Datagram send state changed - only release buffer on final states
        // States: 0=Sent, 1=LostSuspect, 2=LostDiscarded, 3=Acknowledged,
        //         4=AcknowledgedSpurious, 5=Canceled
        // Final states are >= 2 (LostDiscarded, Acknowledged, AcknowledgedSpurious,
        // Canceled)
        if (event->DATAGRAM_SEND_STATE_CHANGED.State
            >= QUIC_DATAGRAM_SEND_LOST_DISCARDED) {
            void* ctx = event->DATAGRAM_SEND_STATE_CHANGED.ClientContext;
            if (ctx && reinterpret_cast<uintptr_t>(ctx) > 0x10000) {
                // This is a SendContext from the pool - release it
                WebTransportDataProtocol::releaseSendContext(
                    static_cast<WebTransportDataProtocol::SendContext*>(ctx));
            }
        }
        break;

    case QUIC_CONNECTION_EVENT_IDEAL_PROCESSOR_CHANGED:
        // CPU affinity hint - can be safely ignored
        break;

    case QUIC_CONNECTION_EVENT_STREAMS_AVAILABLE:
        if (gVerboseFlag) {
            cout << "WebTransportSession: STREAMS_AVAILABLE - Bidi: "
                 << event->STREAMS_AVAILABLE.BidirectionalCount
                 << ", Unidi: " << event->STREAMS_AVAILABLE.UnidirectionalCount << endl;
        }
        break;

    case QUIC_CONNECTION_EVENT_PEER_NEEDS_STREAMS:
        if (gVerboseFlag) {
            cout << "WebTransportSession: PEER_NEEDS_STREAMS - Bidi: "
                 << (event->PEER_NEEDS_STREAMS.Bidirectional ? "yes" : "no") << endl;
        }
        break;

    case QUIC_CONNECTION_EVENT_RESUMED:
        if (gVerboseFlag) {
            cout << "WebTransportSession: CONNECTION_RESUMED" << endl;
        }
        break;

    case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
        if (gVerboseFlag) {
            cout << "WebTransportSession: RESUMPTION_TICKET_RECEIVED" << endl;
        }
        break;

    case QUIC_CONNECTION_EVENT_PEER_CERTIFICATE_RECEIVED:
        if (gVerboseFlag) {
            cout << "WebTransportSession: PEER_CERTIFICATE_RECEIVED" << endl;
        }
        break;

    default:
        if (gVerboseFlag) {
            cout << "WebTransportSession: Connection event type: " << event->Type
                 << " (unhandled)" << endl;
        }
        break;
    }

    return QUIC_STATUS_SUCCESS;
}

//*******************************************************************************
unsigned int WebTransportSession::handleStreamEvent(HQUIC stream, void* eventPtr)
{
    QUIC_STREAM_EVENT* event = static_cast<QUIC_STREAM_EVENT*>(eventPtr);

    switch (event->Type) {
    case QUIC_STREAM_EVENT_START_COMPLETE:
        break;

    case QUIC_STREAM_EVENT_RECEIVE: {
        // Received data on stream - could be HTTP/3 frames
        // For WebTransport, we primarily use datagrams for audio
        // but streams are used for signaling (CONNECT request)

        // Check for FIN flag indicating stream closure
        bool finReceived = (event->RECEIVE.Flags & QUIC_RECEIVE_FLAG_FIN) != 0;

        // First, determine if this is a unidirectional or bidirectional stream
        // In QUIC: stream_id bit 1 (0x02) = 1 means unidirectional
        uint64_t streamId     = 0;
        uint32_t streamIdSize = sizeof(streamId);
        bool isUnidirectional = false;

        if (mApi) {
            QUIC_STATUS status =
                mApi->GetParam(stream, QUIC_PARAM_STREAM_ID, &streamIdSize, &streamId);
            if (QUIC_SUCCEEDED(status)) {
                isUnidirectional = (streamId & 0x02) != 0;
            }
        }

        // If this is the CONNECT stream and we received FIN after being connected,
        // treat it as a client disconnect
        if (finReceived && stream == mConnectStream && mState == STATE_CONNECTED) {
            if (gVerboseFlag) {
                cout
                    << "WebTransportSession: Client closing CONNECT stream (FIN received)"
                    << endl;
            }
            // Client is disconnecting - close the connection gracefully
            close();
            break;
        }

        // Parse the HTTP/3 CONNECT request from the stream data
        if (event->RECEIVE.TotalBufferLength > 0) {
            QByteArray data;
            for (uint32_t i = 0; i < event->RECEIVE.BufferCount; i++) {
                data.append(
                    reinterpret_cast<const char*>(event->RECEIVE.Buffers[i].Buffer),
                    event->RECEIVE.Buffers[i].Length);
            }

            // Handle unidirectional streams (control, QPACK encoder/decoder)
            // These start with a stream type byte
            if (isUnidirectional && data.size() > 0) {
                uint8_t streamType = static_cast<uint8_t>(data[0]);

                // Check for HTTP/3 unidirectional stream types
                // 0x00 = Control, 0x02 = QPACK Encoder, 0x03 = QPACK Decoder
                if (streamType == 0x00 || streamType == 0x02 || streamType == 0x03) {
                    // If this is the client's control stream, look for SETTINGS frame
                    if (streamType == 0x00 && data.size() > 1) {
                        // Parse SETTINGS frame after stream type byte
                        size_t pos        = 1;  // Skip stream type byte
                        int64_t frameType = Http3::readVarint(
                            reinterpret_cast<const uint8_t*>(data.constData()),
                            data.size(), pos);

                        if (frameType == Http3::FRAME_SETTINGS) {
                            // Read frame length
                            int64_t frameLen = Http3::readVarint(
                                reinterpret_cast<const uint8_t*>(data.constData()),
                                data.size(), pos);

                            if (frameLen >= 0) {
                                // Mark client settings as received
                                mClientSettingsReceived = true;

                                // Now we can send our SETTINGS in response
                                // (if control stream is ready)
                                sendSettingsFrame();
                            }
                        }
                    }

                    // For infrastructure streams, don't process as CONNECT request
                    break;
                }

                // Unknown unidirectional stream type - log and ignore
                if (gVerboseFlag) {
                    cout << "WebTransportSession: Unknown unidirectional stream type: 0x"
                         << std::hex << static_cast<int>(streamType) << std::dec << endl;
                }
                break;
            }

            // Bidirectional stream - this should be the CONNECT request
            // These start directly with HTTP/3 frames (no stream type byte)

            // Parse HTTP/3 frame to extract QPACK payload
            const uint8_t* qpackPayload = nullptr;
            size_t qpackLen             = 0;

            if (!Http3::parseHttp3Frame(
                    reinterpret_cast<const uint8_t*>(data.constData()), data.size(),
                    qpackPayload, qpackLen)) {
                cerr << "WebTransportSession: Failed to parse HTTP/3 frame (not a "
                        "request stream?)"
                     << endl;
                break;
            }

            // Decode QPACK-encoded HTTP/3 headers
            QMap<QString, QString> headers;

            if (Http3::decodeQPackHeaders(qpackPayload, qpackLen, headers)) {
                // Check if this is a CONNECT request for WebTransport
                QString method    = headers.value(QStringLiteral(":method"));
                QString protocol  = headers.value(QStringLiteral(":protocol"));
                QString path      = headers.value(QStringLiteral(":path"),
                                                  QStringLiteral("/webtransport"));
                QString authority = headers.value(QStringLiteral(":authority"));

                if (gVerboseFlag) {
                    cout << "WebTransportSession: Request headers:" << endl;
                    cout << "  :method = " << method.toStdString() << endl;
                    cout << "  :protocol = " << protocol.toStdString() << endl;
                    cout << "  :path = " << path.toStdString() << endl;
                    cout << "  :authority = " << authority.toStdString() << endl;
                }

                if (method == QStringLiteral("CONNECT")
                    && protocol == QStringLiteral("webtransport")) {
                    // Store the CONNECT stream handle and ID for sending the response
                    // and for the quarter stream ID in datagrams
                    mConnectStream   = stream;
                    mConnectStreamId = streamId;

                    if (processConnectRequest(path)) {
                        sendConnectResponse(200);
                    } else {
                        sendConnectResponse(400);
                    }
                } else {
                    cerr << "WebTransportSession: Invalid request - expected CONNECT "
                            "with webtransport protocol"
                         << endl;
                    cerr << "  Got method=" << method.toStdString()
                         << ", protocol=" << protocol.toStdString() << endl;
                    // Store stream for error response too
                    mConnectStream = stream;
                    sendConnectResponse(400);
                }
            } else {
                cerr << "WebTransportSession: Failed to decode QPACK headers" << endl;
            }
        }
        break;
    }

    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        // Stream send completed - free the buffer we allocated
        if (event->SEND_COMPLETE.ClientContext) {
            QUIC_BUFFER* buffer =
                static_cast<QUIC_BUFFER*>(event->SEND_COMPLETE.ClientContext);
            if (buffer) {
                delete[] buffer->Buffer;
                delete buffer;
            }
        }
        break;

    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        if (gVerboseFlag) {
            cout << "WebTransportSession: PEER_SEND_SHUTDOWN on stream " << stream;
            if (stream == mConnectStream)
                cout << " (CONNECT stream)";
            cout << endl;
        }
        // If the CONNECT stream is shut down by peer, treat as disconnect
        if (stream == mConnectStream && mState == STATE_CONNECTED) {
            if (gVerboseFlag) {
                cout << "WebTransportSession: Client disconnected (CONNECT stream "
                        "shutdown)"
                     << endl;
            }
            close();
        }
        break;

    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        if (gVerboseFlag) {
            cout << "WebTransportSession: PEER_SEND_ABORTED on stream " << stream
                 << " (error code: " << event->PEER_SEND_ABORTED.ErrorCode << ")";
            if (stream == mConnectStream)
                cout << " (CONNECT stream)";
            cout << endl;
        }
        // If the CONNECT stream is aborted by peer, treat as disconnect
        if (stream == mConnectStream && mState == STATE_CONNECTED) {
            if (gVerboseFlag) {
                cout
                    << "WebTransportSession: Client disconnected (CONNECT stream aborted)"
                    << endl;
            }
            close();
        }
        break;

    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        if (gVerboseFlag) {
            cout << "WebTransportSession: STREAM_SHUTDOWN_COMPLETE on stream " << stream;
            if (stream == mConnectStream) {
                cout << " (CONNECT stream - THIS SHOULD NOT HAPPEN DURING ACTIVE "
                        "SESSION!)";
            }
            cout << endl;
        }
        if (mApi) {
            mApi->StreamClose(stream);
        }
        break;

    default:
        if (gVerboseFlag) {
            // Log unhandled stream events for debugging
            cout << "WebTransportSession: Unhandled stream event type: " << event->Type
                 << endl;
        }
        break;
    }

    return QUIC_STATUS_SUCCESS;
}

//*******************************************************************************
unsigned int WebTransportSession::handleInfraStreamEvent(HQUIC stream, void* eventPtr)
{
    QUIC_STREAM_EVENT* event = static_cast<QUIC_STREAM_EVENT*>(eventPtr);

    switch (event->Type) {
    case QUIC_STREAM_EVENT_START_COMPLETE: {
        // Stream is now ready - we can send data on it
        QUIC_STATUS status = event->START_COMPLETE.Status;

        if (QUIC_FAILED(status)) {
            cerr << "WebTransportSession: Infrastructure stream start failed, status: 0x"
                 << std::hex << status << std::dec << endl;
            return QUIC_STATUS_SUCCESS;
        }

        // Determine which stream this is and mark it ready
        if (stream == mControlStream) {
            mControlStreamReady = true;

            // Send stream type byte (0x00 for control)
            sendStreamType(stream, H3_STREAM_CONTROL);

            // Try to send SETTINGS if client settings already received
            if (mClientSettingsReceived && !mServerSettingsSent) {
                sendSettingsFrame();
            }
        } else if (stream == mQpackEncoderStream) {
            mQpackEncoderStreamReady = true;

            // Send stream type byte (0x02 for QPACK encoder)
            sendStreamType(stream, H3_STREAM_QPACK_ENCODER);
        } else if (stream == mQpackDecoderStream) {
            mQpackDecoderStreamReady = true;

            // Send stream type byte (0x03 for QPACK decoder)
            sendStreamType(stream, H3_STREAM_QPACK_DECODER);
        }
        break;
    }

    case QUIC_STREAM_EVENT_SEND_COMPLETE: {
        // Free the buffer we allocated for sending
        if (event->SEND_COMPLETE.ClientContext) {
            QUIC_BUFFER* buffer =
                static_cast<QUIC_BUFFER*>(event->SEND_COMPLETE.ClientContext);
            if (buffer) {
                delete[] buffer->Buffer;
                delete buffer;
            }
        }
        break;
    }

    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        break;

    case QUIC_STREAM_EVENT_PEER_SEND_ABORTED:
        cerr << "WebTransportSession: Infrastructure stream peer aborted (error: "
             << event->PEER_SEND_ABORTED.ErrorCode << ")" << endl;
        break;

    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        // Don't close the stream handle here - cleanup() will handle it
        break;

    default:
        break;
    }

    return QUIC_STATUS_SUCCESS;
}
