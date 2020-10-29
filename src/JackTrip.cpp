//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
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
#include "UdpDataProtocol.h"
#include "RingBufferWavetable.h"
#include "JitterBuffer.h"
#include "jacktrip_globals.h"
#include "JackAudioInterface.h"
#ifdef __RT_AUDIO__
#include "RtAudioInterface.h"
#endif

#include <iostream>
#include <cstdlib>
#include <stdexcept>

#include <QHostAddress>
#include <QHostInfo>
#include <QThread>
#include <QTimer>
#include <QDateTime>

using std::cout; using std::endl;

//the following function has to remain outside the Jacktrip class definition
//its purpose is to close the app when control c is hit by the user in rtaudio/asio4all mode
/*if defined __WIN_32__
void sigint_handler(int sig)
{
    exit(0);
}
#endif*/

bool JackTrip::sSigInt = false;
bool JackTrip::sJackStopped = false;

//*******************************************************************************
JackTrip::JackTrip(jacktripModeT JacktripMode,
                   dataProtocolT DataProtocolType,
                   int NumChans,
                   #ifdef WAIR // WAIR
                   int NumNetRevChans,
                   #endif // endwhere
                   int BufferQueueLength,
                   unsigned int redundancy,
                   AudioInterface::audioBitResolutionT AudioBitResolution,
                   DataProtocol::packetHeaderTypeT PacketHeaderType,
                   underrunModeT UnderRunMode,
                   int receiver_bind_port, int sender_bind_port,
                   int receiver_peer_port, int sender_peer_port, int tcp_peer_port) :
    mJackTripMode(JacktripMode),
    mDataProtocol(DataProtocolType),
    mPacketHeaderType(PacketHeaderType),
    mAudiointerfaceMode(JackTrip::JACK),
    mNumChans(NumChans),
    #ifdef WAIR // WAIR
    mNumNetRevChans(NumNetRevChans),
    #endif // endwhere
    mBufferQueueLength(BufferQueueLength),
    mBufferStrategy(1),
    mBroadcastQueueLength(0),
    mSampleRate(gDefaultSampleRate),
    mDeviceID(gDefaultDeviceID),
    mAudioBufferSize(gDefaultBufferSizeInSamples),
    mAudioBitResolution(AudioBitResolution),
    mLoopBack(false),
    mDataProtocolSender(NULL),
    mDataProtocolReceiver(NULL),
    mAudioInterface(NULL),
    mPacketHeader(NULL),
    mUnderRunMode(UnderRunMode),
    mStopOnTimeout(false),
    mSendRingBuffer(NULL),
    mReceiveRingBuffer(NULL),
    mReceiverBindPort(receiver_bind_port),
    mSenderPeerPort(sender_peer_port),
    mSenderBindPort(sender_bind_port),
    mReceiverPeerPort(receiver_peer_port),
    mTcpServerPort(tcp_peer_port),
    mRedundancy(redundancy),
    mJackClientName(gJackDefaultClientName),
    mConnectionMode(JackTrip::NORMAL),
    mTimeoutTimer(this),
    mSleepTime(100),
    mElapsedTime(0),
    mEndTime(0),
    mTcpClient(this),
    mUdpSockTemp(this),
    mReceivedConnection(false),
    mTcpConnectionError(false),
    mStopped(false),
    mHasShutdown(false),
    mConnectDefaultAudioPorts(true),
    mIOStatTimeout(0),
    mIOStatLogStream(std::cout.rdbuf()),
    mSimulatedLossRate(0.0),
    mSimulatedJitterRate(0.0),
    mSimulatedDelayRel(0.0),
    mUseRtUdpPriority(false),
    mAudioTesterP(nullptr)
{
    createHeader(mPacketHeaderType);
    sJackStopped = false;
}


//*******************************************************************************
JackTrip::~JackTrip()
{
    //wait();
    delete mDataProtocolSender;
    delete mDataProtocolReceiver;
    delete mAudioInterface;
    delete mPacketHeader;
    delete mSendRingBuffer;
    delete mReceiveRingBuffer;
}


//*******************************************************************************
void JackTrip::setupAudio(
        #ifdef WAIRTOHUB // WAIR
        __attribute__((unused)) int ID
        #endif // endwhere
        )
{
    // Check if mAudioInterface has already been created or not
    if (mAudioInterface != NULL)  { // if it has been created, disconnet it from JACK and delete it
        cout << "WARINING: JackAudio interface was setup already:" << endl;
        cout << "It will be erased and setup again." << endl;
        cout << gPrintSeparator << endl;
        closeAudio();
    }

    // Create AudioInterface Client Object
    if ( mAudiointerfaceMode == JackTrip::JACK ) {
#ifndef __NO_JACK__
        if (gVerboseFlag) std::cout << "  JackTrip:setupAudio before new JackAudioInterface" << std::endl;
        mAudioInterface = new JackAudioInterface(this, mNumChans, mNumChans,
                                         #ifdef WAIR // wair
                                                 mNumNetRevChans,
                                         #endif // endwhere
                                                 mAudioBitResolution);

#ifdef WAIRTOHUB // WAIR
        QString VARIABLE_AUDIO_NAME = WAIR_AUDIO_NAME; // legacy for WAIR
        //Set our Jack client name if we're a hub server or a custom name hasn't been set
        if (!mPeerAddress.isEmpty() && (mJackClientName.constData() == gJackDefaultClientName.constData())) {
            mJackClientName = QString(mPeerAddress).replace(":", "_");
        }
        //std::cout  << "WAIR ID " << ID << " jacktrip client name set to=" <<
        //              mJackClientName.toStdString() << std::endl;

#endif // endwhere
        mAudioInterface->setClientName(mJackClientName);
        if (0 < mBroadcastQueueLength) {
            mAudioInterface->enableBroadcastOutput();
        }

        if (gVerboseFlag) std::cout << "  JackTrip:setupAudio before mAudioInterface->setup" << std::endl;
        mAudioInterface->setup();
        if (gVerboseFlag) std::cout << "  JackTrip:setupAudio before mAudioInterface->getSampleRate" << std::endl;
        mSampleRate = mAudioInterface->getSampleRate();
        if (gVerboseFlag) std::cout << "  JackTrip:setupAudio before mAudioInterface->getDeviceID" << std::endl;
        mDeviceID = mAudioInterface->getDeviceID();
        if (gVerboseFlag) std::cout << "  JackTrip:setupAudio before mAudioInterface->getBufferSizeInSamples" << std::endl;
        mAudioBufferSize = mAudioInterface->getBufferSizeInSamples();
#endif //__NON_JACK__
#ifdef __NO_JACK__ /// \todo FIX THIS REPETITION OF CODE
#ifdef __RT_AUDIO__
        cout << "Warning: using non jack version, RtAudio will be used instead" << endl;
        mAudioInterface = new RtAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
        mAudioInterface->setSampleRate(mSampleRate);
        mAudioInterface->setDeviceID(mDeviceID);
        mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
        mAudioInterface->setup();
#endif
#endif
    }
    else if ( mAudiointerfaceMode == JackTrip::RTAUDIO ) {
#ifdef __RT_AUDIO__
        mAudioInterface = new RtAudioInterface(this, mNumChans, mNumChans, mAudioBitResolution);
        mAudioInterface->setSampleRate(mSampleRate);
        mAudioInterface->setDeviceID(mDeviceID);
        mAudioInterface->setBufferSizeInSamples(mAudioBufferSize);
        mAudioInterface->setup();
#endif
    }

    mAudioInterface->setLoopBack(mLoopBack);
    if (mAudioTesterP) { // if we're a hub server, this will be a nullptr - MAJOR REFACTOR NEEDED, in my opinion
      mAudioTesterP->setSampleRate(mSampleRate);
    }
    mAudioInterface->setAudioTesterP(mAudioTesterP);

    std::cout << "The Sampling Rate is: " << mSampleRate << std::endl;
    std::cout << gPrintSeparator << std::endl;
    int AudioBufferSizeInBytes = mAudioBufferSize*sizeof(sample_t);
    std::cout << "The Audio Buffer Size is: " << mAudioBufferSize << " samples" << std::endl;
    std::cout << "                      or: " << AudioBufferSizeInBytes
              << " bytes" << std::endl;
    if (0 < mBroadcastQueueLength) {
        std::cout << gPrintSeparator << std::endl;
        cout << "Broadcast Output is enabled, delay = "
             << mBroadcastQueueLength * mAudioBufferSize * 1000 / mSampleRate << " ms"
             << " (" << mBroadcastQueueLength * mAudioBufferSize << " samples)" << endl;
    }
    std::cout << gPrintSeparator << std::endl;
    cout << "The Number of Channels is: " << mAudioInterface->getNumInputChannels() << endl;
    std::cout << gPrintSeparator << std::endl;
    QThread::usleep(100);
}


//*******************************************************************************
void JackTrip::closeAudio()
{
    //mAudioInterface->close();
    if ( mAudioInterface != NULL ) {
        mAudioInterface->stopProcess();
        delete mAudioInterface;
        mAudioInterface = NULL;
    }
}


//*******************************************************************************
void JackTrip::setupDataProtocol()
{
    double simulated_max_delay = mSimulatedDelayRel * getBufferSizeInSamples() / getSampleRate();
    // Create DataProtocol Objects
    switch (mDataProtocol) {
    case UDP:
        std::cout << "Using UDP Protocol" << std::endl;
        QThread::usleep(100);
        mDataProtocolSender = new UdpDataProtocol(this, DataProtocol::SENDER,
                                                  //mSenderPeerPort, mSenderBindPort,
                                                  mSenderBindPort, mSenderPeerPort,
                                                  mRedundancy);
        mDataProtocolReceiver =  new UdpDataProtocol(this, DataProtocol::RECEIVER,
                                                     mReceiverBindPort, mReceiverPeerPort,
                                                     mRedundancy);
        if (0.0 < mSimulatedLossRate || 0.0 < mSimulatedJitterRate || 0.0 < simulated_max_delay) {
            mDataProtocolReceiver->setIssueSimulation(mSimulatedLossRate, mSimulatedJitterRate, simulated_max_delay);
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
    default:
        throw std::invalid_argument("Protocol not defined or unimplemented");
        break;
    }

    // Set Audio Packet Size
    //mDataProtocolSender->setAudioPacketSize
    //  (mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
    //mDataProtocolReceiver->setAudioPacketSize
    //  (mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
    mDataProtocolSender->setAudioPacketSize(getTotalAudioPacketSizeInBytes());
    mDataProtocolReceiver->setAudioPacketSize(getTotalAudioPacketSizeInBytes());
}


//*******************************************************************************
void JackTrip::setupRingBuffers()
{
    // Create RingBuffers with the apprioprate size
    /// \todo Make all this operations cleaner
    //int total_audio_packet_size = getTotalAudioPacketSizeInBytes();
    int slot_size = getRingBuffersSlotSize();
    if (0 <=  mBufferStrategy) {
        mUnderRunMode = ZEROS;
    }
    else if (0 > mBufferQueueLength) {
      throw std::invalid_argument("Auto queue is not supported by RingBuffer");
    }

    switch (mUnderRunMode) {
    case WAVETABLE:
        mSendRingBuffer = new RingBufferWavetable(slot_size,
                                                  gDefaultOutputQueueLength);
        mReceiveRingBuffer = new RingBufferWavetable(slot_size,
                                                     mBufferQueueLength);
        /*
    mSendRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
                gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBufferWavetable(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
             mBufferQueueLength);
             */
        break;
    case ZEROS:
        mSendRingBuffer = new RingBuffer(slot_size,
                                         gDefaultOutputQueueLength);
        if (0 > mBufferStrategy) {
            mReceiveRingBuffer = new RingBuffer(slot_size,
                                                mBufferQueueLength);
        }
        else {
            cout << "Using JitterBuffer strategy " << mBufferStrategy << endl;
            if (0 > mBufferQueueLength) {
                cout << "Using AutoQueue 1/" << -mBufferQueueLength << endl;
            }
            mReceiveRingBuffer = new JitterBuffer(mAudioBufferSize, mBufferQueueLength,
                                        mSampleRate, mBufferStrategy,
                                        mBroadcastQueueLength, mNumChans, mAudioBitResolution);
        }
        /*
    mSendRingBuffer = new RingBuffer(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
             gDefaultOutputQueueLength);
    mReceiveRingBuffer = new RingBuffer(mAudioInterface->getSizeInBytesPerChannel() * mNumChans,
          mBufferQueueLength);
          */
        break;
    default:
        throw std::invalid_argument("Underrun Mode undefined");
        break;
    }
}


//*******************************************************************************
void JackTrip::setPeerAddress(QString PeerHostOrIP)
{
    mPeerAddress = PeerHostOrIP;
}


//*******************************************************************************
void JackTrip::appendProcessPluginToNetwork(ProcessPlugin* plugin)
{
  if (plugin) {
    mProcessPluginsToNetwork.append(plugin); // ownership transferred
    //mAudioInterface->appendProcessPluginToNetwork(plugin);
  }
}

//*******************************************************************************
void JackTrip::appendProcessPluginFromNetwork(ProcessPlugin* plugin)
{
  if (plugin) {
    mProcessPluginsFromNetwork.append(plugin); // ownership transferred
    //mAudioInterface->appendProcessPluginFromNetwork(plugin);
  }
}


//*******************************************************************************
void JackTrip::startProcess(
        #ifdef WAIRTOHUB // WAIR
        int ID
        #endif // endwhere
        )
{ //signal that catches ctrl c in rtaudio-asio mode
/*#if defined (__WIN_32__)
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
#endif*/
    // Check if ports are already binded by another process on this machine
    // ------------------------------------------------------------------
    if (gVerboseFlag) std::cout << "step 1" << std::endl;

    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before checkIfPortIsBinded(mReceiverBindPort)" << std::endl;
#if defined __WIN_32__
    //cc fixed windows crash with this print statement!
    //qDebug() << "before mJackTrip->startProcess" << mReceiverBindPort<< mSenderBindPort;
#endif
    checkIfPortIsBinded(mReceiverBindPort);
    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before checkIfPortIsBinded(mSenderBindPort)" << std::endl;
    checkIfPortIsBinded(mSenderBindPort);
    // Set all classes and parameters
    // ------------------------------
    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before setupAudio" << std::endl;
    setupAudio(
            #ifdef WAIRTOHUB // wair
                ID
            #endif // endwhere
                );
    //cc redundant with instance creator  createHeader(mPacketHeaderType); next line fixme
    createHeader(mPacketHeaderType);
    setupDataProtocol();
    setupRingBuffers();
    // Connect Signals and Slots
    // -------------------------
    QObject::connect(mPacketHeader, &PacketHeader::signalError,
                     this, &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver, SIGNAL(signalReceivedConnectionFromPeer()),
                     this, SLOT(slotReceivedConnectionFromPeer()),
                     Qt::QueuedConnection);
    //QObject::connect(this, SIGNAL(signalUdpTimeOut()),
    //                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
    QObject::connect((UdpDataProtocol *)mDataProtocolReceiver, &UdpDataProtocol::signalUdpWaitingTooLong, this,
                     &JackTrip::slotUdpWaitingTooLong, Qt::QueuedConnection);
    QObject::connect(mDataProtocolSender, &DataProtocol::signalCeaseTransmission,
                     this, &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver, &DataProtocol::signalCeaseTransmission,
                     this, &JackTrip::slotStopProcessesDueToError, Qt::QueuedConnection);

    //QObject::connect(mDataProtocolSender, SIGNAL(signalError(const char*)),
    //                 this, SLOT(slotStopProcesses()), Qt::QueuedConnection);
    QObject::connect(mDataProtocolReceiver, SIGNAL(signalError(const char*)),
                     this, SLOT(slotStopProcesses()), Qt::QueuedConnection);

    // Start the threads for the specific mode
    // ---------------------------------------
    switch ( mJackTripMode )
    {
    case CLIENT :
        if (gVerboseFlag) std::cout << "step 2c client only" << std::endl;
        if (gVerboseFlag) std::cout << "  JackTrip:startProcess case CLIENT before clientStart" << std::endl;
        clientStart();
        break;
    case SERVER :
        if (gVerboseFlag) std::cout << "step 2s server only" << std::endl;
        if (gVerboseFlag) std::cout << "  JackTrip:startProcess case SERVER before serverStart" << std::endl;
        serverStart();
        break;
    case CLIENTTOPINGSERVER :
        if (gVerboseFlag) std::cout << "step 2C client only" << std::endl;
        if (gVerboseFlag) std::cout << "  JackTrip:startProcess case CLIENTTOPINGSERVER before clientPingToServerStart" << std::endl;
        if ( clientPingToServerStart() == -1 ) { // if error on server start (-1) we return inmediatly
            stop("Peer Address has to be set if you run in CLIENTTOPINGSERVER mode");
            return;
        }
        break;
    case SERVERPINGSERVER :
        if (gVerboseFlag) std::cout << "step 2S server only (same as 2s)" << std::endl;
        if (gVerboseFlag) std::cout << "  JackTrip:startProcess case SERVERPINGSERVER before serverStart" << std::endl;
        if ( serverStart(true) == -1 ) { // if error on server start (-1) we return inmediatly
            stop();
            return;
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
#if defined (__WIN_32__)
    SOCKET sock_fd = INVALID_SOCKET;
#else
    int sock_fd = -1;
#endif
    mDataProtocolReceiver->setSocket(sock_fd);
    mDataProtocolSender->setSocket(sock_fd);

    // Start Threads
    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before mDataProtocolReceiver->start" << std::endl;
    mDataProtocolReceiver->start();
    QThread::msleep(1);
    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before mDataProtocolSender->start" << std::endl;
    mDataProtocolSender->start();
    /*
     * changed order so that audio starts after receiver and sender
     * because UdpDataProtocol:run0 before setRealtimeProcessPriority()
     * causes an audio hiccup from jack JackPosixSemaphore::TimedWait err = Interrupted system call
     * new QThread::msleep(1);
     * to allow sender to start
     */
    QThread::msleep(1);
    if (gVerboseFlag) std::cout << "step 5" << std::endl;
    if (gVerboseFlag) std::cout << "  JackTrip:startProcess before mAudioInterface->startProcess" << std::endl;
    for (int i = 0; i < mProcessPluginsFromNetwork.size(); ++i) {
        mAudioInterface->appendProcessPluginFromNetwork(mProcessPluginsFromNetwork[i]);
    }
    for (int i = 0; i < mProcessPluginsToNetwork.size(); ++i) {
        mAudioInterface->appendProcessPluginToNetwork(mProcessPluginsToNetwork[i]);
    }
    mAudioInterface->initPlugins();  // mSampleRate known now, which plugins require
    mAudioInterface->startProcess(); // Tell JACK server we are ready for audio flow now

    if (mConnectDefaultAudioPorts) {  mAudioInterface->connectDefaultPorts(); }
    
    //Start our IO stat timer
    if (mIOStatTimeout > 0) {
        cout << "STATS" << mIOStatTimeout << endl;
        if (!mIOStatStream.isNull()) {
            mIOStatLogStream.rdbuf(((std::ostream *)mIOStatStream.data())->rdbuf());
        }
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(onStatTimer()));
        timer->start(mIOStatTimeout*1000);
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
    if (mAudioTesterP && mAudioTesterP->getEnabled()) {
      mIOStatLogStream << "\n";
    }
    mIOStatLogStream << now.toLocal8Bit().constData()
      << " " << getPeerAddress().toLocal8Bit().constData()
      << " send: "
      << send_io_stat.underruns
      << "/" << send_io_stat.overflows
      << " recv: "
      << recv_io_stat.underruns
      << "/" << recv_io_stat.overflows
      << " prot: "
      << pkt_stat.lost
      << "/" << pkt_stat.outOfOrder
      << "/" << pkt_stat.revived
      << " tot: "
      << pkt_stat.tot
      << " sync: "
      << recv_io_stat.level
      << "/" << recv_io_stat.buf_inc_underrun
      << "/" << recv_io_stat.buf_inc_compensate
      << "/" << recv_io_stat.buf_dec_overflows
      << "/" << recv_io_stat.buf_dec_pktloss
      << " skew: " << recv_io_stat.skew
      << "/" << recv_io_stat.skew_raw
      << " bcast: " << recv_io_stat.broadcast_skew
      << "/" << recv_io_stat.broadcast_delta
      << " autoq: " << 0.1*recv_io_stat.autoq_corr
      << "/" << 0.1*recv_io_stat.autoq_rate
      << endl;
}

void JackTrip::receivedConnectionTCP()
{
    mTimeoutTimer.stop();
    if (gVerboseFlag) cout << "TCP Socket Connected to Server!" << endl;
    emit signalTcpClientConnected();

    // Send Client Port Number to Server
    // ---------------------------------
    char port_buf[sizeof(mReceiverBindPort) + gMaxRemoteNameLength];
    std::memcpy(port_buf, &mReceiverBindPort, sizeof(mReceiverBindPort));
    std::memset(port_buf + sizeof(mReceiverBindPort), 0, gMaxRemoteNameLength);
    if (!mRemoteClientName.isEmpty()) {
        //If our remote client name is set, send it too.
        QByteArray name = mRemoteClientName.toUtf8();
        // Find a clean place to truncate if we're over length.
        // (Make sure we're not in the middle of a multi-byte characetr.)
        int length = name.length();
        //Need to take the final null terminator into account here.
        if (length > gMaxRemoteNameLength - 1) {
            length = gMaxRemoteNameLength - 1;
            while ((length > 0) && ((name.at(length) & 0xc0) == 0x80)) {
                //We're in the middle of a multi-byte character. Work back.
                length--;
            }
        }
        name.truncate(length);
        std::memcpy(port_buf + sizeof(mReceiverBindPort), name.data(), length + 1);
    }

    mTcpClient.write(port_buf, sizeof(port_buf));
    /*while ( mTcpClient.bytesToWrite() > 0 ) {
        mTcpClient.waitForBytesWritten(-1);
    }*/
    if (gVerboseFlag) cout << "Port " << mReceiverBindPort << " sent to Server" << endl;
    //Continued in receivedDataTCP slot
}

void JackTrip::receivedDataTCP()
{
    if (mTcpClient.bytesAvailable() < (int)sizeof(uint16_t)) {
        return;
    }
    
    // Read the size of the package
    // ----------------------------
    if (gVerboseFlag) cout << "Reading UDP port from Server..." << endl;
    if (gVerboseFlag) cout << "Ready To Read From Socket!" << endl;

    // Read UDP Port Number from Server
    // --------------------------------
    uint32_t udp_port;
    int size = sizeof(udp_port);
    char port_buf[sizeof(mReceiverBindPort)];
    //char port_buf[size];
    mTcpClient.read(port_buf, size);
    std::memcpy(&udp_port, port_buf, size);
    //cout << "Received UDP Port Number: " << udp_port << endl;

    // Close the TCP Socket
    // --------------------
    mTcpClient.close(); // Close the socket
    //cout << "TCP Socket Closed!" << endl;
    if (gVerboseFlag) cout << "Connection Succesfull!" << endl;

    // Set with the received UDP port
    // ------------------------------
    setPeerPorts(udp_port);
    mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().data() );
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
    mDataProtocolSender->setPeerPort(udp_port);
    mDataProtocolReceiver->setPeerPort(udp_port);
    cout << "Server Address set to: " << mPeerAddress.toStdString() << " Port: " << udp_port << std::endl;
    cout << gPrintSeparator << endl;
    completeConnection();
}

void JackTrip::receivedDataUDP()
{
    //Stop our timer.
    mTimeoutTimer.stop();
    
    QHostAddress peerHostAddress;
    uint16_t peer_port;
    
    // IPv6 addition from fyfe
    // Get the datagram size to avoid problems with IPv6
    qint64 datagramSize = mUdpSockTemp.pendingDatagramSize();
    char buf[datagramSize];
    // set client address
    mUdpSockTemp.readDatagram(buf, datagramSize, &peerHostAddress, &peer_port);
    mUdpSockTemp.close(); // close the socket

    // Check for mapped IPv4->IPv6 addresses that look like ::ffff:x.x.x.x
    if (peerHostAddress.protocol() == QAbstractSocket::IPv6Protocol) {
        bool mappedIPv4;
        uint32_t address = peerHostAddress.toIPv4Address(&mappedIPv4);
        // If the IPv4 address is mapped to IPv6, convert it to IPv4
        if (mappedIPv4) {
            QHostAddress ipv4Address = QHostAddress(address);
            mPeerAddress = ipv4Address.toString();
        } else {
            mPeerAddress = peerHostAddress.toString();
        }
    }
    else {
        mPeerAddress = peerHostAddress.toString();
    }

    // Set the peer address to send packets (in the protocol sender)
    if (gVerboseFlag) std::cout << "JackTrip:serverStart before mDataProtocolSender->setPeerAddress()" << std::endl;
    mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().constData() );
    if (gVerboseFlag) std::cout << "JackTrip:serverStart before mDataProtocolReceiver->setPeerAddress()" << std::endl;
    mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().constData() );
    //     We reply to the same port the peer sent the packets from
    //     This way we can go through NAT
    //     Because of the NAT traversal scheme, the portn need to be
    //     "symetric", e.g.:
    //     from Client to Server : src = 4474, dest = 4464
    //     from Server to Client : src = 4464, dest = 4474
    // no -- all are the same -- 4464
    if (gVerboseFlag) std::cout << "JackTrip:serverStart before setting all peer_port instances to " << peer_port << std::endl;
    mDataProtocolSender->setPeerPort(peer_port);
    mDataProtocolReceiver->setPeerPort(peer_port);
    setPeerPorts(peer_port);
    completeConnection();
}

void JackTrip::udpTimerTick()
{
    if (mStopped || sSigInt || sJackStopped) {
        //Stop everything.
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
        stop();
    }
    
    if (gVerboseFlag) std::cout << mSleepTime << "ms  " << std::flush;
    mElapsedTime += mSleepTime;
    if (mEndTime > 0 && mElapsedTime >= mEndTime) {
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
        cout << "JackTrip Server Timed Out!" << endl;
        stop("JackTrip Server Timed Out");
    }
}

void JackTrip::tcpTimerTick()
{
    if (mStopped || sSigInt || sJackStopped) {
        //Stop everything.
        mTcpClient.close();
        mTimeoutTimer.stop();
        stop();
    }
    
    mElapsedTime += mSleepTime;
    if (mEndTime > 0 && mElapsedTime >= mEndTime) {
        mTcpClient.close();
        mTimeoutTimer.stop();
        cout << "JackTrip Server Timed Out!" << endl;
        stop("Initial TCP Connection Timed Out");
    }
    
}

//*******************************************************************************
void JackTrip::stop(QString errorMessage)
{
    mStopped = true;
    //Make sure we're only run once
    if (mHasShutdown) {
        return;
    }
    mHasShutdown = true;
    std::cout << "Stopping JackTrip..." << std::endl;
    
    // Stop The Sender
    mDataProtocolSender->stop();
    mDataProtocolSender->wait();

    // Stop The Receiver
    mDataProtocolReceiver->stop();
    mDataProtocolReceiver->wait();

    // Stop the audio processes
    //mAudioInterface->stopProcess();
    closeAudio();
    
    cout << "JackTrip Processes STOPPED!" << endl;
    cout << gPrintSeparator << endl;

    // Emit the jack stopped signal
    if (sJackStopped) {
        emit signalError("The Jack Server was shut down!");
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
    if ( mPeerAddress.isEmpty() ) {
        throw std::invalid_argument("Peer Address has to be set if you run in CLIENT mode");
    }
    else {
        mDataProtocolSender->setPeerAddress( mPeerAddress.toLatin1().data() );
        mDataProtocolReceiver->setPeerAddress( mPeerAddress.toLatin1().data() );
        cout << "Peer Address set to: " << mPeerAddress.toStdString() << std::endl;
        cout << gPrintSeparator << endl;
        completeConnection();
    }
}


//*******************************************************************************
int JackTrip::serverStart(bool timeout, int udpTimeout) // udpTimeout unused
{
    // Set the peer address
    if ( !mPeerAddress.isEmpty() ) {
        if (gVerboseFlag) std::cout << "WARNING: SERVER mode: Peer Address was set but will be deleted." << endl;
        //throw std::invalid_argument("Peer Address has to be set if you run in CLIENT mode");
        mPeerAddress.clear();
        //return;
    }

    // Get the client address when it connects
    if (gVerboseFlag) std::cout << "JackTrip:serverStart before mUdpSockTemp.bind(Any)" << std::endl;
    // Bind the socket
    if ( !mUdpSockTemp.bind(QHostAddress::Any, mReceiverBindPort,
                           QUdpSocket::DefaultForPlatform) )
    {
        std::cerr << "in JackTrip: Could not bind UDP socket. It may be already binded." << endl;
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    }
    connect(&mUdpSockTemp, &QUdpSocket::readyRead, this, &JackTrip::receivedDataUDP);
    
    // Start timer and then wait for a signal to read datagrams.
    mElapsedTime = 0;
    if (timeout) {
        mEndTime = udpTimeout;
    }
    mTimeoutTimer.setInterval(mSleepTime);
    connect(&mTimeoutTimer, &QTimer::timeout, this, &JackTrip::udpTimerTick);
    mTimeoutTimer.start();
    
    if (gVerboseFlag) std::cout << "JackTrip:serverStart before !UdpSockTemp.hasPendingDatagrams()" << std::endl;
    cout << "Waiting for Connection From a Client..." << endl;
    return 0;
    // Continued in the receivedDataUDP slot.

    //    char buf[1];
    //    // set client address
    //    UdpSockTemp.readDatagram(buf, 1, &peerHostAddress, &peer_port);
    //    UdpSockTemp.close(); // close the socket
}


//*******************************************************************************
int JackTrip::clientPingToServerStart()
{
    //mConnectionMode = JackTrip::KSTRONG;
    //mConnectionMode = JackTrip::JAMTEST;

    // Set Peer (server in this case) address
    // --------------------------------------
    // For the Client mode, the peer (or server) address has to be specified by the user
    if ( mPeerAddress.isEmpty() ) {
        throw std::invalid_argument("Peer Address has to be set if you run in CLIENTTOPINGSERVER mode");
        return -1;
    }

    // Create Socket Objects
    // --------------------
    QHostAddress serverHostAddress;
    if (!serverHostAddress.setAddress(mPeerAddress)) {
        QHostInfo info = QHostInfo::fromName(mPeerAddress);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            serverHostAddress = info.addresses().first();
        }
    }

    // Connect Socket to Server and wait for response
    // ----------------------------------------------
    connect(&mTcpClient, &QTcpSocket::readyRead, this, &JackTrip::receivedDataTCP);
    connect(&mTcpClient, &QTcpSocket::connected, this, &JackTrip::receivedConnectionTCP);
    mElapsedTime = 0;
    mEndTime = 5000; //Timeout after 5 seconds.
    mTimeoutTimer.setInterval(mSleepTime);
    connect(&mTimeoutTimer, &QTimer::timeout, this, &JackTrip::tcpTimerTick);
    mTimeoutTimer.start();
    mTcpClient.connectToHost(serverHostAddress, mTcpServerPort);
    
    if (gVerboseFlag) cout << "Connecting to TCP Server at " <<  serverHostAddress.toString().toLatin1().constData() << " port " << mTcpServerPort << "..." << endl;
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
    //throw std::runtime_error("Could not bind PingToServer UDP socket. It may be already binded.");
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
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
  local_addr.sin_port = htons(bind_port); //set bind port

  // Set socket to be reusable, this is platform dependent
  int one = 1;
#if defined ( __LINUX__ )
  ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#endif
#if defined ( __MAC_OSX__ )
  // This option is not avialable on Linux, and without it MAC OS X
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
  peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
  peer_addr.sin_port = htons(peer_port); //set local port
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
    delete mPacketHeader; //Just in case it has already been allocated
    switch (headertype) {
    case DataProtocol::DEFAULT :
        mPacketHeader = new DefaultHeader(this);
        break;
    case DataProtocol::JAMLINK :
        mPacketHeader = new JamLinkHeader(this);
        break;
    case DataProtocol::EMPTY :
        mPacketHeader = new EmptyHeader(this);
        break;
    default :
        throw std::invalid_argument("Undefined Header Type");
        break;
    }
}


//*******************************************************************************
void JackTrip::putHeaderInPacket(int8_t* full_packet, int8_t* audio_packet)
{
    mPacketHeader->fillHeaderCommonFromAudio();
    mPacketHeader->putHeaderInPacket(full_packet);

    int8_t* audio_part;
    audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
    //std::memcpy(audio_part, audio_packet, mAudioInterface->getBufferSizeInBytes());
    //std::memcpy(audio_part, audio_packet, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
    std::memcpy(audio_part, audio_packet, getTotalAudioPacketSizeInBytes());
}


//*******************************************************************************
int JackTrip::getPacketSizeInBytes()
{
    //return (mAudioInterface->getBufferSizeInBytes() + mPacketHeader->getHeaderSizeInBytes());
    //return (mAudioInterface->getSizeInBytesPerChannel() * mNumChans  +
    //mPacketHeader->getHeaderSizeInBytes());
    return (getTotalAudioPacketSizeInBytes()  +
            mPacketHeader->getHeaderSizeInBytes());
}


//*******************************************************************************
void JackTrip::parseAudioPacket(int8_t* full_packet, int8_t* audio_packet)
{
    int8_t* audio_part;
    audio_part = full_packet + mPacketHeader->getHeaderSizeInBytes();
    //std::memcpy(audio_packet, audio_part, mAudioInterface->getBufferSizeInBytes());
    //std::memcpy(audio_packet, audio_part, mAudioInterface->getSizeInBytesPerChannel() * mNumChans);
    std::memcpy(audio_packet, audio_part, getTotalAudioPacketSizeInBytes());
}

//*******************************************************************************
void JackTrip::checkPeerSettings(int8_t* full_packet)
{
    mPacketHeader->checkPeerSettings(full_packet);
}


//*******************************************************************************
void JackTrip::checkIfPortIsBinded(int port)
{
    QUdpSocket UdpSockTemp;// Create socket to wait for client
    // Bind the socket
    //cc        if ( !UdpSockTemp.bind(QHostAddress::AnyIPv4, port, QUdpSocket::DontShareAddress) )
    if ( !UdpSockTemp.bind(QHostAddress::Any, port,
                           QUdpSocket::DontShareAddress) )
    {
        UdpSockTemp.close(); // close the socket
        throw std::runtime_error(
                    "Could not bind UDP socket. It may already be binded by another process on your machine. Try using a different port number");
    }
    UdpSockTemp.close(); // close the socket
}
