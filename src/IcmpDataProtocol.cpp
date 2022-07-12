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
 * \file IcmpDataProtocol.cpp
 * \author Dominick Hing
 * \date July 2022
 */

//#define MANUAL_POLL

#include "IcmpDataProtocol.h"

#include <QHostInfo>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "JackTrip.h"
#include "jacktrip_globals.h"
#ifdef _WIN32
//#include <winsock.h>
#include <winsock2.h>  //cc need SD_SEND
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
QMutex IcmpDataProtocol::sIcmpMutex;

//*******************************************************************************
IcmpDataProtocol::IcmpDataProtocol(JackTrip* jacktrip, const runModeT runmode)
    : DataProtocol(jacktrip, runmode, 0, 0)
    , mRunMode(runmode)
{
    mStopped = false;
    mIPv6    = false;
    std::memset(&mPeerAddr, 0, sizeof(mPeerAddr));
    std::memset(&mPeerAddr6, 0, sizeof(mPeerAddr6));
    // mPeerAddr.sin_port   = htons(mPeerPort);
    // mPeerAddr6.sin6_port = htons(mPeerPort);
}

//*******************************************************************************
IcmpDataProtocol::~IcmpDataProtocol()
{
    if (mRunMode == RECEIVER) {
#ifdef _WIN32
        closesocket(mSocket);
#else
        ::close(mSocket);
#endif
    }
    wait();
}


//*******************************************************************************
void IcmpDataProtocol::setPeerAddress(const char* peerHostOrIP)
{
    // Get DNS Address
#ifndef _WIN32
    // Don't make the following code conditional on windows
    //(Addresses a weird timing bug when in hub client mode)
    if (!mPeerAddress.setAddress(peerHostOrIP)) {
#endif
        QHostInfo info = QHostInfo::fromName(peerHostOrIP);
        if (!info.addresses().isEmpty()) {
            // use the first IP address
            mPeerAddress = info.addresses().constFirst();
        }
        // cout << "IcmpDataProtocol::setPeerAddress IP Address Number: "
        //    << mPeerAddress.toString().toStdString() << endl;
#ifndef _WIN32
    }
#endif

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

#if defined(_WIN32)
void IcmpDataProtocol::setSocket(SOCKET& socket)
#else
void IcmpDataProtocol::setSocket(int& socket)
#endif
{
    // If we haven't been passed a valid socket, then we should bind one.
#if defined(_WIN32)
    if (socket == INVALID_SOCKET) {
#else
    if (socket == -1) {
#endif
        try {
            if (gVerboseFlag)
                std::cout << "    IcmpDataProtocol:run" << mRunMode << " before bindSocket"
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
#if defined(_WIN32)
SOCKET IcmpDataProtocol::bindSocket()
#else
int IcmpDataProtocol::bindSocket()
#endif
{
    QMutexLocker locker(&sIcmpMutex);

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
        sock_fd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMP);
        std::memset(&local_addr6, 0, sizeof(local_addr6));
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr   = in6addr_any;
        local_addr6.sin6_port   = htons(0);
    } else {
        sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

        //::bzero(&local_addr, sizeof(local_addr));
        std::memset(&local_addr, 0, sizeof(local_addr));  // set buffer to 0
        local_addr.sin_family = AF_INET;                  // AF_INET: IPv4 Protocol
        local_addr.sin_addr.s_addr =
            htonl(INADDR_ANY);  // INADDR_ANY: let the kernel decide the active address
        local_addr.sin_port = htons(0);  // set local port
    }

    // Bind the Socket
    if (mIPv6) {
        if ((::bind(sock_fd, (struct sockaddr*)&local_addr6, sizeof(local_addr6))) < 0) {
            throw std::runtime_error("ERROR: ICMP Socket Bind Error");
        }
    } else {
        if ((::bind(sock_fd, (struct sockaddr*)&local_addr, sizeof(local_addr))) < 0) {
            throw std::runtime_error("ERROR: ICMP Socket Bind Error");
        }
    }

    // Return our file descriptor so the socket can be shared for a
    // full duplex connection.
    return sock_fd;
}


//*******************************************************************************
int IcmpDataProtocol::receivePacket(char* buf, const size_t n)
{
    QMutexLocker locker(&mIcmpStatsMutex);

    // TODO: Update with actual ICMP Data
    mRttVector.push_back(1.5);
    mReceivedCount++;

    int n_bytes = ::recv(mSocket, buf, n, 0);
    return n_bytes;
}

//*******************************************************************************
int IcmpDataProtocol::sendPacket(const char* buf, const size_t n)
{

    QMutexLocker locker(&mIcmpStatsMutex);
    mSentCount++;

    int n_bytes;
    if (mIPv6) {
        n_bytes = ::sendto(mSocket, buf, n, 0, (struct sockaddr*)&mPeerAddr6,
                           sizeof(mPeerAddr6));
    } else {
        n_bytes =
            ::sendto(mSocket, buf, n, 0, (struct sockaddr*)&mPeerAddr, sizeof(mPeerAddr));
    }
    return n_bytes;
}

void IcmpDataProtocol::run()
{

    if (mRunMode == RECEIVER) {
        // Make sure our socket is in non-blocking mode.
#ifdef _WIN32
        u_long nonblock = 1;
        ioctlsocket(mSocket, FIONBIO, &nonblock);
#else
        int flags = ::fcntl(mSocket, F_GETFL, 0);
        ::fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);
#endif
    }
}

//*******************************************************************************
bool IcmpDataProtocol::getIcmpStats(IcmpDataProtocol::IcmpStat* stat)
{
    // stat->tot        = mTotCount;
    // stat->lost       = mLostCount;
    // stat->outOfOrder = mOutOfOrderCount;
    // stat->revived    = mRevivedCount;
    // stat->statCount  = mStatCount++;
    return true;
}
