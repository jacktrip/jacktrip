//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
SoundWIRE group at CCRMA, Stanford University.
*/
//*****************************************************************

/**
 * \file PipewireAudioInterface.cpp
 * \author Chris Chafe
 * \date March 2026
 */

#include "PipewireAudioInterface.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "JackTrip.h"
#include "jacktrip_globals.h"

#include <QMutexLocker>
#include <QTextStream>

using std::cout;
using std::endl;

// sPWMutex definition
QMutex PipewireAudioInterface::sPWMutex;
int PipewireAudioInterface::sClientNumber = 0;

//*******************************************************************************
PipewireAudioInterface::PipewireAudioInterface(
    QVarLengthArray<int> InputChans, QVarLengthArray<int> OutputChans,
#ifdef WAIR
    int NumNetRevChans,
#endif
    AudioInterface::audioBitResolutionT AudioBitResolution, bool processWithNetwork,
    JackTrip* jacktrip, const QString& ClientName)
    : AudioInterface(InputChans, OutputChans, MIX_UNSET,
#ifdef WAIR
                     NumNetRevChans,
#endif
                     AudioBitResolution, processWithNetwork, jacktrip)
    , mLoop(nullptr)
    , mContext(nullptr)
    , mInputStream(nullptr)
    , mOutputStream(nullptr)
    , mClientName(ClientName)
    , mSampleRate(gDefaultSampleRate)
    , mBufferSize(gDefaultBufferSizeInSamples)
    , mNumFrames(0)
    , mStopped(false)
{
    // Handle empty client name
    if (mClientName.isEmpty()) {
        mClientName = QString("JackTrip");
        std::cerr << "Warning: Empty client name provided, using default 'JackTrip'" << std::endl;
    }

    std::cerr << "PipewireAudioInterface constructor called with ClientName: '"
              << mClientName.toStdString() << "'" << std::endl;
}

//*******************************************************************************
PipewireAudioInterface::~PipewireAudioInterface()
{
    std::cerr << "PipewireAudioInterface destructor called for: "
              << mAssignedClientName.toStdString() << std::endl;
    stopProcess();
}

//*******************************************************************************
void PipewireAudioInterface::setup(bool verbose)
{
    setupClient();
    AudioInterface::setup(verbose);

    if (verbose) {
        std::cerr << "PipeWire setup complete" << std::endl;
    }
}

//*******************************************************************************
void PipewireAudioInterface::setupClient()
{
    // Initialize PipeWire
    pw_init(nullptr, nullptr);

            // Ensure we have a valid client name
    if (mClientName.isEmpty()) {
        mClientName = QString("JackTrip");
    }

    // Try to get a more specific name from the JackTrip object if available
    if (mJackTrip != nullptr && mClientName == "JackTrip") {
        // Try to get peer address or other identifying information
        QString peerAddress = mJackTrip->getPeerAddress();
        if (!peerAddress.isEmpty()) {
            // Clean up IPv6-mapped IPv4 addresses (::ffff:192.168.1.137 -> 192.168.1.137)
            if (peerAddress.startsWith("::ffff:")) {
                peerAddress = peerAddress.mid(7);
            }
            mClientName = peerAddress;
            std::cerr << "Using peer address as client name: " << mClientName.toStdString() << std::endl;
        }
    }

            // Create thread loop
    QByteArray clientName = mClientName.toUtf8();
    mLoop = pw_thread_loop_new(clientName.constData(), nullptr);
    if (!mLoop) {
        throw std::runtime_error("Failed to create PipeWire thread loop");
    }

            // Create context
    mContext = pw_context_new(pw_thread_loop_get_loop(mLoop), nullptr, 0);
    if (!mContext) {
        pw_thread_loop_destroy(mLoop);
        mLoop = nullptr;
        throw std::runtime_error("Failed to create PipeWire context");
    }

            // Get sample rate and buffer size before creating stream
    mSampleRate = getSampleRate();
    mBufferSize = getBufferSizeInSamples();

            // Create input and output streams
    createStreams();

            // Initialize buffer arrays (same as JACK)
    mInBuffer.resize(getNumInputChannels());
    mOutBuffer.resize(getNumOutputChannels());
    mNumFrames = mBufferSize;

    std::cerr << "PipeWire client setup complete: " << mAssignedClientName.toStdString()
              << " (" << mSampleRate << " Hz, " << mBufferSize << " frames)" << std::endl;
}

//*******************************************************************************
void PipewireAudioInterface::createStreams()
{
    // Generate unique client name (similar to JACK's behavior)
    QString uniqueClientName = mClientName;
    QByteArray clientNameBytes = uniqueClientName.toUtf8();

    // Add a counter suffix if needed for uniqueness
    // This handles the case where multiple connections come from the same IP
    {
        QMutexLocker locker(&sPWMutex);
        // Only add suffix if it's the default name or we've seen this name before
        if (mClientName == "JackTrip" && sClientNumber > 0) {
            uniqueClientName = QString("%1-%2").arg(mClientName).arg(sClientNumber);
        }
        sClientNumber++;
    }

    mAssignedClientName = uniqueClientName;
    clientNameBytes = uniqueClientName.toUtf8();

    std::cerr << "Creating streams with base name: '" << uniqueClientName.toStdString() << "'" << std::endl;

    // Create INPUT stream - This is PW_DIRECTION_INPUT (capture)
    // JackTrip reads from this (gets audio from mic to send over network)
    QString inputStreamName = uniqueClientName + QString("_send");
    QByteArray inputNameBytes = inputStreamName.toUtf8();

    struct pw_properties* input_props = pw_properties_new(
        PW_KEY_APP_NAME, "JackTrip",
        PW_KEY_NODE_NAME, inputNameBytes.constData(),
        PW_KEY_NODE_DESCRIPTION, inputStreamName.toUtf8().constData(),
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Capture",
        PW_KEY_MEDIA_ROLE, "Music",
        PW_KEY_MEDIA_SOFTWARE, "JackTrip",
        PW_KEY_NODE_GROUP, clientNameBytes.constData(),
        nullptr);

    mInputStream = pw_stream_new_simple(
        pw_thread_loop_get_loop(mLoop),
        inputNameBytes.constData(),
        input_props,
        &sInputEvents,
        this);

    if (!mInputStream) {
        throw std::runtime_error("Failed to create input stream");
    }

            // Create OUTPUT stream - This is PW_DIRECTION_OUTPUT (playback)
            // JackTrip writes to this (sends received network audio to speakers)
    QString outputStreamName = uniqueClientName + QString("_receive");
    QByteArray outputNameBytes = outputStreamName.toUtf8();

    struct pw_properties* output_props = pw_properties_new(
        PW_KEY_APP_NAME, "JackTrip",
        PW_KEY_NODE_NAME, outputNameBytes.constData(),
        PW_KEY_NODE_DESCRIPTION, outputStreamName.toUtf8().constData(),
        PW_KEY_MEDIA_TYPE, "Audio",
        PW_KEY_MEDIA_CATEGORY, "Playback",
        PW_KEY_MEDIA_ROLE, "Music",
        PW_KEY_MEDIA_SOFTWARE, "JackTrip",
        PW_KEY_NODE_GROUP, clientNameBytes.constData(),
        nullptr);

    mOutputStream = pw_stream_new_simple(
        pw_thread_loop_get_loop(mLoop),
        outputNameBytes.constData(),
        output_props,
        &sOutputEvents,
        this);

    if (!mOutputStream) {
        pw_stream_destroy(mInputStream);
        mInputStream = nullptr;
        throw std::runtime_error("Failed to create output stream");
    }

    std::cerr << "Streams created: " << inputStreamName.toStdString()
              << " and " << outputStreamName.toStdString() << std::endl;
}

//*******************************************************************************
uint32_t PipewireAudioInterface::getSampleRate() const
{
    // Return configured or default sample rate
    // PipeWire will negotiate the actual rate
    return mSampleRate;
}

//*******************************************************************************
uint32_t PipewireAudioInterface::getBufferSizeInSamples() const
{
    return mBufferSize;
}

//*******************************************************************************
size_t PipewireAudioInterface::getSizeInBytesPerChannel() const
{
    return (getBufferSizeInSamples() * getAudioBitResolution() / 8);
}

//*******************************************************************************
int PipewireAudioInterface::startProcess()
{
    std::cout << "Setting PipeWire Process Callback..." << std::endl;

    // Set up audio format
    uint8_t buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));

            // INPUT stream (capture - what we send over network)
    struct spa_audio_info_raw input_info;
    spa_zero(input_info);
    input_info.format = SPA_AUDIO_FORMAT_F32;
    input_info.channels = getNumInputChannels();
    input_info.rate = mSampleRate;

    const struct spa_pod* params[1];
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &input_info);

    int ret = pw_stream_connect(mInputStream,
                                PW_DIRECTION_INPUT,
                                PW_ID_ANY,
                                static_cast<pw_stream_flags>(
                                    PW_STREAM_FLAG_AUTOCONNECT |
                                    PW_STREAM_FLAG_MAP_BUFFERS |
                                    PW_STREAM_FLAG_RT_PROCESS),
                                params, 1);

    if (ret < 0) {
        std::cerr << "Failed to connect input stream: " << strerror(-ret) << std::endl;
        return ret;
    }

            // OUTPUT stream (playback - what we receive from network)
    b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    struct spa_audio_info_raw output_info;
    spa_zero(output_info);
    output_info.format = SPA_AUDIO_FORMAT_F32;
    output_info.channels = getNumOutputChannels();
    output_info.rate = mSampleRate;

    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &output_info);

    ret = pw_stream_connect(mOutputStream,
                            PW_DIRECTION_OUTPUT,
                            PW_ID_ANY,
                            static_cast<pw_stream_flags>(
                                PW_STREAM_FLAG_AUTOCONNECT |
                                PW_STREAM_FLAG_MAP_BUFFERS |
                                PW_STREAM_FLAG_RT_PROCESS),
                            params, 1);

    if (ret < 0) {
        std::cerr << "Failed to connect output stream: " << strerror(-ret) << std::endl;
        return ret;
    }

            // Start the thread loop
    if (pw_thread_loop_start(mLoop) < 0) {
        std::cerr << "Failed to start thread loop" << std::endl;
        return -1;
    }

    std::cout << "SUCCESS" << std::endl;
    std::cout << gPrintSeparator << std::endl;
    return 0;
}

//*******************************************************************************
int PipewireAudioInterface::stopProcess()
{
    QMutexLocker locker(&sPWMutex);

    // Check if already stopped to prevent double cleanup
    if (mStopped) {
        std::cerr << "PipewireAudioInterface::stopProcess() already called, skipping cleanup" << std::endl;
        return 0;
    }

    mStopped = true;

    std::cerr << "PipewireAudioInterface::stopProcess() called for: "
              << mAssignedClientName.toStdString() << std::endl;

    // Stop the thread loop first to prevent callbacks during cleanup
    if (mLoop) {
        std::cerr << "  Stopping thread loop..." << std::endl;
        pw_thread_loop_stop(mLoop);
    }

            // Disconnect and destroy streams
    if (mInputStream) {
        std::cerr << "  Disconnecting and destroying input stream..." << std::endl;
        pw_stream_disconnect(mInputStream);
        pw_stream_destroy(mInputStream);
        mInputStream = nullptr;
    }

    if (mOutputStream) {
        std::cerr << "  Disconnecting and destroying output stream..." << std::endl;
        pw_stream_disconnect(mOutputStream);
        pw_stream_destroy(mOutputStream);
        mOutputStream = nullptr;
    }

            // Destroy context
    if (mContext) {
        std::cerr << "  Destroying context..." << std::endl;
        pw_context_destroy(mContext);
        mContext = nullptr;
    }

            // Destroy thread loop
    if (mLoop) {
        std::cerr << "  Destroying thread loop..." << std::endl;
        pw_thread_loop_destroy(mLoop);
        mLoop = nullptr;
    }

            // Deinit PipeWire
    pw_deinit();

    std::cerr << "PipeWire streams stopped for: " << mAssignedClientName.toStdString() << std::endl;
    return 0;
}

//*******************************************************************************
void PipewireAudioInterface::connectDefaultPorts()
{
    // With PW_STREAM_FLAG_AUTOCONNECT, this happens automatically
    std::cerr << "PipeWire auto-connect enabled" << std::endl;
}

//*******************************************************************************
// Input stream callbacks
//*******************************************************************************
void PipewireAudioInterface::onInputStateChanged(void* data, enum pw_stream_state old,
                                                 enum pw_stream_state state,
                                                 const char* error)
{
    PipewireAudioInterface* self = static_cast<PipewireAudioInterface*>(data);

    if (!self) {
        std::cerr << "*** Input state changed with null self pointer!" << std::endl;
        return;
    }

    std::cerr << "Input stream state: " << pw_stream_state_as_string(old)
              << " -> " << pw_stream_state_as_string(state) << std::endl;

    if (error) {
        std::cerr << "Input stream error: " << error << std::endl;
        if (self->mErrorCallback) {
            self->mErrorCallback(error);
        }
    }
}

//*******************************************************************************
void PipewireAudioInterface::onInputParamChanged(void* data, uint32_t id,
                                                 const struct spa_pod* param)
{
    PipewireAudioInterface* self = static_cast<PipewireAudioInterface*>(data);

    if (!self) return;

    if (param == nullptr || id != SPA_PARAM_Format) {
        return;
    }

    struct spa_audio_info_raw info;
    if (spa_format_audio_raw_parse(param, &info) < 0) {
        return;
    }

    std::cerr << "Input format: " << info.rate << " Hz, "
              << info.channels << " channels" << std::endl;

    self->mSampleRate = info.rate;
}

//*******************************************************************************
void PipewireAudioInterface::onInputProcess(void* data)
{
    PipewireAudioInterface* self = static_cast<PipewireAudioInterface*>(data);

    if (!self || !self->mInputStream) {
        return;
    }

    if (self->mProcessingAudio) {
        std::cerr << "*** PipewireAudioInterface.cpp: DROPPED INPUT BUFFER\n";
        return;
    }

    struct pw_buffer* buf = pw_stream_dequeue_buffer(self->mInputStream);
    if (!buf) {
        return;
    }

    struct spa_buffer* spa_buf = buf->buffer;

    int stride = sizeof(float) * self->getNumInputChannels();
    uint32_t nframes = spa_buf->datas[0].chunk->size / stride;

    if (nframes != self->mNumFrames) {
        nframes = std::min(nframes, self->mNumFrames);
    }

    float* data_ptr = static_cast<float*>(spa_buf->datas[0].data);
    if (!data_ptr) {
        pw_stream_queue_buffer(self->mInputStream, buf);
        return;
    }

            // Store input buffer pointers (interleaved)
    for (int i = 0; i < self->getNumInputChannels(); i++) {
        self->mInBuffer[i] = data_ptr + i;
    }

            // Call input callback
    self->audioInputCallback(self->mInBuffer, nframes);

    pw_stream_queue_buffer(self->mInputStream, buf);
}

//*******************************************************************************
// Output stream callbacks
//*******************************************************************************
void PipewireAudioInterface::onOutputStateChanged(void* data, enum pw_stream_state old,
                                                  enum pw_stream_state state,
                                                  const char* error)
{
    PipewireAudioInterface* self = static_cast<PipewireAudioInterface*>(data);

    if (!self) {
        std::cerr << "*** Output state changed with null self pointer!" << std::endl;
        return;
    }

    std::cerr << "Output stream state: " << pw_stream_state_as_string(old)
              << " -> " << pw_stream_state_as_string(state) << std::endl;

    if (error) {
        std::cerr << "Output stream error: " << error << std::endl;
        if (self->mErrorCallback) {
            self->mErrorCallback(error);
        }
    }
}

//*******************************************************************************
void PipewireAudioInterface::onOutputParamChanged(void* data, uint32_t id,
                                                  const struct spa_pod* param)
{
    if (!data) return;

    if (param == nullptr || id != SPA_PARAM_Format) {
        return;
    }

    struct spa_audio_info_raw info;
    if (spa_format_audio_raw_parse(param, &info) < 0) {
        return;
    }

    std::cerr << "Output format: " << info.rate << " Hz, "
              << info.channels << " channels" << std::endl;
}

//*******************************************************************************
void PipewireAudioInterface::onOutputProcess(void* data)
{
    PipewireAudioInterface* self = static_cast<PipewireAudioInterface*>(data);

    if (!self || !self->mOutputStream) {
        return;
    }

    struct pw_buffer* buf = pw_stream_dequeue_buffer(self->mOutputStream);
    if (!buf) {
        return;
    }

    struct spa_buffer* spa_buf = buf->buffer;

    int stride = sizeof(float) * self->getNumOutputChannels();
    uint32_t nframes = spa_buf->datas[0].maxsize / stride;

    if (nframes != self->mNumFrames) {
        nframes = std::min(nframes, self->mNumFrames);
    }

    float* data_ptr = static_cast<float*>(spa_buf->datas[0].data);
    if (!data_ptr) {
        pw_stream_queue_buffer(self->mOutputStream, buf);
        return;
    }

            // Store output buffer pointers (interleaved)
    for (int i = 0; i < self->getNumOutputChannels(); i++) {
        self->mOutBuffer[i] = data_ptr + i;
    }

            // Call output callback
    self->audioOutputCallback(self->mOutBuffer, nframes);

            // Set buffer metadata
    spa_buf->datas[0].chunk->offset = 0;
    spa_buf->datas[0].chunk->stride = stride;
    spa_buf->datas[0].chunk->size = nframes * stride;

    pw_stream_queue_buffer(self->mOutputStream, buf);
}

//*******************************************************************************
int PipewireAudioInterface::processCallback(uint32_t /*nframes*/)
{
    // This wrapper exists for API compatibility but isn't used directly
    // PipeWire calls onInputProcess/onOutputProcess instead
    return 0;
}

//*******************************************************************************
// Event structures
//*******************************************************************************
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

const struct pw_stream_events PipewireAudioInterface::sInputEvents = {
    .version = PW_VERSION_STREAM_EVENTS,
    .destroy = nullptr,
    .state_changed = onInputStateChanged,
    .control_info = nullptr,
    .io_changed = nullptr,
    .param_changed = onInputParamChanged,
    .add_buffer = nullptr,
    .remove_buffer = nullptr,
    .process = onInputProcess,
    .drained = nullptr,
    .command = nullptr,
    .trigger_done = nullptr,
};

const struct pw_stream_events PipewireAudioInterface::sOutputEvents = {
    .version = PW_VERSION_STREAM_EVENTS,
    .destroy = nullptr,
    .state_changed = onOutputStateChanged,
    .control_info = nullptr,
    .io_changed = nullptr,
    .param_changed = onOutputParamChanged,
    .add_buffer = nullptr,
    .remove_buffer = nullptr,
    .process = onOutputProcess,
    .drained = nullptr,
    .command = nullptr,
    .trigger_done = nullptr,
};

#pragma GCC diagnostic pop
