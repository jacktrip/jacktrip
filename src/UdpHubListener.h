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
 * \file UdpHubListener.h
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#ifndef __UDPHUBLISTENER_H__
#define __UDPHUBLISTENER_H__

#include <QHostAddress>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QUdpSocket>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "JackTrip.h"
#include "jacktrip_globals.h"
#include "jacktrip_types.h"
#ifndef NO_JACK
#include "Patcher.h"
#endif
#include "Auth.h"
#include "SslServer.h"

class JackTripWorker;  // forward declaration
class Settings;

/** \brief Hub UDP listener on the Server.
 *
 * This creates a server that will listen on the well know port (the server port) and will
 * spawn JackTrip threads into the Thread pool. Clients request a connection.
 */
class UdpHubListener : public QObject
{
    Q_OBJECT;

   public:
    UdpHubListener(int server_port = gServerUdpPort, int server_udp_port = 0);
    virtual ~UdpHubListener();

    /// \brief Starts the TCP server
    void start();

    /// \brief Stops the execution of the Thread
    void stop() { mStopped = true; }

#ifndef NO_JACK
    void registerClientWithPatcher(QString& clientName);
    void unregisterClientWithPatcher(QString& clientName);
#endif
    int releaseThread(int id);
    void releaseDuplicateThreads(JackTripWorker* worker, uint16_t actual_peer_port);

    void setConnectDefaultAudioPorts(bool connectDefaultAudioPorts)
    {
        // Only allow us to override this in the default patching mode. (Or TUB mode.)
        //(Allows -D to continue to function as a synonym for -p5.)
        if (mHubPatch == JackTrip::SERVERTOCLIENT
            || mHubPatch == JackTrip::RESERVEDMATRIX) {
            m_connectDefaultAudioPorts = connectDefaultAudioPorts;
        }
    }

    static void sigIntHandler([[maybe_unused]] int unused)
    {
        std::cout << std::endl << "Shutting Down..." << std::endl;
        sSigInt = true;
    }

   private slots:
    void testReceive()
    {
        std::cout << "========= TEST RECEIVE SLOT ===========" << std::endl;
    }
    void receivedNewConnection();
    void stopCheck();

   signals:
    void Listening();
    void ClientAddressSet();
    void signalRemoveThread(int id);
    void signalStopped();
    void signalError(const QString& errorMessage);

   private:
    /** \brief Binds a QUdpSocket. It chooses the available (active) interface.
     * \param udpsocket a QUdpSocket
     * \param port Port number
     */
    void receivedClientInfo(QSslSocket* clientConnection);

    static void bindUdpSocket(QUdpSocket& udpsocket, int port);

    int readClientUdpPort(QSslSocket* clientConnection, QString& clientName);
    int checkAuthAndReadPort(QSslSocket* clientConnection, QString& clientName);
    int sendUdpPort(QSslSocket* clientConnection, qint32 udp_port);

    /**
     * \brief Send the JackTripWorker to the thread pool. This will run
     * until it's done. We still have control over the prototype class.
     * \param id Identification Number
     */
    // void sendToPoolPrototype(int id);

    /**
     * \brief Check if address is already handled and reuse or create
     * a JackTripWorker as appropriate
     * \param address as string (IPv4 or IPv6)
     * \return id number of JackTripWorker
     */
    int getJackTripWorker(QString address, uint16_t port, QString& clientName);

    /** \brief Returns the ID of the client in the pool. If the client
     * is not in the pool yet, returns -1.
     */
    int getPoolID(QString address, uint16_t port);

    void stopAllThreads();

    // QUdpSocket mUdpHubSocket; ///< The UDP socket
    // QHostAddress mPeerAddress; ///< The Peer Address

    // JackTripWorker* mJTWorker; ///< Class that will be used as prototype
    QVector<JackTripWorker*>* mJTWorkers;  ///< Vector of JackTripWorkers

    SslServer mTcpServer;
    int mServerPort;     //< Server known port number
    int mServerUdpPort;  //< Server udp base port number
    int mBasePort;
    // addressPortNameTriple mActiveAddress[gMaxThreads]; ///< Active address pool
    // addresses QHash<QString, uint16_t> mActiveAddressPortPair;

    bool mRequireAuth;
    QString mCertFile;
    QString mKeyFile;
    QString mCredsFile;
    QScopedPointer<Auth> mAuth;

    /// Boolean stop the execution of the thread
    volatile bool mStopped;
    static bool sSigInt;
    QTimer mStopCheckTimer;
    int mTotalRunningThreads;  ///< Number of Threads running in the pool
    QMutex mMutex;
    JackTrip::underrunModeT mUnderRunMode;
    int mBufferQueueLength;

    QStringList mHubPatchDescriptions;
    bool m_connectDefaultAudioPorts;
#ifndef NO_JACK
    Patcher mPatcher;
#endif
    bool mStereoUpmix;

    int mIOStatTimeout;
    QSharedPointer<std::ostream> mIOStatStream;

    int mBufferStrategy;
    int mBroadcastQueue;
    double mSimulatedLossRate;
    double mSimulatedJitterRate;
    double mSimulatedDelayRel;
    bool mUseRtUdpPriority;

#ifdef WAIR  // wair
    bool mWAIR;
    void connectMesh(bool spawn);
    void enumerateRunningThreadIDs();

   public:
    void setWAIR(int b) { mWAIR = b; }
    bool isWAIR() { return mWAIR; }
#endif  // endwhere
#ifndef NO_JACK
    void connectPatch(bool spawn, const QString& clientName);
#endif

   public:
    void setRequireAuth(bool requireAuth) { mRequireAuth = requireAuth; }
    void setCertFile(QString certFile) { mCertFile = certFile; }
    void setKeyFile(QString keyFile) { mKeyFile = keyFile; }
    void setCredsFile(QString credsFile) { mCredsFile = credsFile; }

    unsigned int mHubPatch;
    void setHubPatch(unsigned int p)
    {
        mHubPatch = p;
#ifndef NO_JACK
        mPatcher.setPatchMode(static_cast<JackTrip::hubConnectionModeT>(p));
#endif
        // Set the correct audio port connection setting for our chosen patch mode.
        if (mHubPatch == JackTrip::SERVERTOCLIENT) {
            m_connectDefaultAudioPorts = true;
        } else {
            m_connectDefaultAudioPorts = false;
        }
    }
    unsigned int getHubPatch() { return mHubPatch; }
    
    void setStereoUpmix([[maybe_unused]] bool upmix) {
#ifndef NO_JACK
        mPatcher.setStereoUpmix(upmix);
#endif
    }

    void setUnderRunMode(JackTrip::underrunModeT UnderRunMode)
    {
        mUnderRunMode = UnderRunMode;
    }
    void setBufferQueueLength(int BufferQueueLength)
    {
        mBufferQueueLength = BufferQueueLength;
    }

    void setIOStatTimeout(int timeout) { mIOStatTimeout = timeout; }
    void setIOStatStream(QSharedPointer<std::ostream> statStream)
    {
        mIOStatStream = statStream;
    }

    void setBufferStrategy(int BufferStrategy) { mBufferStrategy = BufferStrategy; }
    void setNetIssuesSimulation(double loss, double jitter, double delay_rel)
    {
        mSimulatedLossRate   = loss;
        mSimulatedJitterRate = jitter;
        mSimulatedDelayRel   = delay_rel;
    }
    void setBroadcast(int broadcast_queue) { mBroadcastQueue = broadcast_queue; }
    void setUseRtUdpPriority(bool use) { mUseRtUdpPriority = use; }
    bool mAppendThreadID = false;
};

#endif  //__UDPHUBLISTENER_H__
