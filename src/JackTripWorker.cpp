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
#ifdef __JAMTEST__
#include "JamTest.h"
#endif

using std::cout; using std::endl;

//*******************************************************************************
JackTripWorker::JackTripWorker(UdpHubListener* udphublistener, int BufferQueueLength, JackTrip::underrunModeT UnderRunMode, QString clientName) :
    mUdpHubListener(udphublistener),
    m_connectDefaultAudioPorts(false),
    mBufferQueueLength(BufferQueueLength),
    mUnderRunMode(UnderRunMode),
    mClientName(clientName),
    mSpawning(false),
    mID(0),
    mNumChans(1),
    mIOStatTimeout(0)
  #ifdef WAIR // wair
  ,mNumNetRevChans(0),
    mWAIR(false)
  #endif // endwhere
{
    setAutoDelete(false); // stick around after calling run()
    //mNetks = new NetKS;
    //mNetks->play();
    mBufferStrategy = 1;
    mBroadcastQueue = 0;
    mSimulatedLossRate = 0.0;
    mSimulatedJitterRate = 0.0;
    mSimulatedDelayRel = 0.0;
    mUseRtUdpPriority = false;
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
    { //Start Spawning, so lock mSpawning
        QMutexLocker locker(&mMutex);
        mSpawning = true;
    }
    mID = id;
    // Set the jacktrip address and ports
    //mClientAddress.setAddress(client_address);
    mClientAddress = client_address;
    mServerPort = server_port;
    mClientPort = client_port;
    mNumChans = num_channels;
    m_connectDefaultAudioPorts = connectDefaultAudioPorts;
}


//*******************************************************************************
void JackTripWorker::run()
{
    /* NOTE: This is the message that qt prints when an exception is thrown:
    'Qt Concurrent has caught an exception thrown from a worker thread.
    This is not supported, exceptions thrown in worker threads must be
    caught before control returns to Qt Concurrent.'*/

    { QMutexLocker locker(&mMutex); mSpawning = true; }

    //QHostAddress ClientAddress;

    // Try catching any exceptions that come from JackTrip
    try
    {
        // Local event loop. this is necesary because QRunnables don't have their own as QThreads
        QEventLoop event_loop;

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

        JackTrip jacktrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans,
                          mNumNetRevChans, FORCEBUFFERQ);
        JackTrip * mJackTrip = &jacktrip;
#else // endwhere
        JackTrip jacktrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans, mBufferQueueLength);
#endif // not wair

#ifdef WAIR // WAIR
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
#endif // endwhere
#endif // ifndef __JAMTEST__

#ifdef __JAMTEST__
        JamTest jacktrip(JackTrip::SERVERPINGSERVER); // ########### JamTest #################
        //JackTrip jacktrip(JackTrip::SERVERPINGSERVER, JackTrip::UDP, mNumChans, 2);
#endif

        jacktrip.setConnectDefaultAudioPorts(m_connectDefaultAudioPorts);

        // Set our underrun mode
        jacktrip.setUnderRunMode(mUnderRunMode);
        if (mIOStatTimeout > 0) {
            jacktrip.setIOStatTimeout(mIOStatTimeout);
            jacktrip.setIOStatStream(mIOStatStream);
        }
        
        if (!mClientName.isEmpty()) {
            jacktrip.setClientName(mClientName);
        }

        // Connect signals and slots
        // -------------------------
        if (gVerboseFlag) cout << "---> JackTripWorker: Connecting signals and slots..." << endl;
        // Connection to terminate JackTrip when packets haven't arrive for
        // a certain amount of time
        QObject::connect(&jacktrip, SIGNAL(signalNoUdpPacketsForSeconds()),
                         &jacktrip, SLOT(slotStopProcesses()), Qt::QueuedConnection);
        // Connection to terminate the local eventloop when jacktrip is done
        QObject::connect(&jacktrip, SIGNAL(signalProcessesStopped()),
                         &event_loop, SLOT(quit()), Qt::QueuedConnection);
        QObject::connect(&jacktrip, &JackTrip::signalError, &event_loop, &QEventLoop::quit, Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(signalRemoveThread()),
                         &jacktrip, SLOT(slotStopProcesses()), Qt::QueuedConnection);

        //ClientAddress.setAddress(mClientAddress);
        // If I don't type this line, I get a bus error in the next line.
        // I still haven't figure out why
        //ClientAddress.toString().toLatin1().constData();
        //jacktrip.setPeerAddress(ClientAddress.toString().toLatin1().constData());
        jacktrip.setPeerAddress(mClientAddress);
        jacktrip.setBindPorts(mServerPort);
        //jacktrip.setPeerPorts(mClientPort);
        jacktrip.setBufferStrategy(mBufferStrategy);
        jacktrip.setNetIssuesSimulation(mSimulatedLossRate,
            mSimulatedJitterRate, mSimulatedDelayRel);
        jacktrip.setBroadcast(mBroadcastQueue);
        jacktrip.setUseRtUdpPriority(mUseRtUdpPriority);

        if (gVerboseFlag) cout << "---> JackTripWorker: setJackTripFromClientHeader..." << endl;
        int PeerConnectionMode = setJackTripFromClientHeader(jacktrip);
        if ( PeerConnectionMode == -1 ) {
            mUdpHubListener->releaseThread(mID);
            { QMutexLocker locker(&mMutex); mSpawning = false; }
            return;
        }

        // Start Threads and event loop
        if (gVerboseFlag) cout << "---> JackTripWorker: startProcess..." << endl;
        jacktrip.startProcess(
            #ifdef WAIRTOHUB // wair
                    mID
            #endif // endwhere
                    );
        // if (gVerboseFlag) cout << "---> JackTripWorker: start..." << endl;
        // jacktrip.start(); // ########### JamTest Only #################

        // Thread is already spawning, so release the lock
        { QMutexLocker locker(&mMutex); mSpawning = false; }

        event_loop.exec(); // Excecution will block here until exit() the QEventLoop
        //--------------------------------------------------------------------------
        
        { QMutexLocker locker(&mMutex); mSpawning = true; }

        // wait for jacktrip to be done before exiting the Worker Thread
        //jacktrip.wait();

    }
    catch ( const std::exception & e )
    {
        std::cerr << "Couldn't send thread to the Pool" << endl;
        std::cerr << e.what() << endl;
        std::cerr << gPrintSeparator << endl;
        mUdpHubListener->releaseThread(mID);
        { QMutexLocker locker(&mMutex); mSpawning = false; }
        return;
    }

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
}


//*******************************************************************************
// returns -1 on error
int JackTripWorker::setJackTripFromClientHeader(JackTrip& jacktrip)
{
    //QHostAddress peerHostAddress;
    //uint16_t peer_port;
    QUdpSocket UdpSockTemp;// Create socket to wait for client

    // Bind the socket
    if ( !UdpSockTemp.bind(QHostAddress::Any, mServerPort,
                           QUdpSocket::DefaultForPlatform) )
    {
        std::cerr << "in JackTripWorker: Could not bind UDP socket. It may be already binded." << endl;
        throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
    }

    // Listen to client
    QWaitCondition sleep; // time is in milliseconds
    QMutex mutex;
    int sleepTime = 100; // ms
    int udpTimeout = gTimeOutMultiThreadedServer; // gTimeOutMultiThreadedServer mseconds
    int elapsedTime = 0;
    {
        QMutexLocker lock(&mutex);
        while ( (!UdpSockTemp.hasPendingDatagrams()) && (elapsedTime <= udpTimeout) ) {
            sleep.wait(&mutex,sleepTime);
            elapsedTime += sleepTime;
            if (gVerboseFlag) cout << "---------> ELAPSED TIME: " << elapsedTime << endl;
        }
    }
    // Check if we time out or not
    if (!UdpSockTemp.hasPendingDatagrams()) {
        std::cerr << "--->JackTripWorker: is not receiving Datagrams (timeout)" << endl;
        UdpSockTemp.close();
        return -1;
    }
    int packet_size = UdpSockTemp.pendingDatagramSize();
    char packet[packet_size];
    UdpSockTemp.readDatagram(packet, packet_size);
    UdpSockTemp.close(); // close the socket
    int8_t* full_packet = reinterpret_cast<int8_t*>(packet);

    int PeerBufferSize = jacktrip.getPeerBufferSize(full_packet);
    int PeerSamplingRate = jacktrip.getPeerSamplingRate(full_packet);
    int PeerBitResolution = jacktrip.getPeerBitResolution(full_packet);
    int PeerNumChannels = jacktrip.getPeerNumChannels(full_packet);
    int PeerConnectionMode = jacktrip.getPeerConnectionMode(full_packet);

    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerBufferSize = " << PeerBufferSize << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerSamplingRate = " << PeerSamplingRate << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerBitResolution = " << PeerBitResolution << endl;
    cout << "--->JackTripWorker: PeerNumChannels = " << PeerNumChannels << endl;
    if (gVerboseFlag) cout << "--->JackTripWorker: getPeerConnectionMode = " << PeerConnectionMode << endl;

    jacktrip.setNumChannels(PeerNumChannels);
    return PeerConnectionMode;
}


//*******************************************************************************
bool JackTripWorker::isSpawning()
{
    QMutexLocker locker(&mMutex);
    return mSpawning;
}


//*******************************************************************************
void JackTripWorker::stopThread()
{
    QMutexLocker locker(&mMutex);
    emit signalRemoveThread();
}
