//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file JackTrip.h
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#ifndef __JACKTRIP_H__
#define __JACKTRIP_H__

//#include <tr1/memory> //for shared_ptr
#include <QObject>
#include <QSharedPointer>
#include <QSslSocket>
#include <QString>
#include <QTimer>
#include <QUdpSocket>
#include <stdexcept>

#include "AudioInterface.h"
#include "DataProtocol.h"

#ifndef NO_JACK
#include "JackAudioInterface.h"
#endif  //NO_JACK

#include "AudioTester.h"
#include "PacketHeader.h"
#include "RingBuffer.h"

//#include <signal.h>
/** \brief Main class to creates a SERVER (to listen) or a CLIENT (to connect
 * to a listening server) to send audio streams in the network.
 *
 * All audio and network settings can be set in this class.
 * This class also acts as a Mediator between all the other class.
 * Classes that uses JackTrip methods need to register with it.
 */

class JackTrip : public QObject
{
    Q_OBJECT;

   public:
    //----------ENUMS------------------------------------------
    /// \brief Enum for the data Protocol. At this time only UDP is implemented
    enum dataProtocolT {
        UDP,  ///< Use UDP (User Datagram Protocol)
        TCP,  ///< <B>NOT IMPLEMENTED</B>: Use TCP (Transmission Control Protocol)
        SCTP  ///< <B>NOT IMPLEMENTED</B>: Use SCTP (Stream Control Transmission Protocol)
    };

    /// \brief Enum for the JackTrip mode
    enum jacktripModeT {
        SERVER,              ///< Run in P2P Server Mode
        CLIENT,              ///< Run in P2P Client Mode
        CLIENTTOPINGSERVER,  ///< Client of the Ping Server Mode
        SERVERPINGSERVER     ///< Server of the MultiThreaded JackTrip
    };

    /// \brief Enum for the JackTrip Underrun Mode, when packets
    enum underrunModeT {
        WAVETABLE,  ///< Loops on the last received packet
        ZEROS       ///< Set new buffers to zero if there are no new ones
    };

    /// \brief Enum for Audio Interface Mode
    enum audiointerfaceModeT {
        JACK,    ///< Jack Mode
        RTAUDIO  ///< RtAudio Mode
    };

    /// \brief Enum for Connection Mode (in packet header)
    enum connectionModeT {
        NORMAL,   ///< Normal Mode
        KSTRONG,  ///< Karplus Strong
        JAMTEST   ///< Karplus Strong
    };

    /// \brief Enum for Hub Server Audio Connection Mode (connections to hub server are
    /// automatically patched in Jack)
    enum hubConnectionModeT {
        SERVERTOCLIENT,  ///< Normal Mode, Sever to All Clients (but not client to any
                         ///< client)
        CLIENTECHO,      ///< Client Echo (client self-to-self)
        CLIENTFOFI,      ///< Client Fan Out to Clients and Fan In from Clients (but not
                         ///< self-to-self)
        RESERVEDMATRIX,  ///< Reserved for custom patch matrix (for TUB ensemble)
        FULLMIX,         ///< Client Fan Out to Clients and Fan In from Clients (including
                         ///< self-to-self)
        NOAUTO           ///< No automatic patching
    };
    //---------------------------------------------------------

    /** \brief The class Constructor with Default Parameters
     * \param JacktripMode JackTrip::CLIENT or JackTrip::SERVER
     * \param DataProtocolType JackTrip::dataProtocolT
     * \param NumChans Number of Audio Channels (same for inputs and outputs)
     * \param BufferQueueLength Audio Buffer for receiving packets
     * \param AudioBitResolution Audio Sample Resolutions in bits
     * \param redundancy redundancy factor for network data
     */
    JackTrip(
        jacktripModeT JacktripMode = CLIENT, dataProtocolT DataProtocolType = UDP,
        int NumChansIn = gDefaultNumInChannels, int NumChansOut = gDefaultNumInChannels,
#ifdef WAIR  // wair
        int NumNetRevChans = 0,
#endif  // endwhere
        int BufferQueueLength                                  = gDefaultQueueLength,
        unsigned int redundancy                                = gDefaultRedundancy,
        AudioInterface::audioBitResolutionT AudioBitResolution = AudioInterface::BIT16,
        DataProtocol::packetHeaderTypeT PacketHeaderType       = DataProtocol::DEFAULT,
        underrunModeT UnderRunMode = WAVETABLE, int receiver_bind_port = gDefaultPort,
        int sender_bind_port = gDefaultPort, int receiver_peer_port = gDefaultPort,
        int sender_peer_port = gDefaultPort, int tcp_peer_port = gDefaultPort);

    /// \brief The class destructor
    virtual ~JackTrip();

    static void sigIntHandler(int /*unused*/)
    {
        std::cout << std::endl << "Shutting Down..." << std::endl;
        sSigInt = true;
    }
    static bool sSigInt;
    static bool sJackStopped;

    /// \brief Starting point for the thread
    /*virtual void run() {
        if (gVerboseFlag) std::cout << "Settings:startJackTrip before mJackTrip->run" <<
    std::endl;
    }*/

    /// \brief Set the Peer Address for jacktripModeT::CLIENT mode only
    virtual void setPeerAddress(QString PeerHostOrIP);

    /** \brief Append a process plugin. Processes will be appended in order
     * \param plugin Pointer to ProcessPlugin Class
     */
    // void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);
    virtual void appendProcessPluginToNetwork(ProcessPlugin* plugin);
    virtual void appendProcessPluginFromNetwork(ProcessPlugin* plugin);

    /// \brief Start the processing threads
    virtual void startProcess(
#ifdef WAIRTOHUB  // wair
        int ID
#endif  // endwhere
    );
    virtual void completeConnection();

    /// \brief Stop the processing threads
    virtual void stop(QString errorMessage = "");

    /// \brief Wait for all the threads to finish. This functions is used when JackTrip is
    /// run as a thread
    virtual void waitThreads();

    /// \brief Check if UDP port is already binded
    /// \param port Port number
    virtual void checkIfPortIsBinded(int port);

    //------------------------------------------------------------------------------------
    /// \name Getters and Setters Methods to change parameters after construction
    //@{
    //
    /// \brief Sets (override) JackTrip Mode after construction
    virtual void setJackTripMode(jacktripModeT JacktripMode)
    {
        mJackTripMode = JacktripMode;
    }
    /// \brief Sets (override) DataProtocol Type after construction
    virtual void setDataProtocoType(dataProtocolT DataProtocolType)
    {
        mDataProtocol = DataProtocolType;
    }
    /// \brief Sets the Packet header type
    virtual void setPacketHeaderType(DataProtocol::packetHeaderTypeT PacketHeaderType)
    {
        mPacketHeaderType = PacketHeaderType;
        delete mPacketHeader;
        mPacketHeader = NULL;
        createHeader(mPacketHeaderType);
    }
    /// \brief Sets (override) Buffer Queue Length Mode after construction
    virtual void setBufferQueueLength(int BufferQueueLength)
    {
        mBufferQueueLength = BufferQueueLength;
    }
    virtual void setBufferStrategy(int BufferStrategy)
    {
        mBufferStrategy = BufferStrategy;
    }
    /// \brief Sets (override) Audio Bit Resolution after construction
    virtual void setAudioBitResolution(
        AudioInterface::audioBitResolutionT AudioBitResolution)
    {
        mAudioBitResolution = AudioBitResolution;
    }
    /// \brief Sets (override) Underrun Mode
    virtual void setUnderRunMode(underrunModeT UnderRunMode)
    {
        mUnderRunMode = UnderRunMode;
    }
    /// \brief Sets whether to quit on timeout.
    virtual void setStopOnTimeout(bool stopOnTimeout) { mStopOnTimeout = stopOnTimeout; }
    /// \brief Sets port numbers for the local and peer machine.
    /// Receive port is <tt>port</tt>
    virtual void setAllPorts(int port)
    {
        mReceiverBindPort = port;
        mSenderPeerPort   = port;
        mSenderBindPort   = port;
        mReceiverPeerPort = port;
    }
    /// \brief Sets port numbers to bind in RECEIVER and SENDER sockets.
    void setBindPorts(int port)
    {
        mReceiverBindPort = port;
        mSenderBindPort   = port;
    }
    /// \brief Sets port numbers for the peer (remote) machine.
    void setPeerPorts(int port)
    {
        mSenderPeerPort   = port;
        mReceiverPeerPort = port;
    }
    void setPeerHandshakePort(int port) { mTcpServerPort = port; }
    void setUseAuth(bool auth) { mUseAuth = auth; }
    void setUsername(QString username) { mUsername = username; }
    void setPassword(QString password) { mPassword = password; }
    /// \brief Set Client Name to something different that the default (JackTrip)
    virtual void setClientName(QString clientName) { mJackClientName = clientName; }
    virtual void setRemoteClientName(QString remoteClientName)
    {
        mRemoteClientName = remoteClientName;
    }
    /// \brief Set the number of audio input channels
    virtual void setNumInputChannels(int num_chans) { mNumAudioChansIn = num_chans; }
    /// \brief Set the number of audio output channels
    virtual void setNumOutputChannels(int num_chans) { mNumAudioChansOut = num_chans; }

    virtual void setIOStatTimeout(int timeout) { mIOStatTimeout = timeout; }
    virtual void setIOStatStream(QSharedPointer<std::ostream> statStream)
    {
        mIOStatStream = statStream;
    }

    /// Set to connect or not default audio ports (only implemented in Jack)
    virtual void setConnectDefaultAudioPorts(bool connect)
    {
        mConnectDefaultAudioPorts = connect;
    }

    virtual int getReceiverBindPort() const { return mReceiverBindPort; }
    virtual int getSenderPeerPort() const { return mSenderPeerPort; }
    virtual int getSenderBindPort() const { return mSenderBindPort; }
    virtual int getReceiverPeerPort() const { return mReceiverPeerPort; }

    virtual DataProtocol* getDataProtocolSender() const { return mDataProtocolSender; }
    virtual DataProtocol* getDataProtocolReceiver() const
    {
        return mDataProtocolReceiver;
    }
    virtual void setDataProtocolSender(DataProtocol* const DataProtocolSender)
    {
        mDataProtocolSender = DataProtocolSender;
    }
    virtual void setDataProtocolReceiver(DataProtocol* const DataProtocolReceiver)
    {
        mDataProtocolReceiver = DataProtocolReceiver;
    }

    virtual RingBuffer* getSendRingBuffer() const { return mSendRingBuffer; }
    virtual RingBuffer* getReceiveRingBuffer() const { return mReceiveRingBuffer; }
    virtual void setSendRingBuffer(RingBuffer* const SendRingBuffer)
    {
        mSendRingBuffer = SendRingBuffer;
    }
    virtual void setReceiveRingBuffer(RingBuffer* const ReceiveRingBuffer)
    {
        mReceiveRingBuffer = ReceiveRingBuffer;
    }

    virtual void setPacketHeader(PacketHeader* const PacketHeader)
    {
        mPacketHeader = PacketHeader;
    }

    virtual int getInputRingBuffersSlotSize()
    {
        return getTotalAudioInputPacketSizeInBytes();
    }
    virtual int getOutputRingBuffersSlotSize()
    {
        return getTotalAudioOutputPacketSizeInBytes();
    }

    virtual void setAudiointerfaceMode(JackTrip::audiointerfaceModeT audiointerface_mode)
    {
        mAudiointerfaceMode = audiointerface_mode;
    }
    virtual void setAudioInterface(AudioInterface* const AudioInterface)
    {
        mAudioInterface = AudioInterface;
    }
    virtual void setLoopBack(bool b) { mLoopBack = b; }
    virtual void setAudioTesterP(QSharedPointer<AudioTester> atp) { mAudioTesterP = atp; }

    void setSampleRate(uint32_t sample_rate) { mSampleRate = sample_rate; }
    void setDeviceID(uint32_t device_id) { mDeviceID = device_id; }
    void setInputDevice(std::string device_name) { mInputDeviceName = device_name; }
    void setOutputDevice(std::string device_name) { mOutputDeviceName = device_name; }
    void setAudioBufferSizeInSamples(uint32_t buf_size) { mAudioBufferSize = buf_size; }

    JackTrip::hubConnectionModeT getHubConnectionModeT() const
    {
        return mHubConnectionModeT;
    }
    void setHubConnectionModeT(JackTrip::hubConnectionModeT connection_mode)
    {
        mHubConnectionModeT = connection_mode;
    }

    JackTrip::jacktripModeT getJackTripMode() const { return mJackTripMode; }

    QString getPeerAddress() const { return mPeerAddress; }

    bool receivedConnectionFromPeer() { return mReceivedConnection; }

    bool tcpConnectionError() { return mTcpConnectionError; }

    //@}
    //------------------------------------------------------------------------------------

    //------------------------------------------------------------------------------------
    /// \name Mediator Functions
    //@{
    /// \todo Document all these functions
    virtual void createHeader(const DataProtocol::packetHeaderTypeT headertype);
    void putHeaderInIncomingPacket(int8_t* full_packet, int8_t* audio_packet);
    void putHeaderInOutgoingPacket(int8_t* full_packet, int8_t* audio_packet);
    int getSendPacketSizeInBytes() const;
    int getReceivePacketSizeInBytes() const;
    virtual void sendNetworkPacket(const int8_t* ptrToSlot)
    {
        mSendRingBuffer->insertSlotNonBlocking(ptrToSlot, 0, 0);
    }
    virtual void receiveBroadcastPacket(int8_t* ptrToReadSlot)
    {
        mReceiveRingBuffer->readBroadcastSlot(ptrToReadSlot);
    }
    virtual void receiveNetworkPacket(int8_t* ptrToReadSlot)
    {
        mReceiveRingBuffer->readSlotNonBlocking(ptrToReadSlot);
    }
    virtual void readAudioBuffer(int8_t* ptrToReadSlot)
    {
        mSendRingBuffer->readSlotBlocking(ptrToReadSlot);
    }
    virtual bool writeAudioBuffer(const int8_t* ptrToSlot, int len, int lostLen)
    {
        return mReceiveRingBuffer->insertSlotNonBlocking(ptrToSlot, len, lostLen);
    }
    uint32_t getBufferSizeInSamples() const
    {
        return mAudioBufferSize; /*return mAudioInterface->getBufferSizeInSamples();*/
    }
    uint32_t getDeviceID() const
    {
        return mDeviceID; /*return mAudioInterface->mDeviceID();*/
    }

    AudioInterface::samplingRateT getSampleRateType() const
    {
        return mAudioInterface->getSampleRateType();
    }
    int getSampleRate() const
    {
        return mSampleRate; /*return mAudioInterface->getSampleRate();*/
    }

    uint8_t getAudioBitResolution() const
    {
        return mAudioBitResolution
               * 8; /*return mAudioInterface->getAudioBitResolution();*/
    }
    unsigned int getNumInputChannels() const
    {
        return mNumAudioChansIn; /*return mAudioInterface->getNumInputChannels();*/
    }
    unsigned int getNumOutputChannels() const
    {
        return mNumAudioChansOut; /*return mAudioInterface->getNumOutputChannels();*/
    }
#ifndef NO_JACK
    QString getAssignedClientName()
    {
        if (mAudioInterface && mAudiointerfaceMode == JackTrip::JACK) {
            return static_cast<JackAudioInterface*>(mAudioInterface)
                ->getAssignedClientName();
        } else {
            return "";
        }
    }
#endif
    virtual bool checkPeerSettings(int8_t* full_packet);
    void increaseSequenceNumber() { mPacketHeader->increaseSequenceNumber(); }
    int getSequenceNumber() const { return mPacketHeader->getSequenceNumber(); }

    uint64_t getPeerTimeStamp(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerTimeStamp(full_packet);
    }

    uint16_t getPeerSequenceNumber(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerSequenceNumber(full_packet);
    }

    uint16_t getPeerBufferSize(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerBufferSize(full_packet);
    }

    uint8_t getPeerSamplingRate(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerSamplingRate(full_packet);
    }

    uint8_t getPeerBitResolution(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerBitResolution(full_packet);
    }

    uint8_t getPeerNumIncomingChannels(int8_t* full_packet) const
    {
        return mPacketHeader->getPeerNumIncomingChannels(full_packet);
    }

    uint8_t getPeerNumOutgoingChannels(int8_t* full_packet) const
    {
        if (0 == mPacketHeader->getPeerNumOutgoingChannels(full_packet)) {
            return mPacketHeader->getPeerNumIncomingChannels(full_packet);
        } else {
            return mPacketHeader->getPeerNumOutgoingChannels(full_packet);
        }
    }

    size_t getSizeInBytesPerChannel() const
    {
        return mAudioInterface->getSizeInBytesPerChannel();
    }
    int getHeaderSizeInBytes() const { return mPacketHeader->getHeaderSizeInBytes(); }
    int getTotalAudioInputPacketSizeInBytes() const
    {
#ifdef WAIR  // WAIR
        if (mNumNetRevChans)
            return mAudioInterface->getSizeInBytesPerChannel() * mNumNetRevChans;
        else  // not wair
#endif        // endwhere
            return mAudioInterface->getSizeInBytesPerChannel() * mNumAudioChansIn;
    }

    int getTotalAudioOutputPacketSizeInBytes() const
    {
#ifdef WAIR  // WAIR
        if (mNumNetRevChans)
            return mAudioInterface->getSizeInBytesPerChannel() * mNumNetRevChans;
        else  // not wair
#endif        // endwhere
            return mAudioInterface->getSizeInBytesPerChannel() * mNumAudioChansOut;
    }
    //@}
    //------------------------------------------------------------------------------------

    void printTextTest() { std::cout << "=== JackTrip PRINT ===" << std::endl; }
    void printTextTest2() { std::cout << "=== JackTrip PRINT2 ===" << std::endl; }

    void setNetIssuesSimulation(double loss, double jitter, double delay_rel)
    {
        mSimulatedLossRate   = loss;
        mSimulatedJitterRate = jitter;
        mSimulatedDelayRel   = delay_rel;
    }
    void setBroadcast(int broadcast_queue) { mBroadcastQueueLength = broadcast_queue; }
    void queueLengthChanged(int queueLength)
    {
        emit signalQueueLengthChanged(queueLength);
    }
    void setUseRtUdpPriority(bool use) { mUseRtUdpPriority = use; }

   public slots:
    /// \brief Slot to stop all the processes and threads
    virtual void slotStopProcesses() { this->stop(); }
    virtual void slotStopProcessesDueToError(const QString& errorMessage)
    {
        this->stop(errorMessage);
    }

    /** \brief This slot emits in turn the signal signalNoUdpPacketsForSeconds
     * when UDP has waited for more than 30 seconds.
     *
     * It is used to remove the thread from the server.
     */
    void slotUdpWaitingTooLongClientGoneProbably(int wait_msec)
    {
        int wait_time = 10000;  // msec
        if (!(wait_msec % wait_time)) {
            std::cerr << "UDP WAITED MORE THAN 10 seconds." << std::endl;
            if (mStopOnTimeout) { stop("No network data received for 10 seconds"); }
            emit signalNoUdpPacketsForSeconds();
        }
    }
    void slotUdpWaitingTooLong() { emit signalUdpWaitingTooLong(); }
    void slotPrintTest() { std::cout << "=== TESTING ===" << std::endl; }
    void slotReceivedConnectionFromPeer()
    {
        mReceivedConnection = true;
        emit signalReceivedConnectionFromPeer();
    }
    void onStatTimer();

   private slots:
    void receivedConnectionTCP();
    void receivedDataTCP();
    void connectionSecured();
    void receivedDataUDP();
    void udpTimerTick();
    void tcpTimerTick();

   signals:
    // void signalUdpTimeOut();
    /// \brief Signal emitted when all the processes and threads are stopped
    void signalProcessesStopped();
    /// \brief Signal emitted when no UDP Packets have been received for a while
    void signalNoUdpPacketsForSeconds();
    void signalTcpClientConnected();
    void signalError(const QString& errorMessage);
    void signalReceivedConnectionFromPeer();
    void signalUdpWaitingTooLong();
    void signalQueueLengthChanged(int queueLength);
    void signalAudioStarted();

   public:
    /// \brief Set the AudioInteface object
    virtual void setupAudio(
#ifdef WAIRTOHUB  // WAIR
        int ID
#endif  // endwhere
    );
    /// \brief Close the JackAudioInteface and disconnects it from JACK
    void closeAudio();
    /// \brief Set the DataProtocol objects
    virtual void setupDataProtocol();
    /// \brief Set the RingBuffer objects
    void setupRingBuffers();
    /// \brief Starts for the CLIENT mode
    void clientStart();
    /// \brief Starts for the SERVER mode
    /// \param timout Set the server to timeout after 2 seconds if no client connections
    /// are received. Usefull for the multithreaded server \return 0 on success, -1 on
    /// error
    int serverStart(bool timeout = false, int udpTimeout = gTimeOutMultiThreadedServer);
    /// \brief Stats for the Client to Ping Server
    /// \return -1 on error, 0 on success
    virtual int clientPingToServerStart();
    /// \brief Sets the client ID
    ///
    void setID(int ID) { mID = ID; }
    /// \brief Gets the client ID
    ///
    int getID() { return mID; }

   private:
    int mID = 0;
    jacktripModeT mJackTripMode;                        ///< JackTrip::jacktripModeT
    dataProtocolT mDataProtocol;                        ///< Data Protocol Tipe
    DataProtocol::packetHeaderTypeT mPacketHeaderType;  ///< Packet Header Type
    JackTrip::audiointerfaceModeT mAudiointerfaceMode;

    int mNumAudioChansIn;    ///< Number of Audio Input Channels
    int mNumAudioChansOut;   ///< Number of Audio Output Channels
#ifdef WAIR                  // WAIR
    int mNumNetRevChans;     ///< Number of Network Audio Channels (net comb filters)
#endif                       // endwhere
    int mBufferQueueLength;  ///< Audio Buffer from network queue length
    int mBufferStrategy;
    int mBroadcastQueueLength;
    uint32_t mSampleRate;                             ///< Sample Rate
    uint32_t mDeviceID;                               ///< RTAudio DeviceID
    std::string mInputDeviceName, mOutputDeviceName;  ///< RTAudio device names
    uint32_t mAudioBufferSize;  ///< Audio buffer size to process on each callback
    AudioInterface::audioBitResolutionT mAudioBitResolution;  ///< Audio Bit Resolutions
    bool mLoopBack;
    QString mPeerAddress;  ///< Peer Address to use in jacktripModeT::CLIENT Mode

    /// Pointer to Abstract Type DataProtocol that sends packets
    DataProtocol* mDataProtocolSender;
    /// Pointer to Abstract Type DataProtocol that receives packets
    DataProtocol* mDataProtocolReceiver;
    AudioInterface* mAudioInterface;  ///< Interface to Jack Client
    PacketHeader* mPacketHeader;      ///< Pointer to Packet Header
    underrunModeT mUnderRunMode;      ///< underrunModeT Mode
    bool mStopOnTimeout;              ///< Stop on 10 second timeout

    /// Pointer for the Send RingBuffer
    RingBuffer* mSendRingBuffer;
    /// Pointer for the Receive RingBuffer
    RingBuffer* mReceiveRingBuffer;

    int mReceiverBindPort;  ///< Incoming (receiving) port for local machine
    int mSenderPeerPort;    ///< Incoming (receiving) port for peer machine
    int mSenderBindPort;    ///< Outgoing (sending) port for local machine
    int mReceiverPeerPort;  ///< Outgoing (sending) port for peer machine
    int mTcpServerPort;

    bool mUseAuth;
    QString mUsername;
    QString mPassword;

    unsigned int mRedundancy;   ///< Redundancy factor in network data
    QString mJackClientName;    ///< JackAudio Client Name
    QString mRemoteClientName;  ///< Remote JackAudio Client Name for hub client mode

    // JackTrip::connectionModeT mConnectionMode;  ///< Connection Mode
    JackTrip::hubConnectionModeT
        mHubConnectionModeT;  ///< Hub Server Jack Audio Patch Connection Mode

    QVector<ProcessPlugin*>
        mProcessPluginsFromNetwork;  ///< Vector of ProcessPlugin<EM>s</EM>
    QVector<ProcessPlugin*>
        mProcessPluginsToNetwork;  ///< Vector of ProcessPlugin<EM>s</EM>

    QTimer mTimeoutTimer;
    int mSleepTime;
    int mElapsedTime;
    int mEndTime;
    QSslSocket mTcpClient;
    QUdpSocket mUdpSockTemp;
    QMutex mTimerMutex;
    bool mAwaitingUdp;
    bool mAwaitingTcp;

    volatile bool mReceivedConnection;  ///< Bool of received connection from peer
    volatile bool mTcpConnectionError;
    volatile bool mStopped;
    volatile bool mHasShutdown;

    bool mConnectDefaultAudioPorts;  ///< Connect or not default audio ports
    QSharedPointer<std::ostream> mIOStatStream;
    int mIOStatTimeout;
    std::ostream mIOStatLogStream;
    double mSimulatedLossRate;
    double mSimulatedJitterRate;
    double mSimulatedDelayRel;
    bool mUseRtUdpPriority;

    QSharedPointer<AudioTester> mAudioTesterP;
};

#endif
