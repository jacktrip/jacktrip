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
 * \file JackTripWorker.h
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#ifndef __JACKTRIPWORKER_H__
#define __JACKTRIPWORKER_H__

#include <QHostAddress>
#include <QMutex>
#include <QObject>
#include <QUdpSocket>
#include <iostream>

#include "JackTrip.h"
#include "jacktrip_globals.h"
#ifdef __JAMTEST__
#include "JamTest.h"
#endif

// class JackTrip; // forward declaration
class UdpHubListener;  // forward declaration

/** \brief Prototype of the worker class that will be cloned through sending threads to
 * the Thread Pool
 *
 * This class can be send to the ThreadPool using the start() method. Each time
 * it is sent, it'll became "independent" of the prototype, which means
 * that the prototype state can be changed, and used to send and start
 * another thread into the pool. setAutoDelete must be set to false
 * in order for this to work.
 */
// Note that it is not possible to start run() as an event loop. That has to be
// implemented inside a QThread
class JackTripWorker : public QObject
{
    Q_OBJECT;

   public:
    /// \brief The class constructor
    JackTripWorker(
        UdpHubListener* udphublistener, int BufferQueueLength = gDefaultQueueLength,
        JackTrip::underrunModeT UnderRunMode                   = JackTrip::WAVETABLE,
        AudioInterface::audioBitResolutionT AudioBitResolution = AudioInterface::BIT16,
        const QString& clientName                              = QLatin1String(""));
    /// \brief The class destructor
    virtual ~JackTripWorker() { stopThread(); }

    /// \brief Starts the jacktrip process
    void start();
    /// \brief Check if the Thread is Spawning
    /// \return true is it is spawning, false if it's already running
    bool isSpawning()
    {
        QMutexLocker lock(&mMutex);
        return mSpawning;
    }
    /// \brief Check if jacktrip is running
    /// \return true if it is running, false if not
    bool isRunning()
    {
        QMutexLocker lock(&mMutex);
        return mRunning;
    }
    /// \brief Sets the JackTripWorker properties
    /// \param id ID number
    /// \param address
    void setJackTrip(int id, const QString& client_address, uint16_t server_port,
                     uint16_t client_port, bool connectDefaultAudioPorts);
    /// Stop thread
    void stopThread();
    int getID() { return mID; }

    void setBufferStrategy(int BufferStrategy) { mBufferStrategy = BufferStrategy; }
    void setRegulatorThread(QThread* ptr) { mRegulatorThreadPtr = ptr; }
    void setNetIssuesSimulation(double loss, double jitter, double delay_rel)
    {
        mSimulatedLossRate   = loss;
        mSimulatedJitterRate = jitter;
        mSimulatedDelayRel   = delay_rel;
    }
    void setBroadcast(int broadcast_queue) { mBroadcastQueue = broadcast_queue; }
    void setUseRtUdpPriority(bool use) { mUseRtUdpPriority = use; }

    void setIOStatTimeout(int timeout) { mIOStatTimeout = timeout; }
    void setIOStatStream(QSharedPointer<std::ostream> statStream)
    {
        mIOStatStream = statStream;
    }

    bool mAppendThreadID;

    void setClientPort(uint16_t port) { mClientPort = port; }
    QString getAssignedClientName() { return mAssignedClientName; }
    uint16_t getServerPort() { return mServerPort; }
    uint16_t getClientPort() { return mClientPort; }
    QString getClientAddress() { return mClientAddress; }

   private slots:
    void slotTest() { std::cout << "--- JackTripWorker TEST SLOT ---" << std::endl; }
    void receivedDataUDP();
    void udpTimerTick();
    void jacktripStopped();
    void alertPatcher();

   signals:
    void signalRemoveThread();

   private:
    JackTrip::connectionModeT getConnectionModeFromHeader();

    QUdpSocket mUdpSockTemp;
    QTimer mTimeoutTimer;
    int mSleepTime;
    int mElapsedTime;
#ifdef __JAMTEST__
    QScopedPointer<JamTest> mJackTrip;
#else
    QScopedPointer<JackTrip> mJackTrip;
#endif

    UdpHubListener* mUdpHubListener;  ///< Hub Listener Socket
    // QHostAddress mClientAddress; ///< Client Address
    QString mClientAddress;
    uint16_t mServerPort;  ///< Server Ephemeral Incoming Port to use with Client
    bool m_connectDefaultAudioPorts = false;

    /// Client Outgoing Port. By convention, the receiving port will be <tt>mClientPort
    /// -1</tt>
    uint16_t mClientPort;

    int mBufferQueueLength;
    JackTrip::underrunModeT mUnderRunMode;
    AudioInterface::audioBitResolutionT mAudioBitResolution;
    QString mClientName;
    QString mAssignedClientName;

    /// Thread spawning internal lock.
    /// If true, the prototype is working on creating (spawning) a new thread
    volatile bool mSpawning = false;
    volatile bool mRunning  = false;
    volatile bool mPatched  = false;
    QMutex mMutex;  ///< Mutex to protect mSpawning

    int mID = 0;  ///< ID thread number

    int mBufferStrategy          = 1;
    int mBroadcastQueue          = 0;
    double mSimulatedLossRate    = 0.0;
    double mSimulatedJitterRate  = 0.0;
    double mSimulatedDelayRel    = 0.0;
    bool mUseRtUdpPriority       = false;
    int mIOStatTimeout           = 0;
    QThread* mRegulatorThreadPtr = NULL;

    QSharedPointer<std::ostream> mIOStatStream;
#ifdef WAIR                   // wair
    int mNumNetRevChans = 0;  ///< Number of Net Channels = net combs
    bool mWAIR          = false;
#endif  // endwhere
};

#endif  //__JACKTRIPWORKER_H__
