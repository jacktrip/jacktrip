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
 * \file JackTripWorker.cpp
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#include "JackTripWorker.h"

#include <QMutexLocker>
#include <QTimer>
#include <QWaitCondition>
#include <iostream>
#include <limits>

#include "JackTrip.h"
#include "UdpHubListener.h"
//#include "NetKS.h"
#include "LoopBack.h"
#include "Settings.h"
#ifdef WAIR  // wair
#include "dcblock2gain.dsp.h"
#endif  // endwhere

using std::cout;
using std::endl;

//*******************************************************************************
JackTripWorker::JackTripWorker(UdpHubListener* udphublistener, int BufferQueueLength,
                               JackTrip::underrunModeT UnderRunMode, QString clientName)
    : mAppendThreadID(false)
    , mSleepTime(100)
    , mUdpHubListener(udphublistener)
    , mBufferQueueLength(BufferQueueLength)
    , mUnderRunMode(UnderRunMode)
    , mClientName(clientName)
{
    // mNetks = new NetKS;
    // mNetks->play();
    connect(&mUdpSockTemp, &QUdpSocket::readyRead, this,
            &JackTripWorker::receivedDataUDP);
}

//*******************************************************************************
void JackTripWorker::setJackTrip(int id, QString client_address, uint16_t server_port,
                                 uint16_t client_port, bool connectDefaultAudioPorts)
{
    QMutexLocker locker(&mMutex);
    mUdpSockTemp.close();
    if (mRunning) {
        mJackTrip->slotStopProcesses();
#ifndef NO_JACK
        if (mPatched) {
            mUdpHubListener->unregisterClientWithPatcher(mAssignedClientName);
            mPatched = false;
        }
#endif
        mRunning = false;
    }
    // Set as spawning from this point on.
    mSpawning = true;

    mID = id;
    // Set the jacktrip address and ports
    mClientAddress             = client_address;
    mServerPort                = server_port;
    mClientPort                = client_port;
    m_connectDefaultAudioPorts = connectDefaultAudioPorts;
    mAssignedClientName        = "";

    // Create and setup JackTrip Object
    // JackTrip jacktrip(JackTrip::SERVER, JackTrip::UDP, mNumChans, 2);
    if (gVerboseFlag) cout << "---> JackTripWorker: Creating jacktrip objects..." << endl;

#ifdef WAIR  // WAIR
             // forces    BufferQueueLength to 2
             // need to parse numNetChans from incoming header
             // but force to 16 for now
#define FORCEBUFFERQ 2
    if (mUdpHubListener->isWAIR()) {  // invoked with -Sw
        mWAIR           = true;
        mNumNetRevChans = NUMNETREVCHANSbecauseNOTINRECEIVEDheader;
    } else {
    };
#endif  // endwhere

#ifndef __JAMTEST__
#ifdef WAIR  // WAIR
    //        bool tmp = mJTWorkers->at(id)->isWAIR();
    //        qDebug() << "is WAIR?" <<  tmp ;
    qDebug() << "mNumNetRevChans" << mNumNetRevChans;

    mJackTrip.reset(new JackTrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, 1, 1,
                                 mNumNetRevChans, FORCEBUFFERQ));
    // Add Plugins
    if (mWAIR) {
        cout << "Running in WAIR Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        switch (mNumNetRevChans) {
        case 16:  // freeverb
            mJackTrip->appendProcessPluginFromNetwork(
                new dcblock2gain(1));  // plugin slot 0
            ///////////////
            //            mJackTrip->appendProcessPlugin(new comb16server(mNumNetChans));
            // -S LAIR no AP  mJackTrip->appendProcessPlugin(new AP8(mNumChans));
            break;
        default:
            throw std::invalid_argument(
                "Settings: mNumNetChans doesn't correspond to Faust plugin");
            break;
        }
    }
#else   // endwhere
    mJackTrip.reset(new JackTrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, 1, 1,
                                 mBufferQueueLength));
#endif  // not wair
#endif  // ifndef __JAMTEST__

#ifdef __JAMTEST__
    mJackTrip.reset(new JamTest(
        JackTrip::SERVERPINGSERVER));  // ########### JamTest #################
    // JackTrip jacktrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans, 2);
#endif
}

//*******************************************************************************
void JackTripWorker::start()
{
    QMutexLocker lock(&mMutex);
    if (!mSpawning) {
        // Something else has aborted the connection.
        return;
    }

    mJackTrip->setConnectDefaultAudioPorts(m_connectDefaultAudioPorts);

    // Set our underrun mode
    mJackTrip->setUnderRunMode(mUnderRunMode);
    if (mIOStatTimeout > 0) {
        mJackTrip->setIOStatTimeout(mIOStatTimeout);
        mJackTrip->setIOStatStream(mIOStatStream);
    }

    if (!mClientName.isEmpty()) { mJackTrip->setClientName(mClientName); }

    // ClientAddress.setAddress(mClientAddress);
    // If I don't type this line, I get a bus error in the next line.
    // I still haven't figure out why
    // ClientAddress.toString().toLatin1().constData();
    // jacktrip.setPeerAddress(ClientAddress.toString().toLatin1().constData());
    if (mAppendThreadID) { mJackTrip->setID(mID + 1); }
    mJackTrip->setPeerAddress(mClientAddress);
    mJackTrip->setBindPorts(mServerPort);
    // jacktrip.setPeerPorts(mClientPort);
    mJackTrip->setBufferStrategy(mBufferStrategy);
    mJackTrip->setNetIssuesSimulation(mSimulatedLossRate, mSimulatedJitterRate,
                                      mSimulatedDelayRel);
    mJackTrip->setBroadcast(mBroadcastQueue);
    mJackTrip->setUseRtUdpPriority(mUseRtUdpPriority);

    mTimeoutTimer.setInterval(mSleepTime);
    connect(&mTimeoutTimer, &QTimer::timeout, this, &JackTripWorker::udpTimerTick);
    mElapsedTime = 0;
    mTimeoutTimer.start();
    if (gVerboseFlag)
        cout << "---> JackTripWorker: setJackTripFromClientHeader..." << endl;
    if (!mUdpSockTemp.bind(QHostAddress::Any, mServerPort,
                           QUdpSocket::DefaultForPlatform)) {
        std::cerr
            << "in JackTripWorker: Could not bind UDP socket. It may already be bound."
            << endl;
        throw std::runtime_error("Could not bind UDP socket. It may already be bound.");
    }
}

//*******************************************************************************
void JackTripWorker::stopThread()
{
    QMutexLocker locker(&mMutex);
    if (mRunning) {
        mRunning = false;
        mJackTrip->slotStopProcesses();
#ifndef NO_JACK
        if (mPatched) {
            mUdpHubListener->unregisterClientWithPatcher(mAssignedClientName);
            mPatched = false;
        }
#endif
    } else if (mSpawning) {
        mSpawning = false;
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
    }
}

void JackTripWorker::receivedDataUDP()
{
    QMutexLocker lock(&mMutex);

    if (!mSpawning || mUdpSockTemp.state() != QAbstractSocket::BoundState) {
        // Check if something has interrupted the process.
        return;
    }
    mTimeoutTimer.stop();

    // Set our jacktrip parameters from the received header data.
    quint16 port;
    int packet_size     = mUdpSockTemp.pendingDatagramSize();
    int8_t* full_packet = new int8_t[packet_size];
    mUdpSockTemp.readDatagram(reinterpret_cast<char*>(full_packet), packet_size, nullptr,
                              &port);
    mUdpSockTemp.close();  // close the socket

    // Alert the hub listener of the actual client port for incoming packets.
    // This will remove any old worker objects, and will set the client port member
    // variable on this object.
    mUdpHubListener->releaseDuplicateThreads(this, port);

    int PeerBufferSize          = mJackTrip->getPeerBufferSize(full_packet);
    int PeerSamplingRate        = mJackTrip->getPeerSamplingRate(full_packet);
    int PeerBitResolution       = mJackTrip->getPeerBitResolution(full_packet);
    int PeerNumIncomingChannels = mJackTrip->getPeerNumIncomingChannels(full_packet);
    int PeerNumOutgoingChannels = mJackTrip->getPeerNumOutgoingChannels(full_packet);
    delete[] full_packet;

    if (gVerboseFlag) {
        cout << "JackTripWorker: getPeerBufferSize       = " << PeerBufferSize << "\n"
             << "JackTripWorker: getPeerSamplingRate     = " << PeerSamplingRate << "\n"
             << "JackTripWorker: getPeerBitResolution    = " << PeerBitResolution << "\n"
             << "JackTripWorker: PeerNumIncomingChannels = " << PeerNumIncomingChannels
             << "\n"
             << "JackTripWorker: PeerNumOutgoingChannels = " << PeerNumOutgoingChannels
             << "\n";
    }

    // The header field for NumOutgoingChannels was used for the ConnectionMode.
    // Only the first Mode was used (NORMAL == 0). If this field is set to 0, we
    // can assume the peer is using an old version, and the last field doesn't reflect the
    // number of Outgoing Channels.
    // The maximum of this field will be used as 0.
    if (JackTrip::NORMAL == PeerNumOutgoingChannels) {
        mJackTrip->setNumInputChannels(PeerNumIncomingChannels);
        mJackTrip->setNumOutputChannels(PeerNumIncomingChannels);
    } else if (std::numeric_limits<uint8_t>::max() == PeerNumOutgoingChannels) {
        mJackTrip->setNumInputChannels(PeerNumIncomingChannels);
        mJackTrip->setNumOutputChannels(0);
    } else {
        mJackTrip->setNumInputChannels(PeerNumIncomingChannels);
        mJackTrip->setNumOutputChannels(PeerNumOutgoingChannels);
    }

    if (PeerNumOutgoingChannels == -1) {
        // Shut it down
        mSpawning = false;
        mUdpHubListener->releaseThread(mID);
    }

    // Connect signals and slots
    // -------------------------
    if (gVerboseFlag)
        cout << "---> JackTripWorker: Connecting signals and slots..." << endl;
    // Connection to terminate JackTrip when packets haven't arrive for
    // a certain amount of time
    connect(mJackTrip.data(), &JackTrip::signalNoUdpPacketsForSeconds, mJackTrip.data(),
            &JackTrip::slotStopProcesses, Qt::QueuedConnection);
    connect(mJackTrip.data(), &JackTrip::signalProcessesStopped, this,
            &JackTripWorker::jacktripStopped, Qt::QueuedConnection);
    connect(mJackTrip.data(), &JackTrip::signalError, this,
            &JackTripWorker::jacktripStopped, Qt::QueuedConnection);
#ifndef NO_JACK
    connect(mJackTrip.data(), &JackTrip::signalAudioStarted, this,
            &JackTripWorker::alertPatcher, Qt::QueuedConnection);
#endif
    connect(this, &JackTripWorker::signalRemoveThread, mJackTrip.data(),
            &JackTrip::slotStopProcesses, Qt::QueuedConnection);

    if (gVerboseFlag) cout << "---> JackTripWorker: startProcess..." << endl;
    mJackTrip->startProcess(
#ifdef WAIRTOHUB  // wair
        mID
#endif  // endwhere
    );
    mRunning  = true;
    mSpawning = false;
    // if (gVerboseFlag) cout << "---> JackTripWorker: start..." << endl;
    // jacktrip.start(); // ########### JamTest Only #################
}

void JackTripWorker::udpTimerTick()
{
    QMutexLocker lock(&mMutex);
    if (!mSpawning) {
        mTimeoutTimer.stop();
        return;
    }
    mElapsedTime += mSleepTime;
    if (gVerboseFlag) cout << "---------> ELAPSED TIME: " << mElapsedTime << endl;
    // Check if we've timed out.
    if (gTimeOutMultiThreadedServer > 0 && mElapsedTime >= gTimeOutMultiThreadedServer) {
        std::cerr << "--->JackTripWorker: is not receiving Datagrams (timeout)" << endl;
        mTimeoutTimer.stop();
        mUdpSockTemp.close();
        mSpawning = false;
        mUdpHubListener->releaseThread(mID);
    }
}

void JackTripWorker::jacktripStopped()
{
    QMutexLocker lock(&mMutex);
    if (mSpawning || !mRunning) {
        // This has already been taken care of elsewhere.
        return;
    }
    mRunning = false;
#ifndef NO_JACK
    if (mPatched) {
        mUdpHubListener->unregisterClientWithPatcher(mAssignedClientName);
        mPatched = false;
    }
#endif
    mUdpHubListener->releaseThread(mID);
}

void JackTripWorker::alertPatcher()
{
#ifndef NO_JACK
    QMutexLocker lock(&mMutex);
    if (mRunning) {
        mAssignedClientName = mJackTrip->getAssignedClientName();
        mUdpHubListener->registerClientWithPatcher(mAssignedClientName);
        mPatched = true;
    }
#endif
}
