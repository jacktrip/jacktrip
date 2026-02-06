//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2024 Juan-Pablo Caceres, Chris Chafe.
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
 * \file JackTrip.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#include "JackTrip.h"

#ifndef NO_JACK
#include "JackAudioInterface.h"
#endif
#include "Auth.h"
#include "JitterBuffer.h"
#include "Regulator.h"
#include "RingBufferWavetable.h"
#include "UdpDataProtocol.h"
#include "jacktrip_globals.h"
#ifdef RT_AUDIO
#include "RtAudioInterface.h"
#endif
#ifdef WEBRTC_SUPPORT
#include "webrtc/WebRtcDataProtocol.h"
#endif
#ifdef WEBTRANSPORT_SUPPORT
#include "webtransport/WebTransportDataProtocol.h"
#endif

#include <QDateTime>
#include <QHostAddress>
#include <QHostInfo>
#include <QRandomGenerator>
#include <QThread>
#include <QTimer>
#include <QtEndian>
#include <QtGlobal>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
using std::setw;

using std::cerr;
using std::cout;
using std::endl;

// the following function has to remain outside the Jacktrip class definition
// its purpose is to close the app when control c is hit by the user in rtaudio/asio4all
// mode
/*if defined _WIN32
void sigint_handler(int sig)
{
    exit(0);
}
#endif*/

bool JackTrip::sSigInt       = false;
bool JackTrip::sAudioStopped = false;

//*******************************************************************************
JackTrip::JackTrip(jacktripModeT JacktripMode, dataProtocolT DataProtocolType,
                   int BaseChanIn, int NumChansIn, int BaseChanOut, int NumChansOut,
                   AudioInterface::inputMixModeT InputMixMode,
#ifdef WAIR  // WAIR
                   int NumNetRevChans,
#endif  // endwhere
                   int BufferQueueLength, unsigned int redundancy,
                   AudioInterface::audioBitResolutionT AudioBitResolution,
                   DataProtocol::packetHeaderTypeT PacketHeaderType,
                   underrunModeT UnderRunMode, int receiver_bind_port,
                   int sender_bind_port, int receiver_peer_port, int sender_peer_port,
                   int tcp_peer_port, QObject* parent)
    : QObject(parent)
    , mJackTripMode(JacktripMode)
    , mDataProtocol(DataProtocolType)
    , mPacketHeaderType(PacketHeaderType)
    , mAudiointerfaceMode(JackTrip::JACK)
    , mBaseAudioChanIn(BaseChanIn)
    , mNumAudioChansIn(NumChansIn)
    , mBaseAudioChanOut(BaseChanOut)
    , mNumAudioChansOut(NumChansOut)
    , mInputMixMode(InputMixMode)
#ifdef WAIR  // WAIR
    , mNumNetRevChans(NumNetRevChans)
#endif  // endwhere
    , mBufferQueueLength(BufferQueueLength)
    , mBufferStrategy(1)
    , mBroadcastQueueLength(0)
    , mSampleRate(gDefaultSampleRate)
    , mDeviceID(gDefaultDeviceID)
    , mAudioBufferSize(gDefaultBufferSizeInSamples)
    , mAudioBitResolution(AudioBitResolution)
    , mLoopBack(false)
    , mDataProtocolSender(NULL)
    , mDataProtocolReceiver(NULL)
    , mAudioInterface(NULL)
    , mPacketHeader(NULL)
    , mUnderRunMode(UnderRunMode)
    , mStopOnTimeout(false)
    , mSendRingBuffer(NULL)
    , mReceiveRingBuffer(NULL)
    , mReceiverBindPort(receiver_bind_port)
    , mSenderPeerPort(sender_peer_port)
    , mSenderBindPort(sender_bind_port)
    , mReceiverPeerPort(receiver_peer_port)
    , mTcpServerPort(tcp_peer_port)
    , mUseAuth(false)
    , mRedundancy(redundancy)
    , mTimeoutTimer(this)
    , mRetryTimer(this)
    , mRetries(0)
    , mSleepTime(100)
    , mElapsedTime(0)
    , mEndTime(0)
    , mTcpClient(this)
    , mUdpSockTemp(this)
    , mAwaitingUdp(false)
    , mAwaitingTcp(false)
    , mReceivedConnection(false)
    , mTcpConnectionError(false)
    , mStopped(false)
    , mHasShutdown(false)
    , mConnectDefaultAudioPorts(true)
    , mIOStatTimeout(0)
    , mIOStatLogStream(std::cout.rdbuf())
    , mStatTimer(nullptr)
    , mSimulatedLossRate(0.0)
    , mSimulatedJitterRate(0.0)
    , mSimulatedDelayRel(0.0)
    , mUseRtUdpPriority(false)
{
    createHeader(mPacketHeaderType);
    sAudioStopped = false;
}

//*******************************************************************************
JackTrip::~JackTrip()
{
    // wait();
    stop();
    delete mDataProtocolSender;
    delete mDataProtocolReceiver;
    delete mAudioInterface;
    delete mPacketHeader;
    delete mSendRingBuffer;
    delete mReceiveRingBuffer;
}

//*******************************************************************************
void JackTrip::setupAudio(
#ifdef WAIRTOHUB  // WAIR
    [[maybe_unused]] int ID
#endif  // endwhere
)
{
    // Check if mAudioInterface has already been created or not
    if (mAudioInterface != nullptr)
        return;

    // Create AudioInterface Client Object
    if (mAudiointerfaceMode == JackTrip::JACK) {
#ifndef NO_JACK
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before new JackAudioInterface"
                      << std::endl;
        QVarLengthArray<int> inputChannels;
        QVarLengthArray<int> outputChannels;
        inputChannels.resize(mNumAudioChansIn);
        outputChannels.resize(mNumAudioChansOut);
        for (int i = 0; i < mNumAudioChansIn; i++) {
            inputChannels[i] = 1 + i;
        }
        for (int i = 0; i < mNumAudioChansOut; i++) {
            outputChannels[i] = 1 + i;
        }
        mAudioInterface = new JackAudioInterface(inputChannels, outputChannels,
#ifdef WAIR  // wair
                                                 mNumNetRevChans,
#endif  // endwhere
                                                 mAudioBitResolution, true, this);

#ifdef WAIRTOHUB  // WAIR

        // Set our Jack client name if we're a hub server or a custom name hasn't been set
        if (mJackClientName.isEmpty()) {
            if (!mPeerAddress.isEmpty()) {
                mJackClientName =
                    QString(mPeerAddress).replace(QLatin1String(":"), QLatin1String("_"));
            } else {
                mJackClientName = gJackDefaultClientName;
            }
        }
        // std::cout  << "WAIR ID " << ID << " jacktrip client name set to=" <<
        //              mJackClientName.toStdString() << std::endl;

#endif  // endwhere
        mAudioInterface->setClientName(mJackClientName);
        if (0 < mBroadcastQueueLength) {
            mAudioInterface->enableBroadcastOutput();
        }

        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before mAudioInterface->setup"
                      << std::endl;
        mAudioInterface->setup(true);
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before mAudioInterface->getSampleRate"
                      << std::endl;
        mSampleRate = mAudioInterface->getSampleRate();
        if (gVerboseFlag)
            std::cout << "  JackTrip:setupAudio before mAudioInterface->getDeviceID"
                      << std::endl;
        mDeviceID = mAudioInterface->getDeviceID();
        if (gVerboseFlag)
            std::cout
                << "  JackTrip:setupAudio before mAudioInterface->getBufferSizeInSamples"
                << std::endl;
        mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
#endif          //__NON_JACK__
#ifdef NO_JACK  /// \todo FIX THIS REPETITION OF CODE
#ifdef RT_AUDIO
        cout << "Warning: using non jack version, RtAudio will be used instead" << endl;
        QVarLengthArray<int> inputChannels;
        QVarLengthArray<int> outputChannels;
        inputChannels.resize(mNumAudioChansIn);
        outputChannels.resize(mNumAudioChansOut);
        for (int i = 0; i < mNumAudioChansIn; i++) {
            inputChannels[i] = mBaseAudioChanIn + i;
        }
        for (int i = 0; i < mNumAudioChansOut; i++) {
            outputChannels[i] = mBaseAudioChanOut + i;
        }
        mAudioInterface =
            new RtAudioInterface(inputChannels, outputChannels, mInputMixMode,
                                 mAudioBitResolution, true, this);
        mAudioInterface->setSampleRate(mSampleRate);
        mAudioInterface->setDeviceID(mDeviceID);
        mAudioInterface->setInputDevice(mInputDeviceName);
        mAudioInterface->setOutputDevice(mOutputDeviceName);
        mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
        mAudioInterface->setup(true);
        // Setup might have reduced number of channels

        // TODO: Add check for if base input channel needs to change
        mNumAudioChansIn  = mAudioInterface->getNumInputChannels();
        mNumAudioChansOut = mAudioInterface->getNumOutputChannels();
        if (mNumAudioChansIn == 2 && mInputMixMode == AudioInterface::MIXTOMONO) {
            mNumAudioChansIn = 1;
        }
        // Setup might have changed buffer size
        mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
#endif
#endif
    } else if (mAudiointerfaceMode == JackTrip::RTAUDIO) {
#ifdef RT_AUDIO
        QVarLengthArray<int> inputChannels;
        QVarLengthArray<int> outputChannels;
        inputChannels.resize(mNumAudioChansIn);
        outputChannels.resize(mNumAudioChansOut);
        for (int i = 0; i < mNumAudioChansIn; i++) {
            inputChannels[i] = mBaseAudioChanIn + i;
        }
        for (int i = 0; i < mNumAudioChansOut; i++) {
            outputChannels[i] = mBaseAudioChanOut + i;
        }
        mAudioInterface =
            new RtAudioInterface(inputChannels, outputChannels, mInputMixMode,
                                 mAudioBitResolution, true, this);
        mAudioInterface->setSampleRate(mSampleRate);
        mAudioInterface->setDeviceID(mDeviceID);
        mAudioInterface->setInputDevice(mInputDeviceName);
        mAudioInterface->setOutputDevice(mOutputDeviceName);
        mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
        mAudioInterface->setup(true);
        // Setup might have reduced number of channels

        // TODO: Add check for if base input channel needs to change
        mNumAudioChansIn  = mAudioInterface->getNumInputChannels();
        mNumAudioChansOut = mAudioInterface->getNumOutputChannels();
        if (mNumAudioChansIn == 2 && mInputMixMode == AudioInterface::MIXTOMONO) {
            mNumAudioChansIn = 1;
        }
        // Setup might have changed buffer size
        mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
#endif
    }

    mAudioInterface->setLoopBack(mLoopBack);
    if (!mAudioTesterP.isNull()) {  // if we're a hub server, this will be a nullptr -
                                    // MAJOR REFACTOR NEEDED, in my opinion
        mAudioTesterP->setSampleRate(mSampleRate);
    }
    mAudioInterface->setAudioTesterP(mAudioTesterP.data());

    std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
    std::cout << gPrintSeparator << std::endl;
    int AudioBufferSizeInBytes = mAudioBufferSize * sizeof(sample_t);
    std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples"
              << std::endl;
    std::cout << "                      or: " << AudioBufferSizeInBytes << " bytes"
              << std::endl;
    if (0 < mBroadcastQueueLength) {
        std::cout << gPrintSeparator << std::endl;
        cout << "Broadcast Output is enabled, delay = "
             << mBroadcastQueueLength * mAudioBufferSize * 1000 / mSampleRate << " ms"
             << " (" << mBroadcastQueueLength * mAudioBufferSize << " samples)" << endl;
    }
    std::cout << gPrintSeparator << std::endl;
    cout << "The Number of Channels is: " << mAudioInterface->getNumInputChannels()
         << endl;
    std::cout << gPrintSeparator << std::endl;
    QThread::usleep(100);
}

//*******************************************************************************
void JackTrip::closeAudio()
{
    // mAudioInterface->close();
    if (mAudioInterface != NULL) {
        mAudioInterface->stopProcess();
        delete mAudioInterface;
        mAudioInterface = NULL;
    }
}

//*******************************************************************************
void JackTrip::setupDataProtocol()
{
    double simulated_max_delay =
        mSimulatedDelayRel * getBufferSizeInSamples() / getSampleRate();
    // Create DataProtocol Objects
    switch (mDataProtocol) {
    case UDP:
        std::cout << "Using UDP Protocol" << std::endl;
        QThread::usleep(100);
        mDataProtocolSender =
            new UdpDataProtocol(this, DataProtocol::SENDER,
                                // mSenderPeerPort, mSenderBindPort,
                                mSenderBindPort, mSenderPeerPort, mRedundancy);
        mDataProtocolReceiver =
            new UdpDataProtocol(this, DataProtocol::RECEIVER, mReceiverBindPort,
                                mReceiverPeerPort, mRedundancy);
        if (0.0 < mSimulatedLossRate || 0.0 < mSimulatedJitterRate
            || 0.0 < simulated_max_delay) {
            mDataProtocolReceiver->setIssueSimulation(
                mSimulatedLossRate, mSimulatedJitterRate, simulated_max_delay);
        }
        mDataProtocolSender->setUseRtPriority(mUseRtUdpPriority);
        mDataProtocolReceiver->setUseRtPriority(mUseRtUdpPriority);
        if (mUseRtUdpPriority) {
            cout << "Using RT thread priority for UDP data" << endl;
        }
        std::cout << gPrintSeparator << std::endl;
        break;
    case TCP:
        throw std::invalid_argument("TCP Protocol is not implemented");
        break;
    case SCTP:
        throw std::invalid_argument("SCTP Protocol is not implemented");
        break;
    case WEBRTC:
#ifdef WEBRTC_SUPPORT
        cout << "JackTrip::setupDataProtocol: Using WebRTC Data Channel Protocol" << endl;
        QThread::usleep(100);
        // Note: WebRTC data protocol requires a data channel to be set externally
        // via setWebRtcDataChannel before calling setupDataProtocol
        if (!mWebRtcDataChannel) {
            cerr << "JackTrip::setupDataProtocol: ERROR - WebRTC data channel not set!"
                 << endl;
            throw std::invalid_argument(
                "WebRTC Protocol requires data channel to be set first");
        }
        mDataProtocolSender =
            new WebRtcDataProtocol(this, DataProtocol::SENDER, mWebRtcDataChannel);
        mDataProtocolReceiver =
            new WebRtcDataProtocol(this, DataProtocol::RECEIVER, mWebRtcDataChannel);
        mDataProtocolSender->setUseRtPriority(mUseRtUdpPriority);
        mDataProtocolReceiver->setUseRtPriority(mUseRtUdpPriority);
        std::cout << gPrintSeparator << std::endl;
        break;
#else
        throw std::invalid_argument("WebRTC Protocol support not compiled in");
        break;
#endif
    case WEBTRANSPORT:
#ifdef WEBTRANSPORT_SUPPORT
        std::cout << "JackTrip::setupDataProtocol: Using WebTransport Protocol"
                  << std::endl;
        QThread::usleep(100);
        // Note: WebTransport protocol requires a session to be set externally
        // via setWebTransportSession before calling setupDataProtocol
        if (!mWebTransportSession) {
            cerr << "JackTrip::setupDataProtocol: ERROR - WebTransport session not set!"
                 << endl;
            throw std::invalid_argument(
                "WebTransport Protocol requires session to be set first");
        }
        mDataProtocolSender   = new WebTransportDataProtocol(this, DataProtocol::SENDER,
                                                             mWebTransportSession);
        mDataProtocolReceiver = new WebTransportDataProtocol(this, DataProtocol::RECEIVER,
                                                             mWebTransportSession);
        mDataProtocolSender->setUseRtPriority(mUseRtUdpPriority);
        mDataProtocolReceiver->setUseRtPriority(mUseRtUdpPriority);
        std::cout << gPrintSeparator << std::endl;
        break;
#else
        throw std::invalid_argument("WebTransport Protocol support not compiled in");
        break;
#endif
    default:
        throw std::invalid_argument("Protocol not defined or unimplemented");
        break;
    }

    // JackTrip's inputs send to the network
    mDataProtocolSender->setAudioPacketSize(getTotalAudioInputPacketSizeInBytes());

    // JackTrip's outputs receive from the network
    mDataProtocolReceiver->setAudioPacketSize(getTotalAudioOutputPacketSizeInBytes());
}

//*******************************************************************************
void JackTrip::setupRingBuffers()
{
    // Create RingBuffers with the apprioprate size
    /// \todo Make all this operations cleaner
    // int total_audio_packet_size = getTotalAudioPacketSizeInBytes();
    int audio_input_slot_size  = getInputRingBuffersSlotSize();
    int audio_output_slot_size = getOutputRingBuffersSlotSize();
    if (0 <= mBufferStrategy) {
        mUnderRunMode = ZEROS;
    } else if (0 > mBufferQueueLength) {
        throw std::invalid_argument("Auto queue is not supported by RingBuffer");
    }

    switch (mUnderRunMode) {
    case WAVETABLE:
        mSendRingBuffer =
            new RingBufferWavetable(audio_input_slot_size, gDefaultOutputQueueLength);
        mReceiveRingBuffer =
            new RingBufferWavetable(audio_output_slot_size, mBufferQueueLength);
        mPacketHeader->setBufferRequiresSameSettings(true);
        break;
    case ZEROS:
        mSendRingBuffer =
            new RingBuffer(audio_input_slot_size, gDefaultOutputQueueLength);
        if (0 > mBufferStrategy) {
            mReceiveRingBuffer =
                new RingBuffer(audio_output_slot_size, mBufferQueueLength);
            mPacketHeader->setBufferRequiresSameSettings(true);
        } else if ((mBufferStrategy == 3) || (mBufferStrategy == 4)) {
            cout << "Using Regulator buffer strategy " << mBufferStrategy << endl;
            Regulator* regulator_ptr =
                new Regulator(mNumAudioChansOut, mAudioBitResolution, mAudioBufferSize,
                              mBufferQueueLength, mBroadcastQueueLength, mSampleRate);
            mReceiveRingBuffer = regulator_ptr;
            // bufStrategy 3 or 4, mBufferQueueLength is in integer msec not packets

            mPacketHeader->setBufferRequiresSameSettings(false);  // = asym is default

            if (0 < mBroadcastQueueLength) {
                mAudioInterface->enableBroadcastOutput();
            }

        } else {
            cout << "Using JitterBuffer strategy " << mBufferStrategy << endl;
            if (0 > mBufferQueueLength) {
                cout << "Using AutoQueue 1/" << -mBufferQueueLength << endl;
            }
            mReceiveRingBuffer = new JitterBuffer(
                mAudioBufferSize, mBufferQueueLength, mSampleRate, mBufferStrategy,
                mBroadcastQueueLength, mNumAudioChansOut, mAudioBitResolution);
            static_cast<JitterBuffer*>(mReceiveRingBuffer)->setJackTrip(this);
        }
        break;
    default:
        throw std::invalid_argument("Underrun Mode undefined");
        break;
    }
}

//*******************************************************************************
void JackTrip::setPeerAddress(const QString& PeerHostOrIP)
{
    mPeerAddress = PeerHostOrIP;
}

//*******************************************************************************
void JackTrip::appendProcessPluginToNetwork(QSharedPointer<ProcessPlugin>& plugin)
{
    if (!plugin.isNull()) {
        mProcessPluginsToNetwork.append(plugin);  // ownership transferred
        // mAudioInterface->appendProcessPluginToNetwork(plugin);
    }
}

//*******************************************************************************
void JackTrip::appendProcessPluginFromNetwork(QSharedPointer<ProcessPlugin>& plugin)
{
    if (!plugin.isNull()) {
        mProcessPluginsFromNetwork.append(plugin);  // ownership transferred
        // mAudioInterface->appendProcessPluginFromNetwork(plugin);
    }
}

//*******************************************************************************
void JackTrip::appendProcessPluginToMonitor(QSharedPointer<ProcessPlugin>& plugin)
{
    if (!plugin.isNull()) {
        mProcessPluginsToMonitor.append(plugin);  // ownership transferred
        // mAudioInterface->appendProcessPluginFromNetwork(plugin);
    }
}

//*******************************************************************************
void JackTrip::startProcess(
#ifdef WAIRTOHUB  // WAIR
    int ID
#endif  // endwhere
)
{  // signal that catches ctrl c in rtaudio-asio mode
    /*#if defined (_WIN32)
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
#endif*/
    // Check if ports are already binded by another process on this machine
    // Skip this check for WebRTC and WebTransport which don't use UDP sockets
    // ------------------------------------------------------------------
    if (gVerboseFlag)
        std::cout << "step 1" << std::endl;

    if (mDataProtocol != WEBRTC && mDataProtocol != WEBTRANSPORT) {
        if (gVerboseFlag)
            std::cout
                << "  JackTrip:startProcess before checkIfPortIsBinded(mReceiverBindPort)"
                << std::endl;
        if (checkIfPortIsBinded(mReceiverBindPort)) {
            stop(QStringLiteral(
                     "Could not bind %1 UDP socket. It may already be binded by "
                     "another process on "
                     "your machine. Try using a different port number")
                     .arg(mReceiverBindPort));
            return;
        }
        if (gVerboseFlag)
            std::cout
                << "  JackTrip:startProcess before checkIfPortIsBinded(mSenderBindPort)"
                << std::endl;
        if (checkIfPortIsBinded(mSenderBindPort)) {
            stop(QStringLiteral(
                     "Could not bind %1 UDP socket. It may already be binded by "
                     "another process on "
                     "your machine. Try using a different port number")
                     .arg(mSenderBindPort));
            return;
        }
    } else {
        if (gVerboseFlag)
            std::cout << "  JackTrip:startProcess skipping port bind check for "
                      << (mDataProtocol == WEBRTC ? "WebRTC" : "WebTransport")
                      << std::endl;
    }
    // Set all classes and parameters
    // ------------------------------
    if (gVerboseFlag)
        std::cout << "  JackTrip:startProcess before setupAudio" << std::endl;
    setupAudio(
#ifdef WAIRTOHUB  // wair
        ID
#endif  // endwhere
    );

    QString audioInterfaceError =
        QString::fromStdString(mAudioInterface->getDevicesErrorMsg());
    if (audioInterfaceError != "") {
        stop(audioInterfaceError);
        return;
    }

    // AudioInterface::setup() can return a different buffer size
    // if the audio interface doesn't support the one that was requested
    if (mAudioInterface->getBufferSizeInSamples() != getBufferSizeInSamples()) {
        setAudioBufferSizeInSamples(mAudioInterface->getBufferSizeInSamples());
    }

    // cc redundant with instance creator  createHeader(mPacketHeaderType); next line
    // fixme
    createHeader(mPacketHeaderType);
    setupDataProtocol();
    setupRingBuffers();
    // Connect Signals and Slots
    // -------------------------
    QObject::connect(mPacketHeader, &PacketHeader::signalError, this,
                     &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver,
                     &DataProtocol::signalReceivedConnectionFromPeer, this,
                     &JackTrip::slotReceivedConnectionFromPeer, Qt::QueuedConnection);
    // QObject::connect(this, SIGNAL(signalUdpTimeOut()),
    //                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);

    // Connect protocol-specific signals
    if (mDataProtocol == UDP) {
        QObject::connect(static_cast<UdpDataProtocol*>(mDataProtocolReceiver),
                         &UdpDataProtocol::signalUdpWaitingTooLong, this,
                         &JackTrip::slotUdpWaitingTooLong, Qt::QueuedConnection);
    }
#ifdef WEBRTC_SUPPORT
    else if (mDataProtocol == WEBRTC) {
        QObject::connect(static_cast<WebRtcDataProtocol*>(mDataProtocolReceiver),
                         &WebRtcDataProtocol::signalWaitingTooLong, this,
                         &JackTrip::slotUdpWaitingTooLong, Qt::QueuedConnection);
    }
#endif
#ifdef WEBTRANSPORT_SUPPORT
    else if (mDataProtocol == WEBTRANSPORT) {
        QObject::connect(static_cast<WebTransportDataProtocol*>(mDataProtocolReceiver),
                         &WebTransportDataProtocol::signalWaitingTooLong, this,
                         &JackTrip::slotUdpWaitingTooLong, Qt::QueuedConnection);
    }
#endif

    QObject::connect(mDataProtocolSender, &DataProtocol::signalCeaseTransmission, this,
                     &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver, &DataProtocol::signalCeaseTransmission, this,
                     &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);

    // QObject::connect(mDataProtocolSender, SIGNAL(signalError(const char*)),
    //                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver, &DataProtocol::signalError, this,
                     &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);

    // Start the threads for the specific mode
    // ---------------------------------------
    switch (mJackTripMode) {
    case CLIENT:
        if (gVerboseFlag)
            std::cout << "step 2c client only" << std::endl;
        if (gVerboseFlag)
            std::cout << "  JackTrip:startProcess case CLIENT before clientStart"
                      << std::endl;
        clientStart();
        break;
    case SERVER:
        if (gVerboseFlag)
            std::cout << "step 2s server only" << std::endl;
        if (gVerboseFlag)
            std::cout << "  JackTrip:startProcess case SERVER before serverStart"
                      << std::endl;
        serverStart();
        break;
    case CLIENTTOPINGSERVER:
        if (gVerboseFlag)
            std::cout << "step 2C client only" << std::endl;
        if (gVerboseFlag)
            std::cout << "  JackTrip:startProcess case CLIENTTOPINGSERVER before "
                         "clientPingToServerStart"
                      << std::endl;
        if (clientPingToServerStart()
            == -1) {  // if error on server start (-1) we return immediately
            stop(QStringLiteral(
                "Peer Address has to be set if you run in CLIENTTOPINGSERVER mode"));
            return;
        }
        break;
    case SERVERPINGSERVER:
        if (gVerboseFlag)
            std::cout << "step 2S server only (same as 2s)" << std::endl;
        // For WebRTC and WebTransport, skip UDP server start and go directly to
        // connection completion
        if (mDataProtocol == WEBRTC || mDataProtocol == WEBTRANSPORT) {
            if (gVerboseFlag)
                std::cout << "  JackTrip:startProcess "
                          << (mDataProtocol == WEBRTC ? "WebRTC" : "WebTransport")
                          << " mode, skipping serverStart" << std::endl;
            // Data channel/session is already open, proceed directly to start threads
            completeConnection();
        } else {
            if (gVerboseFlag)
                std::cout
                    << "  JackTrip:startProcess case SERVERPINGSERVER before serverStart"
                    << std::endl;
            if (serverStart(true) == -1) {
                // if error on server start (-1) we return immediately
                stop();
                return;
            }
        }
        break;
    default:
        throw std::invalid_argument("Jacktrip Mode undefined");
        break;
    }
}

void JackTrip::completeConnection()
{
    // Have the threads share a single socket that operates at full duplex.
#if defined(_WIN32)
    SOCKET sock_fd = INVALID_SOCKET;
#else
    int sock_fd = -1;
#endif
    mDataProtocolReceiver->setSocket(sock_fd);
    mDataProtocolSender->setSocket(sock_fd);

    // Start Threads
    if (gVerboseFlag)
        std::cout
            << "  JackTrip::completeConnection: Starting data protocol receiver thread..."
            << std::endl;
    mDataProtocolReceiver->start();
    mDataProtocolReceiver->waitForStart();

    if (gVerboseFlag)
        std::cout
            << "  JackTrip::completeConnection: Starting data protocol sender thread..."
            << std::endl;
    mDataProtocolSender->start();
    mDataProtocolSender->waitForStart();
    /*
     * changed order so that audio starts after receiver and sender
     * because UdpDataProtocol:run0 before setRealtimeProcessPriority()
     * causes an audio hiccup from jack JackPosixSemaphore::TimedWait err = Interrupted
     */
    if (gVerboseFlag)
        std::cout << "step 5" << std::endl;
    if (gVerboseFlag)
        std::cout << "  JackTrip:startProcess before mAudioInterface->startProcess"
                  << std::endl;
    for (auto& i : mProcessPluginsFromNetwork) {
        mAudioInterface->appendProcessPluginFromNetwork(i);
    }

    for (auto& i : mProcessPluginsToNetwork) {
        mAudioInterface->appendProcessPluginToNetwork(i);
    }

    for (auto& i : mProcessPluginsToMonitor) {
        mAudioInterface->appendProcessPluginToMonitor(i);
    }

    mAudioInterface->initPlugins(true);  // mSampleRate known now, which plugins require
    mAudioInterface->startProcess();  // Tell JACK server we are ready for audio flow now

    if (mConnectDefaultAudioPorts) {
        mAudioInterface->connectDefaultPorts();
    }
    emit signalAudioStarted();

    // Start our IO stat timer
    if (mIOStatTimeout > 0) {
        cout << "Starting stat timer with interval " << mIOStatTimeout << " seconds"
             << endl;
        if (!mIOStatStream.isNull()) {
            mIOStatLogStream.rdbuf((mIOStatStream.data()->rdbuf()));
        }
        mStatTimer = new QTimer(this);
        connect(mStatTimer, &QTimer::timeout, this, &JackTrip::onStatTimer);
        mStatTimer->start(mIOStatTimeout * 1000);
    }
}

//*******************************************************************************
void JackTrip::onStatTimer()
{
    DataProtocol::PktStat pkt_stat;
    if (!mDataProtocolReceiver->getStats(&pkt_stat)) {
        return;
    }
    bool reset = (0 == pkt_stat.statCount);
    RingBuffer::IOStat recv_io_stat;
    if (!mReceiveRingBuffer->getStats(&recv_io_stat, reset)) {
        return;
    }
    RingBuffer::IOStat send_io_stat;
    if (!mSendRingBuffer->getStats(&send_io_stat, reset)) {
        return;
    }
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (!mAudioTesterP.isNull() && mAudioTesterP->getEnabled()) {
        mIOStatLogStream << "\n";
    }
    if (getBufferStrategy() != 3 && getBufferStrategy() != 4)
        mIOStatLogStream << now.toLocal8Bit().constData() << " "
                         << getPeerAddress().toLocal8Bit().constData()
                         << " send: " << send_io_stat.underruns << "/"
                         << send_io_stat.overflows << " recv: " << recv_io_stat.underruns
                         << "/" << recv_io_stat.overflows << " prot: " << pkt_stat.lost
                         << "/" << pkt_stat.outOfOrder << "/" << pkt_stat.revived
                         << " tot: " << pkt_stat.tot << " sync: " << recv_io_stat.level
                         << "/" << recv_io_stat.buf_inc_underrun << "/"
                         << recv_io_stat.buf_inc_compensate << "/"
                         << recv_io_stat.buf_dec_overflows << "/"
                         << recv_io_stat.buf_dec_pktloss << " skew: " << recv_io_stat.skew
                         << "/" << recv_io_stat.skew_raw
                         << " bcast: " << recv_io_stat.broadcast_skew << "/"
                         << recv_io_stat.broadcast_delta
                         << " autoq: " << 0.1 * recv_io_stat.autoq_corr << "/"
                         << 0.1 * recv_io_stat.autoq_rate << endl;
    else {  // bufstrategy 3 or 4
        mIOStatLogStream
            << now.toLocal8Bit().constData() << " "
            << getPeerAddress().toLocal8Bit().constData()
            << " send: " << send_io_stat.underruns << "/" << send_io_stat.overflows
            << " Glitches: " << recv_io_stat.underruns  // pullStat->lastPlcUnderruns;
#define INVFLOATFACTOR 0.001
            << "\nPUSH -- SD avg/last: " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.overflows  // pushStat->longTermStdDev;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.buf_dec_overflows  // pushStat->lastStdDev;
            << " \t mean/min/max: " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.skew  // pushStat->lastMean;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.skew_raw  // pushStat->lastMin;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.level  // pushStat->lastMax;

            << "\nPULL -- SD avg/last: " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.buf_dec_pktloss  // pullStat->longTermStdDev;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.broadcast_delta  // pullStat->lastStdDev;
            << " \t mean/min/max: " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.buf_inc_underrun  // pullStat->lastMean;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.buf_inc_compensate  // pullStat->lastMin;
            << " / " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.broadcast_skew  // pullStat->lastMax;

            //                     << "/" << recv_io_stat.overflows << " prot: " <<
            //                     pkt_stat.lost << "/"
            //                     << pkt_stat.outOfOrder << "/" << pkt_stat.revived
            << " \n tot: " << pkt_stat.tot << " \t tol: " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.autoq_corr
            << " \t latency (max): " << setw(5) << std::setprecision(3)
            << mReceiveRingBuffer->getLatency() << " \t dsp (max): " << setw(5)
            << INVFLOATFACTOR * recv_io_stat.autoq_rate
            //                     << " sync: " << recv_io_stat.level << "/"
            //                     << recv_io_stat.buf_inc_underrun << "/"
            //                     << recv_io_stat.buf_inc_compensate << "/"
            //                     << recv_io_stat.buf_dec_overflows << "/"
            //                     << recv_io_stat.buf_dec_pktloss << " skew: " <<
            //                     recv_io_stat.skew
            //                     << "/" << recv_io_stat.skew_raw
            //                     << " bcast: " << recv_io_stat.broadcast_skew << "/"
            //                     << recv_io_stat.broadcast_delta
            //                     << " autoq: " << 0.1 * recv_io_stat.autoq_corr << "/"
            //                     << 0.1 * recv_io_stat.autoq_rate
            << "\n"
            << endl;
    }
}

void JackTrip::receivedConnectionTCP()
{
    {
        QMutexLocker lock(&mTimerMutex);
        if (!mAwaitingTcp) {
            return;
        }
        mAwaitingTcp = false;
        mRetryTimer.stop();
    }
    if (gVerboseFlag)
        cout << "TCP Socket Connected to Server!" << endl;
    emit signalTcpClientConnected();

    // If we're planning to authenticate, signal the server.
    if (mUseAuth) {
        char port_buf[sizeof(qint32)];
        qToLittleEndian<qint32>(Auth::OK, port_buf);
        mTcpClient.write(port_buf, sizeof(port_buf));
        if (gVerboseFlag)
            cout << "Auth request sent to Server" << endl;
        return;
    }
    // Send Client Port Number to Server
    // ---------------------------------
    char port_buf[sizeof(qint32) + gMaxRemoteNameLength];
    qToLittleEndian<qint32>(mReceiverBindPort, port_buf);
    // std::memcpy(port_buf, &mReceiverBindPort, sizeof(mReceiverBindPort));

    std::memset(port_buf + sizeof(qint32), 0, gMaxRemoteNameLength);
    if (!mRemoteClientName.isEmpty()) {
        // If our remote client name is set, send it too.
        QByteArray name = mRemoteClientName.toUtf8();
        // Find a clean place to truncate if we're over length.
        // (Make sure we're not in the middle of a multi-byte characetr.)
        int length = name.length();
        // Need to take the final null terminator into account here.
        if (length > gMaxRemoteNameLength - 1) {
            length = gMaxRemoteNameLength - 1;
            while ((length > 0) && ((name.at(length) & 0xc0) == 0x80)) {
                // We're in the middle of a multi-byte character. Work back.
                length--;
            }
        }
        name.truncate(length);
        std::memcpy(port_buf + sizeof(qint32), name.data(), length + 1);
    }

    mTcpClient.write(port_buf, sizeof(port_buf));
    /*while ( mTcpClient.bytesToWrite() > 0 ) {
        mTcpClient.waitForBytesWritten(-1);
    }*/
    if (gVerboseFlag)
        cout << "Port " << mReceiverBindPort << " sent to Server" << endl;
    // Continued in receivedDataTCP slot
}

void JackTrip::receivedDataTCP()
{
    if (mUseAuth && !mTcpClient.isEncrypted()) {
        // If we're using authentication and haven't established a secure connection yet
        // check that our server supports it.
        qint32 authResponse;
        int size       = sizeof(authResponse);
        char* auth_buf = new char[size];
        mTcpClient.read(auth_buf, size);
        authResponse = qFromLittleEndian<qint32>(auth_buf);
        delete[] auth_buf;
        if (authResponse == Auth::OK) {
            mTcpClient.startClientEncryption();
        } else {
            if (authResponse == Auth::NOTREQUIRED) {
                std::cout << "ERROR: The Server does not require authentication."
                          << std::endl;
                stop(QStringLiteral("The server does not require authentication"));
            } else {
                std::cout << "ERROR: The Server does not support authentication."
                          << std::endl;
                stop(QStringLiteral("The server does not support authentication"));
                // Send a header sized packet to the server so we don't lock up the
                // main/UdpHubListener thread on the server. (Prevents a denial of
                // service.) TODO: This should ultimately be fixed server side, but work
                // around it here so we don't interfere with older deployments.
                if (mUdpSockTemp.bind(QHostAddress::Any, mReceiverBindPort,
                                      QUdpSocket::DefaultForPlatform)) {
                    QThread::msleep(100);
                    DefaultHeader temp(nullptr);
                    size           = temp.getHeaderSizeInBytes();
                    int8_t* header = new int8_t[size];
                    // The header doesn't need to make sense, it just has to be non-zero
                    // so we don't cause any divide by zero errors on the other end.
                    memset(header, 1, size);
                    mUdpSockTemp.writeDatagram((const char*)header, size,
                                               mTcpClient.peerAddress(), authResponse);
                    mUdpSockTemp.close();
                }
            }
            mTcpClient.close();
        }
        return;
    }

    if (mTcpClient.bytesAvailable() < (int)sizeof(qint32)) {
        return;
    }

    // Read the size of the package
    // ----------------------------
    if (gVerboseFlag)
        cout << "Reading UDP port from Server..." << endl;
    if (gVerboseFlag)
        cout << "Ready To Read From Socket!" << endl;

    // Read UDP Port Number from Server
    // --------------------------------
    uint32_t udp_port;
    int size       = sizeof(udp_port);
    char* port_buf = new char[size];
    mTcpClient.read(port_buf, size);
    udp_port = qFromLittleEndian<qint32>(port_buf);
    delete[] port_buf;
    // std::memcpy(&udp_port, port_buf, size);
    // cout << "Received UDP Port Number: " << udp_port << endl;

    // Close the TCP Socket
    // --------------------
    mTcpClient.close();  // Close the socket
    // cout << "TCP Socket Closed!" << endl;

    // If we sent authentication data, check if our authentication attempt was successful
    if (mUseAuth && udp_port > 65535) {
        QString error_message;
        if (udp_port == Auth::WRONGCREDS) {
            error_message = QStringLiteral("Incorrect username or password.");
        } else if (udp_port == Auth::WRONGTIME) {
            error_message = QStringLiteral(
                "You are not authorized to access the server at this time.");
        } else {
            error_message = QStringLiteral("Unknown authentication error.");
        }
        std::cout << "ERROR: " << error_message.toStdString() << std::endl;
        stop(error_message);
        return;
    } else if (udp_port > 65535) {
        QString error_message;
        if (udp_port == Auth::REQUIRED) {
            error_message = QStringLiteral(
                "The server you are attempting to connect to requires authentication.");
        } else {
            error_message = QStringLiteral("Unknown authentication error.");
        }
        std::cout << "ERROR: " << error_message.toStdString() << std::endl;
        stop(error_message);
        return;
    }

    if (gVerboseFlag)
        cout << "Connection Successful!" << endl;

    // Set with the received UDP port
    // ------------------------------
    setPeerPorts(udp_port);
    mDataProtocolReceiver->setPeerAddress(mPeerAddress.toLatin1().data());
    mDataProtocolSender->setPeerAddress(mPeerAddress.toLatin1().data());
    mDataProtocolSender->setPeerPort(udp_port);
    mDataProtocolReceiver->setPeerPort(udp_port);
    cout << "Server Address set to: " << mPeerAddress.toStdString()
         << " Port: " << udp_port << std::endl;
    cout << gPrintSeparator << endl;
    completeConnection();
}

void JackTrip::receivedErrorTCP(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::ConnectionRefusedError) {
        mTcpClient.close();
        mRetryTimer.stop();
        stop(QStringLiteral("TCP Socket Error: ") + QString::number(socketError));
    }
}

void JackTrip::connectionSecured()
{
    // Now that the connection is encrypted, send out port, and credentials.
    //(Remember to include an additional 2 bytes for the username and password
    // terminators.)
    QByteArray username = mUsername.toUtf8();
    QByteArray password = mPassword.toUtf8();
    int size            = (sizeof(qint32) * 3) + gMaxRemoteNameLength + username.length()
               + password.length() + 2;
    char* buf    = new char[size];
    int location = sizeof(qint32);
    std::memset(buf, 0, size);
    qToLittleEndian<qint32>(mReceiverBindPort, buf);

    if (!mRemoteClientName.isEmpty()) {
        // If our remote client name is set, send it too.
        QByteArray name = mRemoteClientName.toUtf8();
        // Find a clean place to truncate if we're over length.
        // (Make sure we're not in the middle of a multi-byte character.)
        int length = name.length();
        // Need to take the final null terminator into account here.
        if (length > gMaxRemoteNameLength - 1) {
            length = gMaxRemoteNameLength - 1;
            while ((length > 0) && ((name.at(length) & 0xc0) == 0x80)) {
                // We're in the middle of a multi-byte character. Work back.
                length--;
            }
        }
        name.truncate(length);
        std::memcpy(buf + location, name.data(), length + 1);
    }
    location += gMaxRemoteNameLength;

    qToLittleEndian<qint32>(username.length(), buf + location);
    location += sizeof(qint32);
    qToLittleEndian<qint32>(password.length(), buf + location);
    location += sizeof(qint32);

    std::memcpy(buf + location, username.data(), username.length() + 1);
    location += username.length() + 1;
    std::memcpy(buf + location, password.data(), password.length() + 1);

    mTcpClient.write(buf, size);
    if (gVerboseFlag)
        cout << "Port " << mReceiverBindPort << " sent to Server with credentials"
             << endl;
}

void JackTrip::receivedFirstPacketUDP()
{
    // Stop our timer.
    {
        QMutexLocker lock(&mTimerMutex);
        if (!mAwaitingUdp) {
            return;
        }
        mAwaitingUdp = false;
        mTimeoutTimer.stop();
    }

    QHostAddress peerHostAddress;
    uint16_t peer_port;

    // IPv6 addition from fyfe
    // Get the datagram size to avoid problems with IPv6
    qint64 datagramSize = mUdpSockTemp.pendingDatagramSize();
    char* buf           = new char[datagramSize];
    // set client address
    mUdpSockTemp.readDatagram(buf, datagramSize, &peerHostAddress, &peer_port);
    mUdpSockTemp.close();  // close the socket
    delete[] buf;
    // Check for mapped IPv4->IPv6 addresses that look like ::ffff:x.x.x.x
    if (peerHostAddress.protocol() == QAbstractSocket::IPv6Protocol) {
        bool mappedIPv4;
        uint32_t address = peerHostAddress.toIPv4Address(&mappedIPv4);
        // If the IPv4 address is mapped to IPv6, convert it to IPv4
        if (mappedIPv4) {
            QHostAddress ipv4Address = QHostAddress(address);
            mPeerAddress             = ipv4Address.toString();
        } else {
            mPeerAddress = peerHostAddress.toString();
        }
    } else {
        mPeerAddress = peerHostAddress.toString();
    }

    // Set the peer address to send packets (in the protocol sender)
    if (gVerboseFlag)
        std::cout << "JackTrip:serverStart before mDataProtocolSender->setPeerAddress()"
                  << std::endl;
    mDataProtocolSender->setPeerAddress(mPeerAddress.toLatin1().constData());
    if (gVerboseFlag)
        std::cout << "JackTrip:serverStart before mDataProtocolReceiver->setPeerAddress()"
                  << std::endl;
    mDataProtocolReceiver->setPeerAddress(mPeerAddress.toLatin1().constData());
    //     We reply to the same port the peer sent the packets from
    //     This way we can go through NAT
    //     Because of the NAT traversal scheme, the portn need to be
    //     "symmetric", e.g.:
    //     from Client to Server : src = 4474, dest = 4464
    //     from Server to Client : src = 4464, dest = 4474
    // no -- all are the same -- 4464
    if (gVerboseFlag)
        std::cout << "JackTrip:serverStart before setting all peer_port instances to "
                  << peer_port << std::endl;
    mDataProtocolSender->setPeerPort(peer_port);
    mDataProtocolReceiver->setPeerPort(peer_port);
    setPeerPorts(peer_port);
    completeConnection();
}

void JackTrip::udpTimerTick()
{
    QMutexLocker lock(&mTimerMutex);
    if (!mAwaitingUdp) {
        return;
    }

    if (mStopped || sSigInt || sAudioStopped) {
        // Stop everything.
        mAwaitingUdp = false;
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
        stop();
    }

    if (gVerboseFlag)
        std::cout << mSleepTime << "ms  " << std::flush;
    mElapsedTime += mSleepTime;
    if (mEndTime > 0 && mElapsedTime >= mEndTime) {
        mAwaitingUdp = false;
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
        cout << "JackTrip Server Timed Out!" << endl;
        stop(QStringLiteral("JackTrip Server Timed Out"));
    }
}

void JackTrip::tcpTimerTick()
{
    QMutexLocker lock(&mTimerMutex);
    if (!mAwaitingTcp) {
        return;
    }

    if (mStopped || sSigInt || sAudioStopped) {
        // Stop everything.
        mAwaitingTcp = false;
        mTcpClient.close();
        mRetryTimer.stop();
        stop();
        return;
    }

    mElapsedTime += mRetryTimer.interval();
    if (mEndTime > 0 && mElapsedTime >= mEndTime) {
        mAwaitingTcp = false;
        mTcpClient.close();
        mRetryTimer.stop();
        cout << "JackTrip Server Timed Out!" << endl;
        stop(QStringLiteral("Initial TCP Connection Timed Out"));
        return;
    }

    // Use randomized exponential backoff to reconnect the TCP client
    mRetries++;
    // exponential backoff sleep with 6s maximum + jitter
    int newInterval = 2000 * pow(2, mRetries);
    newInterval     = std::min(newInterval, 6000);
    newInterval += QRandomGenerator::global()->bounded(0, 500);
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    qDebug() << "Sleep time " << newInterval << " ms at " << now;
    mRetryTimer.setInterval(newInterval);

    qDebug() << "Connection timed out. Retrying again using exponential backoff";

    mTcpClient.abort();

    // Create Socket Objects
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(mPeerAddress)) {
        QHostInfo info = QHostInfo::fromName(mPeerAddress);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }

    if (mTcpClient.state() == QAbstractSocket::UnconnectedState) {
        mTcpClient.connectToHost(serverHostAddress, mTcpServerPort);
    }

    mRetryTimer.start();
}

//*******************************************************************************
void JackTrip::stop(const QString& errorMessage)
{
    // Take a snapshot of sAudioStopped
    bool audioStopped = sAudioStopped;
    mStopped          = true;
    // Make sure we're only run once
    if (mHasShutdown) {
        return;
    }
    mHasShutdown = true;
    std::cout << "Stopping JackTrip..." << std::endl;

    // Stop the stats timer if it's running
    if (mStatTimer != nullptr) {
        mStatTimer->stop();
    }

    if (mDataProtocolSender != nullptr) {
        // Stop The Sender
        mDataProtocolSender->stop();
        mDataProtocolSender->wait();
    }

    if (mDataProtocolReceiver != nullptr) {
        // Stop The Receiver
        mDataProtocolReceiver->stop();
        mDataProtocolReceiver->wait();
    }

    // check for errors from audio interface
    QString audioErrorMsg;
    if (mAudioInterface != nullptr && !mAudioInterface->getDevicesErrorMsg().empty()) {
        audioErrorMsg = QString::fromStdString(mAudioInterface->getDevicesErrorMsg());
    } else if (audioStopped) {
        audioErrorMsg = QStringLiteral("Your audio interface has stopped!");
    }

    // Stop the audio processes
    // mAudioInterface->stopProcess();
    closeAudio();

    cout << "JackTrip Processes STOPPED!" << endl;
    cout << gPrintSeparator << endl;

    // Emit the jack stopped signal
    if (!audioErrorMsg.isEmpty()) {
        emit signalError(audioErrorMsg);
    } else if (errorMessage.isEmpty()) {
        emit signalProcessesStopped();
    } else {
        emit signalError(errorMessage);
    }
}

//*******************************************************************************
void JackTrip::waitThreads()
{
    mDataProtocolSender->wait();
    mDataProtocolReceiver->wait();
}

//*******************************************************************************
void JackTrip::clientStart()
{
    // For the Client mode, the peer (or server) address has to be specified by the user
    if (mPeerAddress.isEmpty()) {
        throw std::invalid_argument(
            "Peer Address has to be set if you run in CLIENT mode");
    } else {
        mDataProtocolSender->setPeerAddress(mPeerAddress.toLatin1().data());
        mDataProtocolReceiver->setPeerAddress(mPeerAddress.toLatin1().data());
        cout << "Peer Address set to: " << mPeerAddress.toStdString() << std::endl;
        cout << gPrintSeparator << endl;
        completeConnection();
    }
}

//*******************************************************************************
int JackTrip::serverStart(bool timeout, int udpTimeout)  // udpTimeout unused
{
    // Set the peer address
    if (!mPeerAddress.isEmpty()) {
        if (gVerboseFlag)
            std::cout << "WARNING: SERVER mode: Peer Address was set but will be deleted."
                      << endl;
        // throw std::invalid_argument("Peer Address has to be set if you run in CLIENT
        // mode");
        mPeerAddress.clear();
        // return;
    }

    // Start timer before binding our port and waiting for datagrams
    {
        QMutexLocker lock(&mTimerMutex);
        mAwaitingUdp = true;
        mElapsedTime = 0;
        if (timeout) {
            mEndTime = udpTimeout;
        }
        mTimeoutTimer.setInterval(mSleepTime);
        mTimeoutTimer.disconnect();
        connect(&mTimeoutTimer, &QTimer::timeout, this, &JackTrip::udpTimerTick);
        mTimeoutTimer.start();
    }

    // Get the client address when it connects
    if (gVerboseFlag)
        std::cout << "JackTrip:serverStart before mUdpSockTemp.bind(Any)" << std::endl;
    // Bind the socket
    if (!mUdpSockTemp.bind(QHostAddress::Any, mReceiverBindPort,
                           QUdpSocket::DefaultForPlatform)) {
        {
            QMutexLocker lock(&mTimerMutex);
            mAwaitingUdp = false;
            mTimeoutTimer.stop();
        }
        cerr << "in JackTrip: Could not bind UDP socket. It may be already binded."
             << endl;
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    }
    connect(&mUdpSockTemp, &QUdpSocket::readyRead, this,
            &JackTrip::receivedFirstPacketUDP);

    if (gVerboseFlag)
        std::cout << "JackTrip:serverStart before !UdpSockTemp.hasPendingDatagrams()"
                  << std::endl;
    cout << "Waiting for Connection From a Client..." << endl;
    return 0;
    // Continued in the receivedFirstPacketUDP slot.

    //    char buf[1];
    //    // set client address
    //    UdpSockTemp.readDatagram(buf, 1, &peerHostAddress, &peer_port);
    //    UdpSockTemp.close(); // close the socket
}

//*******************************************************************************
int JackTrip::clientPingToServerStart()
{
    // mConnectionMode = JackTrip::KSTRONG;
    // mConnectionMode = JackTrip::JAMTEST;

    // Set Peer (server in this case) address
    // --------------------------------------
    // For the Client mode, the peer (or server) address has to be specified by the user
    if (mPeerAddress.isEmpty()) {
        throw std::invalid_argument(
            "Peer Address has to be set if you run in CLIENTTOPINGSERVER mode");
        return -1;
    }

    // If we're using authentication, check that SSL support is available.
    if (mUseAuth) {
        if (!QSslSocket::supportsSsl()) {
            QString error_message =
                "SSL not supported. Make sure you have the appropriate SSL "
                "libraries\ninstalled to enable authentication.";
            cerr << "ERROR: " << error_message.toStdString() << endl;
            stop(error_message);
            return -1;
        } else if (mUsername.isEmpty() || mPassword.isEmpty()) {
            QString error_message =
                "You must supply a username and password to authenticate with a hub "
                "server.";
            cerr << "ERROR: " << error_message.toStdString() << endl;
            stop(error_message);
            return -1;
        } else {
            // At the moment, don't verify the certificate so we can use self signed ones.
            mTcpClient.setPeerVerifyMode(QSslSocket::VerifyNone);
            QObject::connect(&mTcpClient, &QSslSocket::encrypted, this,
                             &JackTrip::connectionSecured, Qt::QueuedConnection);
        }
    }

    // Create Socket Objects
    // --------------------
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(mPeerAddress)) {
        QHostInfo info = QHostInfo::fromName(mPeerAddress);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().constFirst();
        }
    }

    // Connect Socket to Server and wait for response
    // ----------------------------------------------
    connect(&mTcpClient, &QTcpSocket::readyRead, this, &JackTrip::receivedDataTCP);
    connect(&mTcpClient, &QTcpSocket::connected, this, &JackTrip::receivedConnectionTCP);
    // Enable CI builds on Ubuntu 20.04 with Qt 5.12.8
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
    connect(&mTcpClient,
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this,
            &JackTrip::receivedErrorTCP);
#else
    connect(&mTcpClient, &QTcpSocket::errorOccurred, this, &JackTrip::receivedErrorTCP);
#endif
    {
        QMutexLocker lock(&mTimerMutex);
        mAwaitingTcp = true;
        mElapsedTime = 0;
        mEndTime     = 30000;  // Timeout after 30 seconds.
        mRetryTimer.setInterval(
            QRandomGenerator::global()->bounded(0, int(2000 * pow(2, mRetries))));
        mRetryTimer.setSingleShot(true);
        mRetryTimer.disconnect();
        connect(&mRetryTimer, &QTimer::timeout, this, &JackTrip::tcpTimerTick);
        mRetryTimer.start();
    }

    if (mTcpClient.state() == QAbstractSocket::UnconnectedState) {
        mTcpClient.connectToHost(serverHostAddress, mTcpServerPort);
    }

    if (gVerboseFlag)
        cout << "Connecting to TCP Server at "
             << serverHostAddress.toString().toLatin1().constData() << " port "
             << mTcpServerPort << "..." << endl;
    return 0;
    // Continued in the receivedConnectionTCP slot.

    /*
  else {
    // Set the peer address
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
  }

  // Start the Sender Threads
  // ------------------------
  mAudioInterface->startProcess();
  mDataProtocolSender->start();
  // block until mDataProtocolSender thread starts
  while ( !mDataProtocolSender->isRunning() ) { QThread::msleep(100); }

  // Create a Socket to listen to Server's answer
  // --------------------------------------------
  QHostAddress serverHostAddress;
  QUdpSocket UdpSockTemp;// Create socket to wait for server answer
  uint16_t server_port;

  // Bind the socket
  //bindReceiveSocket(UdpSockTemp, mReceiverBindPort,
  //                  mPeerAddress, peer_port);
  if ( !UdpSockTemp.bind(QHostAddress::Any,
                         mReceiverBindPort,
                         QUdpSocket::ShareAddress) ) {
    //throw std::runtime_error("Could not bind PingToServer UDP socket. It may be already
  binded.");
  }

  // Listen to server response
  cout << "Waiting for server response..." << endl;
  while ( !UdpSockTemp.hasPendingDatagrams() ) { QThread::msleep(100); }
  cout << "Received response from server!" << endl;
  char buf[1];
  // set client address
  UdpSockTemp.readDatagram(buf, 1, &serverHostAddress, &server_port);
  UdpSockTemp.close(); // close the socket

  // Stop the sender thread to change server port
  mDataProtocolSender->stop();
  mDataProtocolSender->wait(); // Wait for the thread to terminate

  cout << "Server port now set to: " << server_port << endl;
  cout << gPrintSeparator << endl;
  mDataProtocolSender->setPeerPort(server_port);

  // Start Threads
  //mAudioInterface->connectDefaultPorts();
  mDataProtocolSender->start();
  mDataProtocolReceiver->start();
  */
}

//*******************************************************************************
/*
void JackTrip::bindReceiveSocket(QUdpSocket& UdpSocket, int bind_port,
                                 QHostAddress PeerHostAddress, int peer_port)
throw(std::runtime_error)
{
  // Creat socket descriptor
  int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  // Set local IPv4 Address
  struct sockaddr_in local_addr;
  ::bzero(&local_addr, sizeof(local_addr));
  local_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the
active address local_addr.sin_port = htons(bind_port); //set bind port

  // Set socket to be reusable, this is platform dependent
  int one = 1;
#if defined ( __linux__ )
  ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif
#if defined ( __APPLE__ )
  // This option is not available on Linux, and without it MAC OS X
  // has problems rebinding a socket
  ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif

  // Bind the Socket
  if ( (::bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr))) < 0 )
  { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }

  // To be able to use the two UDP sockets bound to the same port number,
  // we connect the receiver and issue a SHUT_WR.
  // Set peer IPv4 Address
  struct sockaddr_in peer_addr;
  bzero(&peer_addr, sizeof(peer_addr));
  peer_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
  peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the
active address peer_addr.sin_port = htons(peer_port); //set local port
  // Connect the socket and issue a Write shutdown (to make it a
  // reader socket only)
  if ( (::inet_pton(AF_INET, PeerHostAddress.toString().toLatin1().constData(),
                    &peer_addr.sin_addr)) < 1 )
  { throw std::runtime_error("ERROR: Invalid address presentation format"); }
  if ( (::connect(sock_fd, (struct sockaddr *) &peer_addr, sizeof(peer_addr))) < 0)
  { throw std::runtime_error("ERROR: Could not connect UDP socket"); }
  if ( (::shutdown(sock_fd,SHUT_WR)) < 0)
  { throw std::runtime_error("ERROR: Could suntdown SHUT_WR UDP socket"); }

  UdpSocket.setSocketDescriptor(sock_fd, QUdpSocket::ConnectedState,
                                QUdpSocket::ReadOnly);
  cout << "UDP Socket Receiving in Port: " << bind_port << endl;
  cout << gPrintSeparator << endl;
}
*/

//*******************************************************************************
void JackTrip::createHeader(const DataProtocol::packetHeaderTypeT headertype)
{
    delete mPacketHeader;  // Just in case it has already been allocated
    switch (headertype) {
    case DataProtocol::DEFAULT:
        mPacketHeader = new DefaultHeader(this);
        break;
    case DataProtocol::JAMLINK:
        mPacketHeader = new JamLinkHeader(this);
        break;
    case DataProtocol::EMPTY:
        mPacketHeader = new EmptyHeader(this);
        break;
    default:
        throw std::invalid_argument("Undefined Header Type");
        break;
    }
}

//*******************************************************************************
void JackTrip::putHeaderInIncomingPacket(int8_t* full_packet, int8_t* audio_packet)
{
    mPacketHeader->fillHeaderCommonFromAudio();
    mPacketHeader->putHeaderInPacket(full_packet);
    if (audio_packet == nullptr)
        return;

    int8_t* audio_part;
    audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
    // std::memcpy(audio_part, audio_packet, mAudioInterface->getBufferSizeInBytes());
    // std::memcpy(audio_part, audio_packet, mAudioInterface->getSizeInBytesPerChannel() *
    // mNumChans);
    std::memcpy(audio_part, audio_packet, getTotalAudioOutputPacketSizeInBytes());
}

void JackTrip::putHeaderInOutgoingPacket(int8_t* full_packet, int8_t* audio_packet)
{
    mPacketHeader->fillHeaderCommonFromAudio();
    mPacketHeader->putHeaderInPacket(full_packet);
    if (audio_packet == nullptr)
        return;

    int8_t* audio_part;
    audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
    // std::memcpy(audio_part, audio_packet, mAudioInterface->getBufferSizeInBytes());
    // std::memcpy(audio_part, audio_packet, mAudioInterface->getSizeInBytesPerChannel() *
    // mNumChans);
    std::memcpy(audio_part, audio_packet, getTotalAudioInputPacketSizeInBytes());
}

//*******************************************************************************
int JackTrip::getSendPacketSizeInBytes() const
{
    return (getTotalAudioInputPacketSizeInBytes()
            + mPacketHeader->getHeaderSizeInBytes());
}

int JackTrip::getReceivePacketSizeInBytes() const
{
    return (getTotalAudioOutputPacketSizeInBytes()
            + mPacketHeader->getHeaderSizeInBytes());
}

//*******************************************************************************
bool JackTrip::checkPeerSettings(int8_t* full_packet)
{
    return mPacketHeader->checkPeerSettings(full_packet);
}

//*******************************************************************************
bool JackTrip::checkIfPortIsBinded(int port)
{
    QUdpSocket UdpSockTemp;  // Create socket to wait for client
    // Bind the socket
    // cc        if ( !UdpSockTemp.bind(QHostAddress::AnyIPv4, port,
    // QUdpSocket::DontShareAddress) )

    // check all combinations to ensure the port is free
    std::map<std::string, QHostAddress::SpecialAddress> interfaces = {
        {"IPv4", QHostAddress::AnyIPv4},
        {"IPv6", QHostAddress::AnyIPv6},
        {"IPv4+IPv6", QHostAddress::Any}};

    std::map<std::string, QHostAddress::SpecialAddress>::iterator it;
    for (it = interfaces.begin(); it != interfaces.end(); it++) {
        bool binded = UdpSockTemp.bind(it->second, port, QUdpSocket::DontShareAddress);
        QUdpSocket::SocketError err = UdpSockTemp.error();
        UdpSockTemp.close();
        if (!binded && err != QUdpSocket::UnsupportedSocketOperationError) {
            return true;
        }
    }
    return false;
}
