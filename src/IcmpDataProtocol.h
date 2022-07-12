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
 * \file IcmpDataProtocol.h
 * \author Dominick Hing
 * \date July 2022
 */

#include <QHostAddress>
#include <QMutex>
#include <QThread>
#include <atomic>
#include <random>
#include <stdexcept>
#include <vector>

#include "DataProtocol.h"
#include "jacktrip_globals.h"
#include "jacktrip_types.h"

/** \brief ICMP implementation of DataProtocol class
 *
 */
class IcmpDataProtocol : public DataProtocol
{
    Q_OBJECT;

   public:
    /** \brief The class constructor
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     * \param runmode Sets the run mode, use either SENDER or RECEIVER
     */
    IcmpDataProtocol(JackTrip* jacktrip, const runModeT runmode);

    /** \brief The class destructor
     */
    virtual ~IcmpDataProtocol();

    /** \brief Set the Peer address to connect to
     * \param peerHostOrIP IPv4 number or host name
     */
    void setPeerAddress(const char* peerHostOrIP);

    void setPeerPort(int /* port */ ) { return; };


#if defined(_WIN32)
    void setSocket(SOCKET& socket);
#else
    void setSocket(int& socket);
#endif

    /** \brief Receives a packet. It blocks until a packet is received
     *
     * This function makes sure we receive a complete packet
     * of size n
     * \param buf Buffer to store the received packet
     * \param n size of packet to receive
     * \return number of bytes read, -1 on error
     */
    // virtual int receivePacket(char* buf, const size_t n);
    virtual int receivePacket(char* buf, const size_t n);

    /** \brief Sends a packet
     *
     * This function meakes sure we send a complete packet
     * of size n
     * \param buf Buffer to send
     * \param n size of packet to receive
     * \return number of bytes read, -1 on error
     */
    virtual int sendPacket(const char* buf, const size_t n);

    // /** \brief Sets the bind port number
    //  */
    // void setBindPort(int port) { mBindPort = port; }

    // /** \brief Sets the peer port number
    //  */
    // void setPeerPort(int port)
    // {
    //     mPeerPort            = port;
    //     mPeerAddr.sin_port   = htons(mPeerPort);
    //     mPeerAddr6.sin6_port = htons(mPeerPort);
    // }

    /** \brief Implements the Thread Loop. To start the thread, call start()
     * ( DO NOT CALL run() )
     *
     * This function creates and binds all the socket and start the connection loop
     * thread.
     */
    virtual void run();

    struct IcmpStat {
        uint32_t tot;
        uint32_t lost;
        uint32_t outOfOrder;
        uint32_t revived;
        uint32_t statCount;
    };

    bool getIcmpStats(IcmpStat*);
    bool resetIcmpStats(IcmpStat*) { return false; }

    // private:
   protected:
    /** \brief Binds the UDP socket to the available address and specified port
     */
#if defined(_WIN32)
    SOCKET bindSocket();
#else
    int bindSocket();
#endif

    /** \brief This function blocks until data is available for reading in the
     * socket. The function will timeout after timeout_msec microseconds.
     *
     * This function is intended to replace QAbstractSocket::waitForReadyRead which has
     * some problems with multithreading.
     *
     * \return returns true if there is data available for reading;
     * otherwise it returns false (if an error occurred or the operation timed out)
     */
    void waitForReady(int timeout_msec);


   private:
    bool datagramAvailable();

    // int mBindPort;            ///< Local Port number to Bind
    // int mPeerPort;            ///< Peer Port number
    const runModeT mRunMode;  ///< Run mode, either SENDER or RECEIVER
    bool mIPv6;               /// Use IPv6

    QHostAddress mPeerAddress;  ///< The Peer Address
    struct sockaddr_in mPeerAddr;
    struct sockaddr_in6 mPeerAddr6;
#if defined(_WIN32)
    SOCKET mSocket;
#else
    int mSocket;
#endif

    static QMutex sIcmpMutex;            ///< Mutex to make thread safe the binding process

    QMutex mIcmpStatsMutex;
    std::vector<double> mRttVector;
    uint32_t mReceivedCount;
    uint32_t mSentCount;
};

