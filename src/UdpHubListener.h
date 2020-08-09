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
 * \file UdpHubListener.h
 * \author Juan-Pablo Caceres and Chris Chafe
 * \date September 2008
 */

#ifndef __UDPHUBLISTENER_H__
#define __UDPHUBLISTENER_H__

#include <iostream>
#include <stdexcept>

#include <QThread>
#include <QThreadPool>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMutex>

#include "JackTrip.h"
#include "jacktrip_types.h"
#include "jacktrip_globals.h"
class JackTripWorker; // forward declaration
class Settings;

typedef struct {
    QString address;
    int16_t port;
} addressPortPair;

/** \brief Hub UDP listener on the Server.
 *
 * This creates a server that will listen on the well know port (the server port) and will
 * spawn JackTrip threads into the Thread pool. Clients request a connection.
 */
class UdpHubListener : public QThread
{
    Q_OBJECT;

public:
    UdpHubListener(int server_port = gServerUdpPort, int server_udp_port = NULL);
    virtual ~UdpHubListener();

    /// \brief Implements the Thread Loop. To start the thread, call start()
    /// ( DO NOT CALL run() )
    void run();

    /// \brief Stops the execution of the Thread
    void stop() { mStopped = true; }

    int releaseThread(int id);

    void setConnectDefaultAudioPorts(bool connectDefaultAudioPorts) { m_connectDefaultAudioPorts = connectDefaultAudioPorts; }

    void setSettings(Settings* s) {m_settings = s;}
    Settings* getSettings() const {return m_settings;}

private slots:
    void testReceive()
    { std::cout << "========= TEST RECEIVE SLOT ===========" << std::endl; }

signals:
    void Listening();
    void ClientAddressSet();
    void signalRemoveThread(int id);


private:
    /** \brief Binds a QUdpSocket. It chooses the available (active) interface.
   * \param udpsocket a QUdpSocket
   * \param port Port number
   */
    static void bindUdpSocket(QUdpSocket& udpsocket, int port);

    int readClientUdpPort(QTcpSocket* clientConnection);
    int sendUdpPort(QTcpSocket* clientConnection, int udp_port);


    /** \brief Send the JackTripWorker to the thread pool. This will run
   * until it's done. We still have control over the prototype class.
   * \param id Identification Number
   */
    //void sendToPoolPrototype(int id);

    /** \brief Check if address is already handled, if not add to array
   * \param address as string (IPv4 or IPv6)
   * \return -1 if address is busy, id number if not
   */
    int isNewAddress(QString address, uint16_t port);

    /** \brief Returns the ID of the client in the pool. If the client
    * is not in the pool yet, returns -1.
    */
    int getPoolID(QString address, uint16_t port);

    //QUdpSocket mUdpHubSocket; ///< The UDP socket
    //QHostAddress mPeerAddress; ///< The Peer Address

    //JackTripWorker* mJTWorker; ///< Class that will be used as prototype
    QVector<JackTripWorker*>* mJTWorkers; ///< Vector of JackTripWorker s
    QThreadPool mThreadPool; ///< The Thread Pool

    int mServerPort; //< Server known port number
    int mServerUdpPort; //< Server udp base port number
    int mBasePort;
    addressPortPair mActiveAddress[gMaxThreads]; ///< Active address pool addresses
    QHash<QString, uint16_t> mActiveAddressPortPair;

    /// Boolean stop the execution of the thread
    volatile bool mStopped;
    int mTotalRunningThreads; ///< Number of Threads running in the pool
    QMutex mMutex;
    JackTrip::underrunModeT mUnderRunMode;
    int mBufferQueueLength;

    bool m_connectDefaultAudioPorts;
    Settings* m_settings;

#ifdef WAIR // wair
    bool mWAIR;
    void connectMesh(bool spawn);
    void enumerateRunningThreadIDs();
public :
    void setWAIR(int b) {mWAIR = b;}
    bool isWAIR() {return mWAIR;}
#endif // endwhere
    void connectPatch(bool spawn);
public :
    unsigned int mHubPatch;
    void setHubPatch(unsigned int p) {mHubPatch = p;}
    unsigned int getHubPatch() {return mHubPatch;}

    void setUnderRunMode(JackTrip::underrunModeT UnderRunMode) { mUnderRunMode = UnderRunMode; }
    void setBufferQueueLength(int BufferQueueLength) { mBufferQueueLength = BufferQueueLength; }
};


#endif //__UDPHUBLISTENER_H__
