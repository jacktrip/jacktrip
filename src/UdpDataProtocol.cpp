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
 * \file UdpDataProtocol.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include "UdpDataProtocol.h"
#include "jacktrip_globals.h"
#include "JackTrip.h"

#include <QHostInfo>

#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <stdexcept>
#ifdef __WIN_32__
//#include <winsock.h>
#include <winsock2.h> //cc need SD_SEND
#include <ws2tcpip.h> // for IPv6
#endif
#if defined (__LINUX__) || (__MAC_OSX__)
#include <sys/socket.h> // for POSIX Sockets
#endif

using std::cout; using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

// sJackMutex definition
QMutex UdpDataProtocol::sUdpMutex;

//*******************************************************************************
UdpDataProtocol::UdpDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                                 int bind_port, int peer_port,
                                 unsigned int udp_redundancy_factor) :
    DataProtocol(jacktrip, runmode, bind_port, peer_port),
    mBindPort(bind_port), mPeerPort(peer_port),
    mRunMode(runmode),
    mAudioPacket(NULL), mFullPacket(NULL),
    mUdpRedundancyFactor(udp_redundancy_factor)
{
    mStopped = false;
    mIPv6 = false;
    std::memset(&mPeerAddr, 0, sizeof(mPeerAddr));
    std::memset(&mPeerAddr6, 0, sizeof(mPeerAddr6));
    mPeerAddr.sin_port = htons(mPeerPort);
    mPeerAddr6.sin6_port = htons(mPeerPort);
    
    if (mRunMode == RECEIVER) {
        QObject::connect(this, SIGNAL(signalWaitingTooLong(int)),
                         jacktrip, SLOT(slotUdpWaitingTooLongClientGoneProbably(int)), Qt::QueuedConnection);
    }
}


//*******************************************************************************
UdpDataProtocol::~UdpDataProtocol()
{
    delete[] mAudioPacket;
    delete[] mFullPacket;
    wait();
}


//*******************************************************************************
void UdpDataProtocol::setPeerAddress(const char* peerHostOrIP)
{
    // Get DNS Address
#if defined (__LINUX__) || (__MAC_OSX__)
    //Don't make the following code conditional on windows
    //(Addresses a weird timing bug when in hub client mode)
    if (!mPeerAddress.setAddress(peerHostOrIP)) {
#endif
        QHostInfo info = QHostInfo::fromName(peerHostOrIP);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            mPeerAddress = info.addresses().first();
        }
        //cout << "UdpDataProtocol::setPeerAddress IP Address Number: "
        //    << mPeerAddress.toString().toStdString() << endl;
#if defined (__LINUX__) || (__MAC_OSX__)
    }
#endif

    // check if the ip address is valid
    if ( mPeerAddress.protocol() == QAbstractSocket::IPv6Protocol ) {
        mIPv6 = true;
    } else  if ( mPeerAddress.protocol() != QAbstractSocket::IPv4Protocol ) {
        QString error_message = "Incorrect presentation format address\n '";
        error_message.append(peerHostOrIP);
        error_message.append("' is not a valid IP address or Host Name");
        //std::cerr << "ERROR: Incorrect presentation format address" << endl;
        //std::cerr << "'" << peerHostOrIP <<"' does not seem to be a valid IP address" << endl;
        //throw std::invalid_argument("Incorrect presentation format address");
        throw std::invalid_argument( error_message.toStdString());
    }
    /*
    else {
        std::cout << "Peer Address set to: "
            << mPeerAddress.toString().toStdString() << std::endl;
        cout << gPrintSeparator << endl;
        usleep(100);
    }
    */

    // Save our address as an appropriate address structure
    if (mIPv6) {
        mPeerAddr6.sin6_family = AF_INET6;
        ::inet_pton(AF_INET6, mPeerAddress.toString().toLatin1().constData(),
                    &mPeerAddr6.sin6_addr);
    } else {
        mPeerAddr.sin_family = AF_INET;
        ::inet_pton(AF_INET, mPeerAddress.toString().toLatin1().constData(),
                    &mPeerAddr.sin_addr);
    }
}

#if defined (__WIN_32__)
void UdpDataProtocol::setSocket(SOCKET &socket)
#else
void UdpDataProtocol::setSocket(int &socket)
#endif
{
    //If we haven't been passed a valid socket, then we should bind one.
#if defined (__WIN_32__)
    if (socket == INVALID_SOCKET) {
#else
    if (socket == -1) {
#endif
        try {
            if (gVerboseFlag) std::cout << "    UdpDataProtocol:run" << mRunMode << " before bindSocket(UdpSocket)" << std::endl;
            socket = bindSocket(); // Bind Socket
        } catch ( const std::exception & e ) {
            emit signalError( e.what() );
            return;
        }
    }
    mSocket = socket;
}


//*******************************************************************************
#if defined (__WIN_32__)
SOCKET UdpDataProtocol::bindSocket()
#else
int UdpDataProtocol::bindSocket()
#endif
{
    QMutexLocker locker(&sUdpMutex);

#if defined __WIN_32__
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        // Tell the user that we couldn't find a useable
        // winsock.dll.

        return INVALID_SOCKET;
    }

    // Confirm that the Windows Sockets DLL supports 1.1. or higher

    if ( LOBYTE( wsaData.wVersion ) != 2 ||
         HIBYTE( wsaData.wVersion ) != 2 ) {
        // Tell the user that we couldn't find a useable
        // winsock.dll.
        WSACleanup( );
        return INVALID_SOCKET;
    }

    SOCKET sock_fd;
#endif

#if defined ( __LINUX__ ) || (__MAC_OSX__)
    int sock_fd;
#endif

    //Set local IPv4 or IPv6 Address
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;

    // Create socket descriptor
    if (mIPv6) {
        sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);
        std::memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr = in6addr_any;
        local_addr6.sin6_port = htons(mBindPort);
    } else {
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

        //::bzero(&local_addr, sizeof(local_addr));
        std::memset(&local_addr, 0, sizeof(local_addr)); // set buffer to 0
        local_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
        local_addr.sin_port = htons(mBindPort); //set local port
    }

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
#if defined (__WIN_32__)
    //make address/port reusable
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
#endif

    // Bind the Socket
    if (mIPv6) {
        if ( (::bind(sock_fd, (struct sockaddr *) &local_addr6, sizeof(local_addr6))) < 0 )
        { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }
    } else {
        if ( (::bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr))) < 0 )
        { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }
    }

    // To be able to use the two UDP sockets bound to the same port number,
    // we connect the receiver and issue a SHUT_WR.

    // This didn't work for IPv6, so we'll instead share a full duplex socket.
    /*if (mRunMode == SENDER) {
        // We use the sender as an unconnected UDP socket
        UdpSocket.setSocketDescriptor(sock_fd, QUdpSocket::BoundState,
                                      QUdpSocket::WriteOnly);
    }*/
    if (!mIPv6) {
        // Connect only if we're using IPv4.
        // (Connecting presents an issue when a host has multiple IP addresses and the peer decides to send from
        // a different address. While this generally won't be a problem for IPv4, it will for IPv6.)
        if ( (::connect(sock_fd, (struct sockaddr *) &mPeerAddr, sizeof(mPeerAddr))) < 0)
        { throw std::runtime_error("ERROR: Could not connect UDP socket"); }
#if defined (__LINUX__) || (__MAC_OSX__)
        //if ( (::shutdown(sock_fd,SHUT_WR)) < 0)
        //{ throw std::runtime_error("ERROR: Could shutdown SHUT_WR UDP socket"); }
#endif
#if defined __WIN_32__
        /*int shut_sr = shutdown(sock_fd, SD_SEND);  //shut down sender's receive function
        if ( shut_sr< 0)
        {
            fprintf(stderr, "ERROR: Could not shutdown SD_SEND UDP socket");
            throw std::runtime_error("ERROR: Could not shutdown SD_SEND UDP socket");
        }*/
#endif
    }

    return sock_fd;

    // OLD CODE WITHOUT POSIX FIX--------------------------------------------------
    /*
  /// \todo if port is already used, try binding in a different port
  QUdpSocket::BindMode bind_mode;
  if (mRunMode == RECEIVER) {
    bind_mode = QUdpSocket::DontShareAddress; }
  else if (mRunMode == SENDER) { //Share sender socket
    bind_mode = QUdpSocket::ShareAddress; }

  // QHostAddress::Any : let the kernel decide the active address
  if ( !UdpSocket.bind(QHostAddress::Any, mBindPort, bind_mode) ) {
    throw std::runtime_error("Could not bind UDP socket. It may be already binded.");
  }
  else {
    if ( mRunMode == RECEIVER ) {
      cout << "UDP Socket Receiving in Port: " << mBindPort << endl;
      cout << gPrintSeparator << endl;
    }
  }
  */
    // ----------------------------------------------------------------------------
}


//*******************************************************************************
int UdpDataProtocol::receivePacket(QUdpSocket& UdpSocket, char* buf, const size_t n)
{
    // Block until There's something to read
    while ( (UdpSocket.pendingDatagramSize() < n) && !mStopped ) { QThread::usleep(100); }
    int n_bytes = UdpSocket.readDatagram(buf, n);
    return n_bytes;
}


//*******************************************************************************
int UdpDataProtocol::sendPacket(const char* buf, const size_t n)
{
/*#if defined (__WIN_32__)
    //Alternative windows specific code that uses winsock equivalents of the bsd socket functions.
    DWORD n_bytes;
    WSABUF buffer;
    int error;
    buffer.len = n;
    buffer.buf = (char *)buf;

    if (mIPv6) {
        error = WSASendTo(mSocket, &buffer, 1, &n_bytes, 0, (struct sockaddr *) &mPeerAddr6, sizeof(mPeerAddr6), 0, 0);
    } else {
        error = WSASend(mSocket, &buffer, 1, &n_bytes, 0, 0, 0);
    }
    if (error == SOCKET_ERROR) {
        cout << "Socket Error: " << WSAGetLastError() << endl;
    }
    return (int)n_bytes;
#else*/
    int n_bytes;
    if (mIPv6) {
        n_bytes = ::sendto(mSocket, buf, n, 0, (struct sockaddr *) &mPeerAddr6, sizeof(mPeerAddr6));
    } else {
        n_bytes = ::send(mSocket, buf, n, 0);
    }
    return n_bytes;
//#endif
}


//*******************************************************************************
void UdpDataProtocol::getPeerAddressFromFirstPacket(QUdpSocket& UdpSocket,
                                                    QHostAddress& peerHostAddress,
                                                    uint16_t& port)
{
    while ( !UdpSocket.hasPendingDatagrams() ) {
        msleep(100);
    }
    char buf[1];
    UdpSocket.readDatagram(buf, 1, &peerHostAddress, &port);
}


//*******************************************************************************
void UdpDataProtocol::run()
{
    if (gVerboseFlag) switch ( mRunMode )
    {
    case RECEIVER : {
        std::cout << "step 3" << std::endl;
        break; }

    case SENDER : {
        std::cout << "step 4" << std::endl;
        break; }
    }

    //QObject::connect(this, SIGNAL(signalError(const char*)),
    //                 mJackTrip, SLOT(slotStopProcesses()),
    //                 Qt::QueuedConnection);

    //Wrap our socket in a QUdpSocket object if we're the receiver, for convenience.
    //If we're the sender, we'll just write directly to our socket.
    QUdpSocket UdpSocket;
    if (mRunMode == RECEIVER) {
        if (mIPv6) {
            UdpSocket.setSocketDescriptor(mSocket, QUdpSocket::BoundState,
                                          QUdpSocket::ReadOnly);
        } else {
            UdpSocket.setSocketDescriptor(mSocket, QUdpSocket::ConnectedState,
                                          QUdpSocket::ReadOnly);
        }
        cout << "UDP Socket Receiving in Port: " << mBindPort << endl;
        cout << gPrintSeparator << endl;
    }

    if (gVerboseFlag) std::cout << "    UdpDataProtocol:run" << mRunMode << " before Setup Audio Packet buffer, Full Packet buffer, Redundancy Variables" << std::endl;
    // Setup Audio Packet buffer
    size_t audio_packet_size = getAudioPacketSizeInBites();
    //cout << "audio_packet_size: " << audio_packet_size << endl;
    mAudioPacket = new int8_t[audio_packet_size];
    std::memset(mAudioPacket, 0, audio_packet_size); // set buffer to 0

    // Setup Full Packet buffer
    int full_packet_size = mJackTrip->getPacketSizeInBytes();
    //cout << "full_packet_size: " << full_packet_size << endl;
    mFullPacket = new int8_t[full_packet_size];
    std::memset(mFullPacket, 0, full_packet_size); // set buffer to 0

    //  bool timeout = false; // Time out flag for packets that arrive too late

    // Put header in first packet
    mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);

    // Redundancy Variables
    // (Algorithm explained at the end of this file)
    // ---------------------------------------------
    int full_redundant_packet_size = full_packet_size * mUdpRedundancyFactor;
    int8_t* full_redundant_packet;
    full_redundant_packet = new int8_t[full_redundant_packet_size];
    std::memset(full_redundant_packet, 0, full_redundant_packet_size); // Initialize to 0

    // Set realtime priority (function in jacktrip_globals.h)
    if (gVerboseFlag) std::cout << "    UdpDataProtocol:run" << mRunMode << " before setRealtimeProcessPriority()" << std::endl;
    //std::cout << "Experimental version -- not using setRealtimeProcessPriority()" << std::endl;
    //setRealtimeProcessPriority();

    /////////////////////
    // to see thread priorities
    // sudo ps -eLo pri,rtprio,cls,pid,nice,cmd | grep -E 'jackd|jacktrip|rtc|RTPRI' | sort -r

    // from David Runge

    //  It seems that it tries to apply the highest available SCHED_FIFO to
    //  jacktrip or half of it (?) [1] (although that's not what you would want,
    //  as this would mean assigning a higher priority to jacktrip than e.g. to
    //  the audio interface and e.g. IRQs that need to be taken care of).

    //  The version on github [2] (current 1.1) is actually worse off, as it
    //  just hardcodes RTPRIO 99 (which means jacktrip will compete with the
    //  Linux kernel watchdog, if the user trying to launch jacktrip is even
    //  allowed to use that high of a priority!).
    //  On most systems this will not work at all (aside from it being outright
    //  dangerous). On Arch (and also Ubuntu) the sane default is to allow
    //  rtprio 95 to a privileged user group (e.g. 'realtime' or 'audio', etc.)

    //  It would be very awesome, if setting the priority would be dealt with by
    //  a command line flag to jacktrip (e.g. `jacktrip --priority=50`) and
    //  otherwise defaulting to a much much lower number (e.g. 10), so the
    //  application can be run out-of-the-box (even without being in a
    //  privileged group).

    // from Nando

    //  You should actually be using the priority that jack gives you when you
    //  create the realtime thread, that puts your process "behind" - so to
    //  speak - the processing that jack does on behalf of all its clients, and
    //  behind (in a properly configured system) the audio interface processing
    //  interrupt. No need to select a priority yourself.

    //  In a Fedora system I run jack with a priority of 65 (the Fedora packages
    //  changed the default to a much lower one which is a big no-no). The
    //  clients inherit 60, I think. Some clients that have their own internal
    //  structure of processes (jconvolver) run multiple threads and use
    //  priorities below 60 for them (ie: they start with what jack gave them).

    //  If you need to run a thread (not the audio thread) with higher priority
    //  you could retrieve the priority that jack gave you and add some magic
    //  number to get it to be above jack itself (10 would be fine in my
    //  experience).

    //without setting it
    //        PRI RTPRIO CLS   PID  NI CMD
    //         60     20  FF  4348   - /usr/bin/jackd -dalsa -dhw:CODEC -r48000 -p128 -n2 -Xseq
    //         55     15  FF  9835   - ./jacktrip -s
    //         19      -  TS  9835   0 ./jacktrip -s
    //         19      -  TS  9835   0 ./jacktrip -s
    //         19      -  TS  9835   0 ./jacktrip -s
    //         19      -  TS  9835   0 ./jacktrip -s
    //         19      -  TS  9835   0 ./jacktrip -s
    //         19      -  TS  4348   0 /usr/bin/jackd -dalsa -dhw:CODEC -r48000 -p128 -n2 -Xseq
    //         19      -  TS  4348   0 /usr/bin/jackd -dalsa -dhw:CODEC -r48000 -p128 -n2 -Xseq
    //         19      -  TS  4348   0 /usr/bin/jackd -dalsa -dhw:CODEC -r48000 -p128 -n2 -Xseq
    //         19      -  TS  4348   0 /usr/bin/jackd -dalsa -dhw:CODEC -r48000 -p128 -n2 -Xseq

    // jack puts its clients in FF at 5 points below itself

    switch ( mRunMode )
    {
    case RECEIVER : {
        // Connect signals and slots for packets arriving too late notifications
        QObject::connect(this, SIGNAL(signalWaitingTooLong(int)),
                         this, SLOT(printUdpWaitedTooLong(int)),
                         Qt::QueuedConnection);
        //-----------------------------------------------------------------------------------
        // Wait for the first packet to be ready and obtain address
        // from that packet
        if (gVerboseFlag) std::cout << "    UdpDataProtocol:run" << mRunMode << " before !UdpSocket.hasPendingDatagrams()" << std::endl;
        std::cout << "Waiting for Peer..." << std::endl;
        // This blocks waiting for the first packet
        while ( !UdpSocket.hasPendingDatagrams() ) {
            if (mStopped) { return; }
            QThread::msleep(100);
            if (gVerboseFlag) std::cout << "100ms  " << std::flush;
        }
        int first_packet_size = UdpSocket.pendingDatagramSize();
        // The following line is the same as
        int8_t* first_packet = new int8_t[first_packet_size];
        /// \todo fix this to avoid memory leaks
        // but avoids memory leaks
        //std::tr1::shared_ptr<int8_t> first_packet(new int8_t[first_packet_size]);
        receivePacket( UdpSocket, reinterpret_cast<char*>(first_packet), first_packet_size);
        // Check that peer has the same audio settings
        if (gVerboseFlag) std::cout << std::endl << "    UdpDataProtocol:run" << mRunMode << " before mJackTrip->checkPeerSettings()" << std::endl;
        mJackTrip->checkPeerSettings(first_packet);
        if (gVerboseFlag) std::cout << "step 7" << std::endl;
        if (gVerboseFlag) std::cout << "    UdpDataProtocol:run" << mRunMode << " before mJackTrip->parseAudioPacket()" << std::endl;
        mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
        std::cout << "Received Connection from Peer!" << std::endl;
        emit signalReceivedConnectionFromPeer();

        // Redundancy Variables
        // --------------------
        // NOTE: These types need to be the same unsigned integer as the sequence
        // number in the header. That way, they wrap around in the "same place"
        uint16_t current_seq_num = 0; // Store current sequence number
        uint16_t last_seq_num = 0;    // Store last package sequence number
        uint16_t newer_seq_num = 0;   // Store newer sequence number
        mTotCount = 0;
        mLostCount = 0;
        mOutOfOrderCount = 0;
        mRevivedCount = 0;
        mStatCount = 0;

        if (gVerboseFlag) std::cout << "step 8" << std::endl;
        while ( !mStopped )
        {
            // Timer to report packets arriving too late
            // This QT method gave me a lot of trouble, so I replaced it with my own 'waitForReady'
            // that uses signals and slots and can also report with packets have not
            // arrive for a longer time
            //timeout = UdpSocket.waitForReadyRead(30);
            //        timeout = cc unused!
            waitForReady(UdpSocket, 60000); //60 seconds

            // OLD CODE WITHOUT REDUNDANCY----------------------------------------------------
            /*
        // This is blocking until we get a packet...
        receivePacket( UdpSocket, reinterpret_cast<char*>(mFullPacket), full_packet_size);

        mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);

        // ...so we want to send the packet to the buffer as soon as we get in from
        // the socket, i.e., non-blocking
        //mRingBuffer->insertSlotNonBlocking(mAudioPacket);
        mJackTrip->writeAudioBuffer(mAudioPacket);
        */
            //----------------------------------------------------------------------------------
            receivePacketRedundancy(UdpSocket,
                                    full_redundant_packet,
                                    full_redundant_packet_size,
                                    full_packet_size,
                                    current_seq_num,
                                    last_seq_num,
                                    newer_seq_num);
        }
        break; }

    case SENDER : {
        while ( !mStopped )
        {
            // OLD CODE WITHOUT REDUNDANCY -----------------------------------------------------
            /*
        // We block until there's stuff available to read
        mJackTrip->readAudioBuffer( mAudioPacket );
        mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);
        // This will send the packet immediately
        //int bytes_sent = sendPacket( reinterpret_cast<char*>(mFullPacket), full_packet_size);
        sendPacket( UdpSocket, PeerAddress, reinterpret_cast<char*>(mFullPacket), full_packet_size);
        */
            //----------------------------------------------------------------------------------
            sendPacketRedundancy(full_redundant_packet,
                                 full_redundant_packet_size,
                                 full_packet_size);
        }
        break; }
    }
}


//*******************************************************************************
//bool
void UdpDataProtocol::waitForReady(QUdpSocket& UdpSocket, int timeout_msec)
{
    int loop_resolution_usec = 100; // usecs to wait on each loop
    int emit_resolution_usec = 10000; // 10 milliseconds
    int timeout_usec = timeout_msec * 1000;
    int elapsed_time_usec = 0; // Ellapsed time in milliseconds

    while ( ( !(
                  UdpSocket.hasPendingDatagrams() &&
                  (UdpSocket.pendingDatagramSize() > 0)
                  ) && (elapsed_time_usec <= timeout_usec) )
            && !mStopped ){
        //    if (mStopped) { return false; }
        QThread::usleep(loop_resolution_usec);
        elapsed_time_usec += loop_resolution_usec;

        if ( !(elapsed_time_usec % emit_resolution_usec) ) {
            emit signalWaitingTooLong(static_cast<int>(elapsed_time_usec/1000));
        }
    }
    // cc under what condition?
    //  if ( elapsed_time_usec >= timeout_usec )
    //  {
    //    emit signalWaitingTooLong(elapsed_time_usec/1000);
    //    return false;
    //  }
    //  return true;
}


//*******************************************************************************
void UdpDataProtocol::printUdpWaitedTooLong(int wait_msec)
{
    int wait_time = 30; // msec
    if ( !(wait_msec%wait_time) ) {
        std::cerr << "UDP waiting too long (more than " << wait_time << "ms) for " << mPeerAddress.toString().toStdString() << "..." << endl;
    }
}


//*******************************************************************************
void UdpDataProtocol::receivePacketRedundancy(QUdpSocket& UdpSocket,
                                              int8_t* full_redundant_packet,
                                              int full_redundant_packet_size,
                                              int full_packet_size,
                                              uint16_t& current_seq_num,
                                              uint16_t& last_seq_num,
                                              uint16_t& newer_seq_num)
{
    // This is blocking until we get a packet...
    receivePacket( UdpSocket, reinterpret_cast<char*>(full_redundant_packet),
                   full_redundant_packet_size);

    // Get Packet Sequence Number
    newer_seq_num =
            mJackTrip->getPeerSequenceNumber(full_redundant_packet);
    if (gVerboseFlag) cout << "current_seq_num: " << current_seq_num << " ; newer_seq_num: " << newer_seq_num << endl;
    current_seq_num = newer_seq_num;


    if (0 != last_seq_num) {
        int16_t lost = newer_seq_num - last_seq_num - 1;
        if (0 > lost) {
            // Out of order packet, should be ignored
            ++mOutOfOrderCount;
            if (gVerboseFlag) cout << "mOutOfOrderCount = " << mOutOfOrderCount << " ; ";
            return;
        }
        else if (0 != lost) {
            mLostCount += lost;
            if (gVerboseFlag) cout << "mLostCount = " << mLostCount << " ; ";
        }
        mTotCount += 1 + lost;
    }

    //cout << current_seq_num << " ";
    int redun_last_index = 0;
    for (unsigned int i = 1; i<mUdpRedundancyFactor; i++) {
        // Check if the package we receive is the next one expected, i.e.,
        // current_seq_num == (last_seq_num+1)
        if ( current_seq_num == (last_seq_num+1) ) { break; }

        // if it's not, check the next one until it is the corresponding packet
        // or there aren't more available packets
        redun_last_index = i; // index of packet to use in the redundant packet
        current_seq_num =
                mJackTrip->getPeerSequenceNumber( full_redundant_packet + (i*full_packet_size) );
        //cout << current_seq_num << " ";
    }
    mRevivedCount += redun_last_index;
    //cout << endl;

    last_seq_num = newer_seq_num; // Save last read packet

    // Send to audio all available audio packets, in order
    for (int i = redun_last_index; i>=0; i--) {
        memcpy(mFullPacket,
               full_redundant_packet + (i*full_packet_size),
               full_packet_size);
        mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
        mJackTrip->writeAudioBuffer(mAudioPacket);
    }
}

//*******************************************************************************
bool UdpDataProtocol::getStats(DataProtocol::PktStat* stat)
{
    if (0 == mStatCount) {
        mLostCount = 0;
        mOutOfOrderCount = 0;
        mRevivedCount = 0;
    }
    stat->tot = mTotCount;
    stat->lost = mLostCount;
    stat->outOfOrder = mOutOfOrderCount;
    stat->revived = mRevivedCount;
    stat->statCount = mStatCount++;
    return true;
}

//*******************************************************************************
void UdpDataProtocol::sendPacketRedundancy(int8_t* full_redundant_packet,
                                           int full_redundant_packet_size,
                                           int full_packet_size)
{
    mJackTrip->readAudioBuffer( mAudioPacket );
    mJackTrip->putHeaderInPacket(mFullPacket, mAudioPacket);

    // Move older packets to end of array of redundant packets
    std::memmove(full_redundant_packet+full_packet_size,
                 full_redundant_packet,
                 full_packet_size*(mUdpRedundancyFactor-1));
    // Copy new packet to the begining of array
    std::memcpy(full_redundant_packet,
                mFullPacket, full_packet_size);

    // 10% (or other number) packet lost simulation.
    // Uncomment the if to activate
    //---------------------------------------------------------------------------------
    //int random_integer = rand();
    //if ( random_integer > (RAND_MAX/10) )
    //{
    sendPacket( reinterpret_cast<char*>(full_redundant_packet),
                full_redundant_packet_size);
    //}
    //---------------------------------------------------------------------------------

    mJackTrip->increaseSequenceNumber();
}


/*
  The Redundancy Algorythmn works as follows. We send a packet that contains
  a mUdpRedundancyFactor number of packets (header+audio). This big packet looks
  as follows

  ----------  ------------       -----------------------------------
  | UDP[n] |  | UDP[n-1] |  ...  | UDP[n-(mUdpRedundancyFactor-1)] |
  ----------  ------------       -----------------------------------

  Then, for the new audio buffer, we shift everything to the right and send:

  ----------  ------------       -------------------------------------
  | UDP[n+1] |  | UDP[n] |  ...  | UDP[n-(mUdpRedundancyFactor-1)+1] |
  ----------  ------------       -------------------------------------

  etc...

  For a redundancy factor of 4, this will look as follows:
  ----------  ----------  ----------  ----------
  | UDP[4] |  | UDP[3] |  | UDP[2] |  | UDP[1] |
  ----------  ----------  ----------  ----------

  ----------  ----------  ----------  ----------
  | UDP[5] |  | UDP[4] |  | UDP[3] |  | UDP[2] |
  ----------  ----------  ----------  ----------

  ----------  ----------  ----------  ----------
  | UDP[6] |  | UDP[5] |  | UDP[4] |  | UDP[3] |
  ----------  ----------  ----------  ----------

  etc...

  Then, the receiving end checks if the firs packet in the list is the one it should use,
  otherwise it continure reding the mUdpRedundancyFactor packets until it finds the one that
  should come next (this can better perfected by just jumping until the correct packet).
  If it has more than one packet that it hasn't yet received, it sends it to the soundcard
  one by one.
*/
