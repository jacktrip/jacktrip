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
 * \file UdpDataProtocol.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

// #define MANUAL_POLL

#include "UdpDataProtocol.h"

#include <QHostInfo>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "JackTrip.h"
#include "jacktrip_globals.h"
#ifdef _WIN32
// #include <winsock.h>
#include <qos2.h>
#include <stdio.h>
#include <winsock2.h>  //cc need SD_SEND
#pragma comment(lib, "ws2_32.lib")
#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#else
#include <fcntl.h>
#include <sys/socket.h>  // for POSIX Sockets
#include <unistd.h>
#ifndef MANUAL_POLL
#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif  // __linux__
#endif  // MANUAL_POLL
#endif  // _WIN32

using std::cout;
using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

// sJackMutex definition
QMutex UdpDataProtocol::sUdpMutex;

//*******************************************************************************
UdpDataProtocol::UdpDataProtocol(JackTrip* jacktrip, const runModeT runmode,
                                 int bind_port, int peer_port,
                                 unsigned int udp_redundancy_factor)
    : DataProtocol(jacktrip, runmode, bind_port, peer_port)
    , mBindPort(bind_port)
    , mPeerPort(peer_port)
    , mRunMode(runmode)
#if defined(_WIN32)
    , mSocket(INVALID_SOCKET)
#else
    , mSocket(-1)
#endif
    , mAudioPacket(NULL)
    , mFullPacket(NULL)
    , mUdpRedundancyFactor(udp_redundancy_factor)
    , mControlPacketSize(63)
    , mStopSignalSent(false)
{
    mIPv6 = false;
    std::memset(&mPeerAddr, 0, sizeof(mPeerAddr));
    std::memset(&mPeerAddr6, 0, sizeof(mPeerAddr6));
    mPeerAddr.sin_port   = htons(mPeerPort);
    mPeerAddr6.sin6_port = htons(mPeerPort);

    if (mRunMode == RECEIVER) {
        QObject::connect(this, &UdpDataProtocol::signalWaitingTooLong, jacktrip,
                         &JackTrip::slotUdpWaitingTooLongClientGoneProbably,
                         Qt::QueuedConnection);
    }
    mSimulatedLossRate       = 0.0;
    mSimulatedJitterRate     = 0.0;
    mSimulatedJitterMaxDelay = 0.0;
}

//*******************************************************************************
UdpDataProtocol::~UdpDataProtocol()
{
    delete[] mAudioPacket;
    delete[] mFullPacket;
    closeSocket();
    wait();
}

//*******************************************************************************
void UdpDataProtocol::stop()
{
    closeSocket();
    DataProtocol::stop();
}

//*******************************************************************************
void UdpDataProtocol::closeSocket()
{
    if (mRunMode != RECEIVER) {
        return;
    }
#if defined(_WIN32)
    if (mSocket != INVALID_SOCKET) {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
#else
    if (mSocket != -1) {
        ::close(mSocket);
        mSocket = -1;
    }
#endif
}

//*******************************************************************************
void UdpDataProtocol::setPeerAddress(const char* peerHostOrIP)
{
    // Get DNS Address
    if (!mPeerAddress.setAddress(peerHostOrIP)) {
        QHostInfo info = QHostInfo::fromName(peerHostOrIP);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            mPeerAddress = info.addresses().constFirst();
        }
        // cout << "UdpDataProtocol::setPeerAddress IP Address Number: "
        //    << mPeerAddress.toString().toStdString() << endl;
    }

    // check if the ip address is valid
    if (mPeerAddress.protocol() == QAbstractSocket::IPv6Protocol) {
        mIPv6 = true;
    } else if (mPeerAddress.protocol() != QAbstractSocket::IPv4Protocol) {
        QString error_message =
            QStringLiteral("Incorrect presentation format address\n'");
        error_message.append(peerHostOrIP);
        error_message.append("' is not a valid IP address or Host Name");
        // std::cerr << "ERROR: Incorrect presentation format address" << endl;
        // std::cerr << "'" << peerHostOrIP <<"' does not seem to be a valid IP address"
        // << endl; throw std::invalid_argument("Incorrect presentation format address");
        throw std::invalid_argument(error_message.toStdString());
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

void UdpDataProtocol::setSocket(socket_type& socket)
{
    // If we haven't been passed a valid socket, then we should bind one.
#if defined(_WIN32)
    if (socket == INVALID_SOCKET) {
#else
    if (socket == -1) {
#endif
        try {
            if (gVerboseFlag)
                std::cout << "    UdpDataProtocol:run" << mRunMode << " before bindSocket"
                          << std::endl;
            socket = bindSocket();  // Bind Socket
        } catch (const std::exception& e) {
            emit signalError(e.what());
            return;
        }
    }
    mSocket = socket;
}

//*******************************************************************************
socket_type UdpDataProtocol::bindSocket()
{
    QMutexLocker locker(&sUdpMutex);

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        // Tell the user that we couldn't find a usable
        // winsock.dll.

        return INVALID_SOCKET;
    }

    // Confirm that the Windows Sockets DLL supports 1.1. or higher

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        // Tell the user that we couldn't find a usable
        // winsock.dll.
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKET sock_fd;
#else
    int sock_fd;
#endif

    // Set local IPv4 or IPv6 Address
    struct sockaddr_in local_addr;
    struct sockaddr_in6 local_addr6;

    // Create socket descriptor
    if (mIPv6) {
        sock_fd = socket(AF_INET6, SOCK_DGRAM, 0);
        std::memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr   = in6addr_any;
        local_addr6.sin6_port   = htons(mBindPort);
    } else {
        sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

        //::bzero(&local_addr, sizeof(local_addr));
        std::memset(&local_addr, 0, sizeof(local_addr));  // set buffer to 0
        local_addr.sin_family = AF_INET;                  // AF_INET: IPv4 Protocol
        local_addr.sin_addr.s_addr =
            htonl(INADDR_ANY);  // INADDR_ANY: let the kernel decide the active address
        local_addr.sin_port = htons(mBindPort);  // set local port
    }

    // Prevent WSAECONNRESET errors that occur on Windows due to async UDP port setup
#if defined(_WIN32)
    BOOL bNewBehavior     = FALSE;
    DWORD dwBytesReturned = 0;
    WSAIoctl(sock_fd, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0,
             &dwBytesReturned, NULL, NULL);
#endif

    // Set socket to be reusable, this is platform dependent
    int one = 1;
#if defined(_WIN32)
    // make address/port reusable
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
#elif defined(__linux__)
    ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
#else
    // This option is not available on Linux, and without it MAC OS X
    // has problems rebinding a socket
    ::setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
#endif

    // set quality of service for the UDP socket
    if (setSocketQos(sock_fd)) {
        std::cout << "Set QoS for network socket" << std::endl;
    } else {
        std::cerr << "Failed to set QoS for network socket" << std::endl;
    }

    // Bind the Socket
    if (mIPv6) {
        if ((::bind(sock_fd, (struct sockaddr*)&local_addr6, sizeof(local_addr6))) < 0) {
            throw std::runtime_error("ERROR: UDP Socket Bind Error");
        }
    } else {
        if ((::bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr))) < 0) {
            throw std::runtime_error("ERROR: UDP Socket Bind Error");
        }
    }

    // Return our file descriptor so the socket can be shared for a
    // full duplex connection.
    return sock_fd;
}

bool UdpDataProtocol::setSocketQos(socket_type& sock_fd)
{
#if defined(_WIN32)
    // Windows QoS (qWave) for audio traffic flows
    // https://learn.microsoft.com/en-us/windows/win32/api/_qos/
    // https://learn.microsoft.com/en-us/previous-versions/windows/desktop/qos/qwave-api-reference

#if 0
    // DISABLED SINCE THIS CAUSES GARBLED AUDIO FOR SOME PEOPLE

    // Initialize the QoS version parameter.
    QOS_VERSION Version;
    Version.MajorVersion = 1;
    Version.MinorVersion = 0;

    // Get a handle to the QoS subsystem.
    HANDLE QoSHandle = NULL;
    BOOL QoSResult   = QOSCreateHandle(&Version, &QoSHandle);
    if (QoSResult != TRUE) {
        std::cerr << "QOSCreateHandle failed. Error: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Add socket to flow.
    QOS_FLOWID QoSFlowId = 0;  // Flow Id must be 0.
    PSOCKADDR pSockAddr;
    if (mIPv6) {
        pSockAddr = reinterpret_cast<PSOCKADDR>(&mPeerAddr6);
    } else {
        pSockAddr = reinterpret_cast<PSOCKADDR>(&mPeerAddr);
    }
    // Note: QOSTrafficTypeVoice sets DSCP to 56 (high VO for WMM)
    // without having to call QOSSetFlow(). This is best for voice.
    QoSResult = QOSAddSocketToFlow(QoSHandle, sock_fd, pSockAddr, QOSTrafficTypeVoice,
                                   QOS_NON_ADAPTIVE_FLOW, &QoSFlowId);
    if (QoSResult != TRUE) {
        std::cerr << "QOSAddSocketToFlow failed. Error: ";
        std::cerr << WSAGetLastError() << std::endl;
        return false;
    }
#endif

#elif defined(__APPLE__)
    // set service type "Interactive Voice"
    // TODO: this is supposed to be the right thing to do on OSX, but doesn't seem to do
    // anything
    const int val = NET_SERVICE_TYPE_VO;
    int result =
        ::setsockopt(sock_fd, SOL_SOCKET, SO_NET_SERVICE_TYPE, &val, sizeof(val));
    if (result != 0) {
        std::cerr << "setsockopt failed. Error: " << errno << std::endl;
        return false;
    }
#else
    // Set ToS to DSCP 56 (high VO for WMM), recommended for Audio
    // See RFC2474 https://datatracker.ietf.org/doc/html/rfc2474
    // See also
    // https://www.slashroot.in/understanding-differentiated-services-tos-field-internet-protocol-header
    const char tos = 0xE0;  // 11100000 (56 << 2)
    int result;
    if (mIPv6) {
        result = ::setsockopt(sock_fd, IPPROTO_IPV6, IPV6_TCLASS, &tos, sizeof(tos));
    } else {
        result = ::setsockopt(sock_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
    }
    if (result != 0) {
        std::cerr << "setsockopt failed. Error: " << errno << std::endl;
        return false;
    }

    // Set 802.1q QoS priority
    int priority = 6;
    result = ::setsockopt(sock_fd, SOL_SOCKET, SO_PRIORITY, &priority, sizeof(priority));
    if (result != 0) {
        std::cerr << "setsockopt failed. Error: " << errno << std::endl;
        return false;
    }
#endif

    return true;
}

void UdpDataProtocol::processControlPacket(const char* buf)
{
    // Control signal (currently just check for exit packet);
    bool exit = true;
    for (int i = 0; i < mControlPacketSize; i++) {
        if (buf[i] != char(0xff)) {
            exit = false;
            i    = mControlPacketSize;
        }
    }
    if (exit && !mStopSignalSent) {
        mStopSignalSent = true;
        emit signalCeaseTransmission(QStringLiteral("Peer Stopped"));
        std::cout << "Peer Stopped" << std::endl;
    }
}

//*******************************************************************************
int UdpDataProtocol::receivePacket(char* buf, const size_t n)
{
    int n_bytes = ::recv(mSocket, buf, int(n), 0);
    if (n_bytes == mControlPacketSize) {
        processControlPacket(buf);
        return 0;
    }
    return n_bytes;
}

//*******************************************************************************
int UdpDataProtocol::sendPacket(const char* buf, const size_t n)
{
    /*#if defined (_WIN32)
    //Alternative windows specific code that uses winsock equivalents of the bsd socket
functions. DWORD n_bytes; WSABUF buffer; int error; buffer.len = n; buffer.buf = (char
*)buf;

    if (mIPv6) {
        error = WSASendTo(mSocket, &buffer, 1, &n_bytes, 0, (struct sockaddr *)
&mPeerAddr6, sizeof(mPeerAddr6), 0, 0); } else { error = WSASend(mSocket, &buffer, 1,
&n_bytes, 0, 0, 0);
    }
    if (error == SOCKET_ERROR) {
        cout << "Socket Error: " << WSAGetLastError() << endl;
    }
    return (int)n_bytes;
#else*/
    int n_bytes;
    if (mIPv6) {
        n_bytes = ::sendto(mSocket, buf, int(n), 0, (struct sockaddr*)&mPeerAddr6,
                           sizeof(mPeerAddr6));
    } else {
        n_bytes = ::sendto(mSocket, buf, int(n), 0, (struct sockaddr*)&mPeerAddr,
                           sizeof(mPeerAddr));
    }
    return n_bytes;
    // #endif
}

//*******************************************************************************
// void UdpDataProtocol::getPeerAddressFromFirstPacket(QHostAddress& peerHostAddress,
//                                                     uint16_t& port)
// {
//     while (!datagramAvailable()) {
//         msleep(100);
//     }
//     char buf[1];

//     struct sockaddr_storage addr;
//     std::memset(&addr, 0, sizeof(addr));
//     socklen_t sa_len = sizeof(addr);
//     ::recvfrom(mSocket, buf, 1, 0, (struct sockaddr*)&addr, &sa_len);
//     peerHostAddress.setAddress((struct sockaddr*)&addr);
//     if (mIPv6) {
//         port = ((struct sockaddr_in6*)&addr)->sin6_port;
//     } else {
//         port = ((struct sockaddr_in*)&addr)->sin_port;
//     }
// }

//*******************************************************************************
void UdpDataProtocol::run()
{
    if (gVerboseFlag)
        switch (mRunMode) {
        case RECEIVER: {
            std::cout << "step 3" << std::endl;
            break;
        }

        case SENDER: {
            std::cout << "step 4" << std::endl;
            break;
        }
        }

    // QObject::connect(this, SIGNAL(signalError(const char*)),
    //                 mJackTrip, SLOT(slotStopProcesses()),
    //                 Qt::QueuedConnection);

    if (mRunMode == RECEIVER) {
        cout << "UDP Socket Receiving in Port: " << mBindPort << endl;
        cout << gPrintSeparator << endl;
        // Make sure our socket is in non-blocking mode.
#ifdef _WIN32
        u_long nonblock = 1;
        ioctlsocket(mSocket, FIONBIO, &nonblock);
#else
        int flags = ::fcntl(mSocket, F_GETFL, 0);
        ::fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);
#endif
    }

    if (gVerboseFlag)
        std::cout << "    UdpDataProtocol:run" << mRunMode
                  << " before Setup Audio Packet buffer, Full Packet buffer, Redundancy "
                     "Variables"
                  << std::endl;

    // Setup Audio Packet buffer
    size_t audio_packet_size = getAudioPacketSizeInBites();
    // cout << "audio_packet_size: " << audio_packet_size << endl;
    mAudioPacket = new int8_t[audio_packet_size];
    std::memset(mAudioPacket, 0, audio_packet_size);  // set buffer to 0
    mBuffer.resize(audio_packet_size, 0);

    int full_packet_size;
    mSmplSize = mJackTrip->getAudioBitResolution() / 8;

    if (mRunMode == RECEIVER) {
        mChans = mJackTrip->getNumOutputChannels();
        if (0 == mChans)
            return;
        full_packet_size = mJackTrip->getReceivePacketSizeInBytes();
        mFullPacket      = new int8_t[full_packet_size];
        std::memset(mFullPacket, 0, full_packet_size);  // set buffer to 0
        // Put header in first packet
        mJackTrip->putHeaderInIncomingPacket(mFullPacket, mAudioPacket);

    } else {
        mChans = mJackTrip->getNumInputChannels();
        if (0 == mChans)
            return;
        full_packet_size = mJackTrip->getSendPacketSizeInBytes();
        mFullPacket      = new int8_t[full_packet_size];
        std::memset(mFullPacket, 0, full_packet_size);  // set buffer to 0
        // Put header in first packet
        mJackTrip->putHeaderInOutgoingPacket(mFullPacket, mAudioPacket);
    }

    // Redundancy Variables
    // (Algorithm explained at the end of this file)
    // ---------------------------------------------
    int full_redundant_packet_size = full_packet_size * mUdpRedundancyFactor;
    int8_t* full_redundant_packet  = NULL;

    if (full_redundant_packet_size > 0x10000) {
        throw std::runtime_error(
            "Maximum UDP packet size exceeded! Either reduce your "
            "Jack period size, the number of send channels or "
            "the packet redundancy.");
    }

    // Set realtime priority (function in jacktrip_globals.h)
    if (gVerboseFlag)
        std::cout << "    UdpDataProtocol:run" << mRunMode
                  << " before setRealtimeProcessPriority()" << std::endl;
    // std::endl;
    // Anton Runov: making setRealtimeProcessPriority optional
    if (mUseRtPriority) {
#if defined(__APPLE__)
        setRealtimeProcessPriority(mJackTrip->getBufferSizeInSamples(),
                                   mJackTrip->getSampleRate());
#else
        setRealtimeProcessPriority();
#endif
    }

    // clang-format off
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
    //
    // clang-format off

    threadHasStarted();

    switch (mRunMode) {
    case RECEIVER: {
        // Connect signals and slots for packets arriving too late notifications
        QObject::connect(this, &UdpDataProtocol::signalWaitingTooLong, this,
                         &UdpDataProtocol::printUdpWaitedTooLong, Qt::QueuedConnection);
        //-----------------------------------------------------------------------------------
        // Wait for the first packet to be ready and obtain address
        // from that packet
        if (gVerboseFlag)
            std::cout << "    UdpDataProtocol:run" << mRunMode
                      << " before !UdpSocket.hasPendingDatagrams()" << std::endl;
        std::cout << "Waiting for Peer..." << std::endl;
        // This blocks waiting for the first packet
        while (!datagramAvailable()) {
            if (mStopped) { return; }
            QThread::msleep(100);
            if (gVerboseFlag) std::cout << "100ms  " << std::flush;
        }
        full_redundant_packet_size = 0x10000;  // max UDP datagram size
        full_redundant_packet      = new int8_t[full_redundant_packet_size];
        full_redundant_packet_size = receivePacket(
            reinterpret_cast<char*>(full_redundant_packet), full_redundant_packet_size);
        // Check that peer has the same audio settings
        if (gVerboseFlag)
            std::cout << std::endl
                      << "    UdpDataProtocol:run" << mRunMode
                      << " before mJackTrip->checkPeerSettings()" << std::endl;
        if (!mJackTrip->checkPeerSettings(full_redundant_packet)) {
            // If our peer settings aren't compatible, don't continue.
            // (The checkPeerSettings function needs to signal the JackTrip instance with the exact error message.)
            delete[] full_redundant_packet;
            full_redundant_packet = nullptr;
            return;
        }

        int peer_chans   = mJackTrip->getPeerNumOutgoingChannels(full_redundant_packet);
        full_packet_size = mJackTrip->getHeaderSizeInBytes()
                           + mJackTrip->getPeerBufferSize(full_redundant_packet)
                                 * peer_chans * mSmplSize;
        /*
        cout << "peer sizes: " << mJackTrip->getHeaderSizeInBytes()
             << " + " << mJackTrip->getPeerBufferSize(full_redundant_packet)
             << " * " << mJackTrip->getNumChannels() << " * " << (int)mJackTrip->getAudioBitResolution()/8 << endl;
        cout << "full_packet_size: " << full_packet_size << " / " << mJackTrip->getPacketSizeInBytes() << endl;
        cout << "full_redundant_packet_size: " << full_redundant_packet_size << endl;
        // */

        if (gVerboseFlag) std::cout << "step 7" << std::endl;
        if (gVerboseFlag)
            std::cout << "    UdpDataProtocol:run" << mRunMode
                      << " before mJackTrip->parseAudioPacket()" << std::endl;
        std::cout << "Received Connection from Peer!" << std::endl;
        emit signalReceivedConnectionFromPeer();

        // Redundancy Variables
        // --------------------
        // NOTE: These types need to be the same unsigned integer as the sequence
        // number in the header. That way, they wrap around in the "same place"
        uint16_t current_seq_num = 0;  // Store current sequence number
        uint16_t last_seq_num    = 0;  // Store last package sequence number
        uint16_t newer_seq_num   = 0;  // Store newer sequence number
        mTotCount                = 0;
        mLostCount               = 0;
        mOutOfOrderCount         = 0;
        mLastOutOfOrderCount     = 0;
        mInitialState            = true;
        mRevivedCount            = 0;
        mStatCount               = 0;

        //Set up our platform specific polling mechanism. (kqueue, epoll, overlapped I/O)
#if !defined (MANUAL_POLL)
#if defined (__linux__)
        int epollfd = epoll_create1(0);
        struct epoll_event change, event;
        change.events = EPOLLIN;
        change.data.fd = mSocket;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, mSocket, &change);
#elif defined (_WIN32)
        WSAEVENT eventArray[WSA_MAXIMUM_WAIT_EVENTS];
        WSAOVERLAPPED socketOverlapped;
        WSABUF dataBuf;
        dataBuf.len = full_redundant_packet_size;
        dataBuf.buf = reinterpret_cast<char *>(full_redundant_packet);
        DWORD recvBytes = 0, flags = 0, index, bytesTransferred = 0;

        eventArray[0] = WSACreateEvent();
        if (eventArray == WSA_INVALID_EVENT) {
            emit signalError("Unable to set up network event monitoring");
            cout << "ERROR: Unable to set up network event monitoring" << endl;
            mStopped = true;
        }
        ZeroMemory(&socketOverlapped, sizeof(WSAOVERLAPPED));
        socketOverlapped.hEvent = eventArray[0];

        if (WSARecv(mSocket, &dataBuf, 1, &recvBytes, &flags, &socketOverlapped, NULL) == SOCKET_ERROR) {
            int result = WSAGetLastError();
            if (result != WSA_IO_PENDING) {
                emit signalError("Unable to listen for incoming network packets");
                cout << "ERROR: Unable to listen for incoming network packets" << endl;
                mStopped = true;
            }
        }
#else
        int kq = kqueue();
        struct kevent change;
        struct kevent event;
        EV_SET(&change, mSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
        struct timespec timeout;
        timeout.tv_sec = 0;
        timeout.tv_nsec = 10000000;
#endif
        int waitTime = 0;
#endif // MANUAL_POLL

        if (gVerboseFlag) std::cout << "step 8" << std::endl;
        while (!mStopped) {
            // Timer to report packets arriving too late
            // This QT method gave me a lot of trouble, so I replaced it with my own 'waitForReady'
            // that uses signals and slots and can also report with packets have not
            // arrive for a longer time
            //timeout = UdpSocket.waitForReadyRead(gUdpWaitTimeout);
            //        timeout = cc unused!
#if defined (MANUAL_POLL)
            waitForReady(60000); //60 seconds
            if (receivePacket(reinterpret_cast<char *>(full_redundant_packet), full_redundant_packet_size) > 0) {
                receivePacketRedundancy(full_redundant_packet, full_redundant_packet_size,
                                        full_packet_size, current_seq_num, last_seq_num,
                                        newer_seq_num);
            }
        }
#else

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

#if defined(_WIN32)
            index = WSAWaitForMultipleEvents(1, eventArray, FALSE, 10, FALSE);
            if (index == WSA_WAIT_TIMEOUT) {
                waitTime += 10;
                emit signalWaitingTooLong(waitTime);
            } else {
                waitTime = 0;
                WSAResetEvent(eventArray[index - WSA_WAIT_EVENT_0]);
                WSAGetOverlappedResult(mSocket, &socketOverlapped, &bytesTransferred, FALSE, &flags);
                if (bytesTransferred == mControlPacketSize) {
                    processControlPacket(reinterpret_cast<char *>(full_redundant_packet));
                } else if (bytesTransferred > 0 ){
                    receivePacketRedundancy(full_redundant_packet, full_redundant_packet_size,
                                        full_packet_size, current_seq_num, last_seq_num,
                                        newer_seq_num);
                }
                WSARecv(mSocket, &dataBuf, 1, &recvBytes, &flags, &socketOverlapped, NULL);
            }
#else
#if defined(__linux__)
            int n = epoll_wait(epollfd, &event, 1, 10);
#else
            int n = kevent(kq, &change, 1, &event, 1, &timeout);
#endif
            if (n > 0) {
                waitTime = 0;
                if (receivePacket(reinterpret_cast<char *>(full_redundant_packet), full_redundant_packet_size) > 0) {
                    receivePacketRedundancy(full_redundant_packet, full_redundant_packet_size,
                                            full_packet_size, current_seq_num, last_seq_num,
                                            newer_seq_num);
                }
            } else {
                waitTime += 10;
                emit signalWaitingTooLong(waitTime);
            }
#endif
        }
#if defined(__linux__)
        close(epollfd);
#elif defined(_WIN32)
        WSACloseEvent(eventArray);
#else
        close(kq);
#endif
#endif // MANUAL_POLL
        break; }

    case SENDER : {
        delete[] full_redundant_packet;
        full_redundant_packet = new int8_t[full_redundant_packet_size];
        std::memset(full_redundant_packet, 0,
                    full_redundant_packet_size); // Initialize to 0
        while (!mStopped && !JackTrip::sSigInt && !JackTrip::sAudioStopped) {
            sendPacketRedundancy(full_redundant_packet, full_redundant_packet_size,
                                 full_packet_size);
        }

        // Send exit packet (with 1 redundant packet).
        cout << "sending exit packet" << endl;
        QByteArray exitPacket = QByteArray(mControlPacketSize, static_cast<char>(0xff));
        sendPacket(exitPacket.constData(), mControlPacketSize);
        sendPacket(exitPacket.constData(), mControlPacketSize);
        emit signalCeaseTransmission();
        break; }
    }

    if (NULL != full_redundant_packet) {
        delete[] full_redundant_packet;
        full_redundant_packet = NULL;
    }
}

//*******************************************************************************
//bool
void UdpDataProtocol::waitForReady(int timeout_msec)
{
    int loop_resolution_usec = 100;    // usecs to wait on each loop
    int emit_resolution_usec = 10000;  // 10 milliseconds
    int timeout_usec         = timeout_msec * 1000;
    int elapsed_time_usec    = 0;  // Elapsed time in milliseconds

    while (!datagramAvailable() && (elapsed_time_usec <= timeout_usec) && !mStopped) {
        //    if (mStopped) { return false; }
        QThread::usleep(loop_resolution_usec);
        elapsed_time_usec += loop_resolution_usec;

        if (!(elapsed_time_usec % emit_resolution_usec)) {
            emit signalWaitingTooLong(static_cast<int>(elapsed_time_usec / 1000));
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
    if (!(wait_msec % gUdpWaitTimeout)) {
        // only log error once per gap in audio, rather than every 30ms
        if (wait_msec <= gUdpWaitTimeout) {
            std::cerr << "UDP waiting too long (more than " << gUdpWaitTimeout << "ms) for "
                    << mPeerAddress.toString().toStdString() << "..." << endl;
        }
        emit signalUdpWaitingTooLong();
    }
}

//*******************************************************************************
void UdpDataProtocol::receivePacketRedundancy(
    int8_t* full_redundant_packet, [[maybe_unused]] int full_redundant_packet_size, int full_packet_size,
    uint16_t& current_seq_num, uint16_t& last_seq_num, uint16_t& newer_seq_num)
{
    if (0.0 < mSimulatedLossRate || 0.0 < mSimulatedJitterRate) {
        double x = mUniformDist(mRndEngine);
        // Drop packets
        x -= mSimulatedLossRate;
        if (0 > x) { return; }
        // Delay packets
        x -= mSimulatedJitterRate;
        if (0 > x) { usleep(mUniformDist(mRndEngine) * mSimulatedJitterMaxDelay * 1e6); }
    }

    // Get Packet Sequence Number
    newer_seq_num   = mJackTrip->getPeerSequenceNumber(full_redundant_packet);
    current_seq_num = newer_seq_num;

    int16_t lost = 0;
    if (!mInitialState) {
        lost = newer_seq_num - last_seq_num - 1;
        if (0 > lost || 1000 < lost) {
            // Out of order packet, should be ignored
            ++mOutOfOrderCount;
            if (5 < ++mLastOutOfOrderCount) {
                mInitialState = true;
                mStatCount    = 0;
                mTotCount     = 0;
            }
            return;
        } else if (0 != lost) {
            mLostCount += lost;
        }
        mTotCount += 1 + lost;
    }
    mLastOutOfOrderCount = 0;
    mInitialState        = false;

    //cout << current_seq_num << " ";
    int redun_last_index = 0;
    for (unsigned int i = 1; i < mUdpRedundancyFactor; i++) {
        // Check if the package we receive is the next one expected, i.e.,
        // current_seq_num == (last_seq_num+1)
        if (current_seq_num == (last_seq_num + 1)) { break; }

        // if it's not, check the next one until it is the corresponding packet
        // or there aren't more available packets
        redun_last_index = i;  // index of packet to use in the redundant packet
        current_seq_num  = mJackTrip->getPeerSequenceNumber(full_redundant_packet
                                                           + (i * full_packet_size));
        //cout << current_seq_num << " ";
    }
    mRevivedCount += redun_last_index;
    //cout << endl;

    int peer_chans    = mJackTrip->getPeerNumOutgoingChannels(full_redundant_packet);
    int N             = mJackTrip->getPeerBufferSize(full_redundant_packet);
    int host_buf_size = N * mChans * mSmplSize;
    int hdr_size      = mJackTrip->getHeaderSizeInBytes();
    int gap_size      = mInitialState ? 0 : (lost - redun_last_index) * host_buf_size;

    last_seq_num = newer_seq_num;  // Save last read packet

    if ((int)mBuffer.size() < host_buf_size) { mBuffer.resize(host_buf_size, 0); }
    // Send to audio all available audio packets, in order
    for (int i = redun_last_index; i >= 0; i--) {
        int8_t* src = full_redundant_packet + (i * full_packet_size) + hdr_size;
        if (1 != mChans) {
            // Convert packet's non-interleaved layout to interleaved one used internally
            int8_t* dst = mBuffer.data();
            int C       = qMin(mChans, peer_chans);
            for (int n = 0; n < N; ++n) {
                for (int c = 0; c < C; ++c) {
                    memcpy(dst + (n * mChans + c) * mSmplSize,
                           src + (n + c * N) * mSmplSize, mSmplSize);
                }
            }
            src = dst;
        }
        int ok = true; // send audio buf to
        ok = mJackTrip->writeAudioBuffer(src, host_buf_size, gap_size, last_seq_num);
        if (!ok) {
            emit signalError("Local and Peer buffer settings are incompatible");
            cout << "ERROR: Local and Peer buffer settings are incompatible" << endl;
            mStopped = true;
            break;
        }
        gap_size = 0;
    }
}

//*******************************************************************************
bool UdpDataProtocol::getStats(DataProtocol::PktStat* stat)
{
    if (0 == mStatCount) {
        mLostCount       = 0;
        mOutOfOrderCount = 0;
        mRevivedCount    = 0;
    }
    stat->tot        = mTotCount;
    stat->lost       = mLostCount;
    stat->outOfOrder = mOutOfOrderCount;
    stat->revived    = mRevivedCount;
    stat->statCount  = mStatCount++;
    return true;
}

//*******************************************************************************
void UdpDataProtocol::setIssueSimulation(double loss, double jitter, double max_delay)
{
    mSimulatedLossRate       = loss;
    mSimulatedJitterRate     = jitter;
    mSimulatedJitterMaxDelay = max_delay;

    std::random_device r;
    mRndEngine   = std::default_random_engine(r());
    mUniformDist = std::uniform_real_distribution<double>(0.0, 1.0);

    cout << "Simulating network issues: "
            "loss_rate="
         << loss << ", jitter_rate=" << jitter << ", jitter_max_delay=" << max_delay
         << endl;
}

//*******************************************************************************
void UdpDataProtocol::sendPacketRedundancy(int8_t* full_redundant_packet,
                                           int full_redundant_packet_size,
                                           int full_packet_size)
{
    mJackTrip->readAudioBuffer(mAudioPacket);
    int8_t* src = mAudioPacket;
    if (1 < mChans) {
        // Convert internal interleaved layout to non-interleaved
        int N       = int(getAudioPacketSizeInBites() / mChans / mSmplSize);
        int8_t* dst = mBuffer.data();
        for (int n = 0; n < N; ++n) {
            for (int c = 0; c < mChans; ++c) {
                memcpy(dst + (n + c * N) * mSmplSize, src + (n * mChans + c) * mSmplSize,
                       mSmplSize);
            }
        }
        src = dst;
    }
    mJackTrip->putHeaderInOutgoingPacket(mFullPacket, src);

    // Move older packets to end of array of redundant packets
    std::memmove(full_redundant_packet + full_packet_size, full_redundant_packet,
                 full_packet_size * (mUdpRedundancyFactor - 1));
    // Copy new packet to the beginning of array
    std::memcpy(full_redundant_packet, mFullPacket, full_packet_size);

    // 10% (or other number) packet lost simulation.
    // Uncomment the if to activate
    //---------------------------------------------------------------------------------
    //int random_integer = rand();
    //if ( random_integer > (RAND_MAX/10) )
    //{
    sendPacket(reinterpret_cast<char*>(full_redundant_packet),
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
  otherwise it continue reading the mUdpRedundancyFactor packets until it finds the one that
  should come next (this can better perfected by just jumping until the correct packet).
  If it has more than one packet that it hasn't yet received, it sends it to the soundcard
  one by one.
*/

bool UdpDataProtocol::datagramAvailable()
{
    //Currently using a simplified version of the way QUdpSocket checks for datagrams.
    char c;
#if defined(_WIN32)
    //Need to use the winsock version of the function for MSG_PEEK
    WSABUF buffer;
    buffer.buf  = &c;
    buffer.len  = sizeof(c);
    DWORD n     = 0;
    DWORD flags = MSG_PEEK;
    int ret     = WSARecv(mSocket, &buffer, 1, &n, &flags, NULL, NULL);
    if (ret == 0) {
        //True if no error,
        return true;
    } else {
        //or if our error is that our buffer is too small.
        int err = WSAGetLastError();
        return (err == WSAEMSGSIZE);
    }
#else
    ssize_t n;
    n = ::recv(mSocket, &c, sizeof(c), MSG_PEEK);
    //We have a datagram if our buffer is too small or if no error.
    return (n != -1 || errno == EMSGSIZE);
#endif
}
