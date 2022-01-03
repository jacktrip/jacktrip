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
 * \file DataProtocol.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#ifndef __DATAPROTOCOL_H__
#define __DATAPROTOCOL_H__

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#ifndef _WIN32
#include <arpa/inet.h>  //inet(3) functions
#include <netdb.h>
#include <netinet/in.h>  //sockaddr_in{} and other Internet defns
//#include <tr1/memory> //for shared_ptr
#endif

#include <QHostAddress>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <iostream>

class JackTrip;  // forward declaration

/** \brief Base class that defines the transmission protocol.
 *
 * This base class defines most of the common method to setup and connect
 * sockets using the individual protocols (UDP, TCP, SCTP, etc).
 *
 * The class has to be constructed using one of two modes (runModeT):\n
 * - SENDER
 * - RECEIVER
 *
 * This has to be specified as a constructor argument. When using, create two instances
 * of the class, one to receive and one to send packets. Each instance will run on a
 * separate thread.
 *
 * Redundancy and forward error correction should be implemented on each
 * Transport protocol, cause they depend on the protocol itself
 *
 * \todo This Class should contain definition of jacktrip header and basic funcionality to
 * obtain local machine IPs and maybe functions to manipulate IPs. Redundancy and forward
 * error correction should be implemented on each Transport protocol, cause they depend on
 * the protocol itself
 *
 * \todo The transport protocol itself has to be implemented subclassing this class, i.e.,
 * using a TCP or UDP protocol.
 *
 * Even if the underlined transmission protocol is stream oriented (as in TCP),
 * we send packets that are the size of the audio processing buffer.
 * Use AudioInterface::getBufferSize to obtain this value.
 *
 * Each transmission (i.e., inputs and outputs) run on its own thread.
 */
class DataProtocol : public QThread
{
    Q_OBJECT;

   public:
    //----------ENUMS------------------------------------------
    /// \brief Enum to define packet header types
    enum packetHeaderTypeT {
        DEFAULT,  ///< Default application header
        JAMLINK,  ///< Header to use with Jamlinks
        EMPTY     ///< Empty Header
    };

    /// \brief Enum to define class modes, SENDER or RECEIVER
    enum runModeT {
        SENDER,   ///< Set class as a Sender (send packets)
        RECEIVER  ///< Set class as a Receiver (receives packets)
    };
    //---------------------------------------------------------

    /** \brief The class constructor
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     * \param runmode Sets the run mode, use either DataProtocol::SENDER or
     * DataProtocol::RECEIVER
     * \param headertype packetHeaderTypeT header type to use for packets
     * \param bind_port Port number to bind for this socket (this is the receive or send
     * port depending on the runmode) \param peer_port Peer port number (this is the
     * receive or send port depending on the runmode)
     */
    DataProtocol(JackTrip* jacktrip, const runModeT runmode, int bind_port,
                 int peer_port);

    /// \brief The class destructor
    virtual ~DataProtocol();

    /** \brief Implements the thread loop
     *
     * Depending on the runmode, with will run a DataProtocol::SENDER thread or
     * DataProtocol::RECEIVER thread
     */
    virtual void run() = 0;

    /// \brief Stops the execution of the Thread
    virtual void stop()
    {
        QMutexLocker lock(&mMutex);
        mStopped = true;
    }

    /** \brief Sets the size of the audio part of the packets
     * \param size_bytes Size in bytes
     */
    void setAudioPacketSize(const size_t size_bytes) { mAudioPacketSize = size_bytes; }

    /** \brief Get the size of the audio part of the packets
     * \return size_bytes Size in bytes
     */
    size_t getAudioPacketSizeInBites() { return (mAudioPacketSize); }

    /** \brief Set the peer address
     * \param peerHostOrIP IPv4 number or host name
     * \todo implement here instead of in the subclass UDP
     */
    virtual void setPeerAddress(const char* peerHostOrIP) = 0;

    /** \brief Set the peer incomming (receiving) port number
     * \param port Port number
     * \todo implement here instead of in the subclass UDP
     */
    virtual void setPeerPort(int port) = 0;

    // virtual void getPeerAddressFromFirstPacket(QHostAddress& peerHostAddress,
    //				     uint16_t& port) = 0;

#if defined(_WIN32)
    virtual void setSocket(SOCKET& socket) = 0;
#else
    virtual void setSocket(int& socket) = 0;
#endif

    struct PktStat {
        uint32_t tot;
        uint32_t lost;
        uint32_t outOfOrder;
        uint32_t revived;
        uint32_t statCount;
    };
    virtual bool getStats(PktStat*) { return false; }

    virtual void setIssueSimulation(double /*loss*/, double /*jitter*/,
                                    double /*max_delay*/)
    {
    }
    void setUseRtPriority(bool use) { mUseRtPriority = use; }

   signals:

    void signalError(const char* error_message);
    void signalReceivedConnectionFromPeer();
    void signalCeaseTransmission(const QString& reason = "");

   protected:
    /** \brief Get the Run Mode of the object
     * \return SENDER or RECEIVER
     */
    runModeT getRunMode() const { return mRunMode; }

    /// Boolean stop the execution of the thread
    volatile bool mStopped;
    /// Boolean to indicate if the RECEIVER is waiting to obtain peer address
    volatile bool mHasPeerAddress;
    /// Boolean that indicates if a packet was received
    volatile bool mHasPacketsToReceive;
    QMutex mMutex;

   private:
    int mLocalPort;           ///< Local Port number to Bind
    int mPeerPort;            ///< Peer Port number to Bind
    const runModeT mRunMode;  ///< Run mode, either SENDER or RECEIVER

    struct sockaddr_in mLocalIPv4Addr;  ///< Local IPv4 Address struct
    struct sockaddr_in mPeerIPv4Addr;   ///< Peer IPv4 Address struct

    /// Number of clients running to check for ports already used
    /// \note Unimplemented, try to find another way to check for used ports
    static int sClientsRunning;

    size_t mAudioPacketSize;  ///< Packet audio part size

    /// \todo check a better way to access the header from the subclasses
   protected:
    // PacketHeader* mHeader; ///< Packet Header
    JackTrip* mJackTrip;  ///< JackTrip mediator class
    bool mUseRtPriority;
};

#endif
