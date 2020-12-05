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
 * \file JackTripWorker.cpp
 * \author Juan-Pablo Caceres
 * \date September 2008
 */

#include <iostream>
#include <unistd.h>

#include <QTimer>
#include <QMutexLocker>
#include <QWaitCondition>

#include "JackTripWorker.h"
#include "JackTrip.h"
#include "UdpHubListener.h"
//#include "NetKS.h"
#include "LoopBack.h"
#include "Settings.h"
#ifdef WAIR // wair
#include "dcblock2gain.dsp.h"
#endif // endwhere

using std::cout; using std::endl;

//*******************************************************************************
JackTripWorker::JackTripWorker(UdpHubListener* udphublistener, int BufferQueueLength, JackTrip::underrunModeT UnderRunMode, QString clientName) :
    mSleepTime(100),
    mUdpHubListener(udphublistener),
    m_connectDefaultAudioPorts(false),
    mBufferQueueLength(BufferQueueLength),
    mUnderRunMode(UnderRunMode),
    mClientName(clientName),
    mRunning(false),
    mID(0),
    mNumChans(1),
    mIOStatTimeout(0)
  #ifdef WAIR // wair
  ,mNumNetRevChans(0),
    mWAIR(false)
  #endif // endwhere
{
    //mNetks = new NetKS;
    //mNetks->play();
    mBufferStrategy = 1;
    mBroadcastQueue = 0;
    mSimulatedLossRate = 0.0;
    mSimulatedJitterRate = 0.0;
    mSimulatedDelayRel = 0.0;
    mUseRtUdpPriority = false;
    connect(&mUdpSockTemp, &QUdpSocket::readyRead, this, &JackTripWorker::receivedDataUDP);
}


//*******************************************************************************
JackTripWorker::~JackTripWorker()
{
    //delete mUdpHubListener;
}


//*******************************************************************************
void JackTripWorker::setJackTrip(int id,
                                 QString client_address,
                                 uint16_t server_port,
                                 uint16_t client_port,
                                 int num_channels,
                                 bool connectDefaultAudioPorts)
{
    QMutexLocker locker(&mMutex);
    mUdpSockTemp.close();
    if (mRunning) {
        mJackTrip->slotStopProcesses();
        mRunning = false;
    }
    //Set as spawning from this point on.
    mSpawning = true;
    
    mID = id;
    //Set the jacktrip address and ports
    //mClientAddress.setAddress(client_address);
    mClientAddress = client_address;
    mServerPort = server_port;
    mClientPort = client_port;
    mNumChans = num_channels;
    m_connectDefaultAudioPorts = connectDefaultAudioPorts;
    
    // Create and setup JackTrip Object
    //JackTrip jacktrip(JackTrip::SERVER, JackTrip::UDP, mNumChans, 2);
    if (gVerboseFlag) cout << "---> JackTripWorker: Creating jacktrip objects..." << endl;

#ifdef WAIR // WAIR
    // forces    BufferQueueLength to 2
    // need to parse numNetChans from incoming header
    // but force to 16 for now
#define FORCEBUFFERQ 2
    if (mUdpHubListener->isWAIR()) { // invoked with -Sw
        mWAIR = true;
        mNumNetRevChans = NUMNETREVCHANSbecauseNOTINRECEIVEDheader;
    } else {};
#endif // endwhere

#ifndef __JAMTEST__
#ifdef WAIR // WAIR
    //        bool tmp = mJTWorkers->at(id)->isWAIR();
    //        qDebug() << "is WAIR?" <<  tmp ;
    qDebug() << "mNumNetRevChans" <<  mNumNetRevChans ;

    mJackTrip.reset(new JackTrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans,
                                 mNumNetRevChans, FORCEBUFFERQ));
    // Add Plugins
    if ( mWAIR ) {
        cout << "Running in WAIR Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        switch ( mNumNetRevChans )
        {
        case 16 : // freeverb
            mJackTrip->appendProcessPluginFromNetwork(new dcblock2gain(mNumChans)); // plugin slot 0
            ///////////////
            //            mJackTrip->appendProcessPlugin(new comb16server(mNumNetChans));
            // -S LAIR no AP  mJackTrip->appendProcessPlugin(new AP8(mNumChans));
            break;
        default:
            throw std::invalid_argument("Settings: mNumNetChans doesn't correspond to Faust plugin");
            break;
        }
    }
#else // endwhere
    mJackTrip.reset(new JackTrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans, mBufferQueueLength));
#endif // not wair
#endif // ifndef __JAMTEST__

#ifdef __JAMTEST__
    mJackTrip.reset(new JamTest(JackTrip::SERVERPINGSERVER)); // ########### JamTest #################
    //JackTrip jacktrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans, 2);
#endif
}


//*******************************************************************************
void JackTripWorker::start()
{
    QMutexLocker lock(&mMutex);
    if (!mSpawning) {
        //Something else has aborted the connection.
        return;
    }
    
    mJackTrip->setConnectDefaultAudioPorts(m_connectDefaultAudioPorts);

    // Set our underrun mode
    mJackTrip->setUnderRunMode(mUnderRunMode);
    if (mIOStatTimeout > 0) {
        mJackTrip->setIOStatTimeout(mIOStatTimeout);
        mJackTrip->setIOStatStream(mIOStatStream);
    }
    
    if (!mClientName.isEmpty()) {
        mJackTrip->setClientName(mClientName);
    }

    //ClientAddress.setAddress(mClientAddress);
    // If I don't type this line, I get a bus error in the next line.
    // I still haven't figure out why
    //ClientAddress.toString().toLatin1().constData();
    //jacktrip.setPeerAddress(ClientAddress.toString().toLatin1().constData());
    mJackTrip->setPeerAddress(mClientAddress);
    mJackTrip->setBindPorts(mServerPort);
    //jacktrip.setPeerPorts(mClientPort);
    mJackTrip->setBufferStrategy(mBufferStrategy);
    mJackTrip->setNetIssuesSimulation(mSimulatedLossRate,
        mSimulatedJitterRate, mSimulatedDelayRel);
    mJackTrip->setBroadcast(mBroadcastQueue);
    mJackTrip->setUseRtUdpPriority(mUseRtUdpPriority);

    if (gVerboseFlag) cout << "---> JackTripWorker: setJackTripFromClientHeader..." << endl;
    if ( !mUdpSockTemp.bind(QHostAddress::Any, mServerPort,
                            QUdpSocket::DefaultForPlatform) )
    {
        std::cerr << "in JackTripWorker: Could not bind UDP socket. It may already be bound." << endl;
        throw std::runtime_error("Could not bind UDP socket. It may already be bound.");
    }
    mTimeoutTimer.setInterval(mSleepTime);
    connect(&mTimeoutTimer, &QTimer::timeout, this, &JackTripWorker::udpTimerTick);
    mTimeoutTimer.start();
}  
    
    
/*    
    event_loop.exec(); // Excecution will block here until exit() the QEventLoop
    //--------------------------------------------------------------------------
    
    { QMutexLocker locker(&mMutex); mSpawning = true; }

    // wait for jacktrip to be done before exiting the Worker Thread
    //jacktrip.wait();


    {
        QMutexLocker locker(&mMutex);
        mUdpHubListener->releaseThread(mID);
    }

    cout << "JackTrip ID = " << mID << " released from the THREAD POOL" << endl;
    cout << gPrintSeparator << endl;
    {
        // Thread is already spawning, so release the lock
        QMutexLocker locker(&mMutex);
        mSpawning = false;
    }
    */


//*******************************************************************************
void JackTripWorker::stopThread()
{
    QMutexLocker locker(&mMutex);
    if (mRunning) {
        mRunning = false;
        mJackTrip->slotStopProcesses();
        mUdpHubListener->releaseThread(mID, mAssignedClientName);
    }
    if (mSpawning) {
        mSpawning = false;
        mUdpSockTemp.close();
        mTimeoutTimer.stop();
        mUdpHubListener->releaseThread(mID);
    }
}

void JackTripWorker::receivedDataUDP()
{
    QMutexLocker lock(&mMutex);
    
    if (!mSpawning || mUdpSockTemp.state() != QAbstractSocket::BoundState) {
        //Check if something has interrupted the process.
        return;
    }
    mTimeoutTimer.stop();
    
    //Set our jacktrip parameters from the received header data.
    int packet_size = mUdpSockTemp.pendingDatagramSize();
    char packet[packet_size];
    mUdpSockTemp.readDatagram(packet, packet_size);
    mUdpSockTemp.close(); // close the socket
    int8_t* full_packet = reinterpret_cast<int8_t*>(packet);

    int PeerBufferSize = mJackTrip->getPeerBufferSize(full_packet);
    int PeerSamplingRate = mJackTrip->getPeerSamplingRate(full_packet);
    int PeerBitResolution = mJackTrip->getPeerBitResolution(full_packet);
    int PeerNumChannels = mJackTrip->getPeerNumChannels(full_packet);
    int PeerConnectionMode = mJackTrip->getPeerConnectionMode(full_packet);

    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerBufferSize = " << PeerBufferSize << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerSamplingRate = " << PeerSamplingRate << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerBitResolution = " << PeerBitResolution << endl;
    cout << "--->JackTripWorker: PeerNumChannels = " << PeerNumChannels << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerConnectionMode = " << PeerConnectionMode << endl;

    mJackTrip->setNumChannels(PeerNumChannels);
    if (PeerConnectionMode == -1) {
        //Shut it down
        mSpawning = false;
        mUdpHubListener->releaseThread(mID);
    }
    
    // Connect signals and slots
    // -------------------------
    if (gVerboseFlag) cout << "---> JackTripWorker: Connecting signals and slots..." << endl;
    // Connection to terminate JackTrip when packets haven't arrive for
    // a certain amount of time
    connect(mJackTrip.data(), &JackTrip::signalNoUdpPacketsForSeconds, mJackTrip.data(),
            &JackTrip::slotStopProcesses, Qt::QueuedConnection);
    connect(mJackTrip.data(), &JackTrip::signalProcessesStopped, this, &JackTripWorker::jacktripStopped, Qt::QueuedConnection);
    connect(mJackTrip.data(), &JackTrip::signalError, this, &JackTripWorker::jacktripStopped, Qt::QueuedConnection);
    connect(mJackTrip.data(), &JackTrip::signalAudioStarted, this, &JackTripWorker::alertPatcher, Qt::QueuedConnection);
    connect(this, &JackTripWorker::signalRemoveThread, mJackTrip.data(), &JackTrip::slotStopProcesses, Qt::QueuedConnection);
    
    if (gVerboseFlag) cout << "---> JackTripWorker: startProcess..." << endl;
    mJackTrip->startProcess(
        #ifdef WAIRTOHUB // wair
                mID
        #endif // endwhere
                );
    mRunning = true;
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
    //Check if we've timed out.
    if (gTimeOutMultiThreadedServer > 0 && mElapsedTime >= gTimeOutMultiThreadedServer) {
        mTimeoutTimer.stop();
        mSpawning = false;
        mUdpHubListener->releaseThread(mID);
    }
}

void JackTripWorker::jacktripStopped()
{
    QMutexLocker lock(&mMutex);
    if (mSpawning || !mRunning) {
        //This has already been taken care of elsewhere.
        return;
    }
    mRunning = false;
    mUdpHubListener->releaseThread(mID, mAssignedClientName);
}

void JackTripWorker::alertPatcher()
{
    if (mRunning) {
        mAssignedClientName = mJackTrip->getAssignedClientName();
        mUdpHubListener->registerClientWithPatcher(mAssignedClientName);
    }
}
