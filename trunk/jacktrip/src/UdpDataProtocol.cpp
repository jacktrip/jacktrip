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
#include <winsock.h>
#endif
#if defined (__LINUX__) || (__MAC__OSX__)
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
  if (mRunMode == RECEIVER) {
    QObject::connect(this, SIGNAL(signalWatingTooLong(int)),
                     jacktrip, SLOT(slotUdpWatingTooLong(int)), Qt::QueuedConnection);
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
void UdpDataProtocol::setPeerAddress(const char* peerHostOrIP) throw(std::invalid_argument)
{
  // Get DNS Address
  QHostInfo info = QHostInfo::fromName(peerHostOrIP);
  if (!info.addresses().isEmpty()) {
    // use the first IP address
    mPeerAddress = info.addresses().first();
    //cout << "UdpDataProtocol::setPeerAddress IP Address Number: "
    //    << mPeerAddress.toString().toStdString() << endl;
  }

  // check if the ip address is valid
  if ( mPeerAddress.isNull() ) {
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
}


//*******************************************************************************
void UdpDataProtocol::bindSocket(QUdpSocket& UdpSocket) throw(std::runtime_error)
{
  QMutexLocker locker(&sUdpMutex);

#if defined __WIN_32__
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  wVersionRequested = MAKEWORD( 1, 1 );

  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    // Tell the user that we couldn't find a useable
    // winsock.dll.

    return;
  }

  // Confirm that the Windows Sockets DLL supports 1.1. or higher

  if ( LOBYTE( wsaData.wVersion ) != 1 ||
       HIBYTE( wsaData.wVersion ) != 1 ) {
    // Tell the user that we couldn't find a useable
    // winsock.dll.
    WSACleanup( );
    return;
  }

  // Creat socket descriptor
  SOCKET sock_fd;
  SOCKADDR_IN local_addr;
#endif

#if defined ( __LINUX__ ) || (__MAC_OSX__)
  int sock_fd;
  //Set local IPv4 Address
  struct sockaddr_in local_addr;
#endif

  // Creat socket descriptor
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

  //::bzero(&local_addr, sizeof(local_addr));
  std::memset(&local_addr, 0, sizeof(local_addr)); // set buffer to 0
  local_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
  local_addr.sin_port = htons(mBindPort); //set local port

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
#if defined ( __LINUX__ ) || ( __MAC_OSX__ )
  if ( (::bind(sock_fd, (struct sockaddr *) &local_addr, sizeof(local_addr))) < 0 )
  { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }
#endif
#if defined (__WIN_32__)
  //int bound;
  //bound = bind(sock_fd, (SOCKADDR *) &local_addr, sizeof(local_addr));
  if ( (bind(sock_fd, (SOCKADDR *) &local_addr, sizeof(local_addr))) == SOCKET_ERROR )
  { throw std::runtime_error("ERROR: UDP Socket Bind Error"); }
#endif

  // To be able to use the two UDP sockets bound to the same port number,
  // we connect the receiver and issue a SHUT_WR.
  if (mRunMode == SENDER) {
    // We use the sender as an unconnected UDP socket
    UdpSocket.setSocketDescriptor(sock_fd, QUdpSocket::BoundState,
                                  QUdpSocket::WriteOnly);
  }
  else if (mRunMode == RECEIVER) {
#if defined (__LINUX__) || (__MAC_OSX__)
    // Set peer IPv4 Address
    struct sockaddr_in peer_addr;
    bzero(&peer_addr, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
    peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
    peer_addr.sin_port = htons(mPeerPort); //set local port
    // Connect the socket and issue a Write shutdown (to make it a
    // reader socket only)
    if ( (::inet_pton(AF_INET, mPeerAddress.toString().toLatin1().constData(),
                      &peer_addr.sin_addr)) < 1 )
    { throw std::runtime_error("ERROR: Invalid address presentation format"); }
    if ( (::connect(sock_fd, (struct sockaddr *) &peer_addr, sizeof(peer_addr))) < 0)
    { throw std::runtime_error("ERROR: Could not connect UDP socket"); }
    if ( (::shutdown(sock_fd,SHUT_WR)) < 0)
    { throw std::runtime_error("ERROR: Could suntdown SHUT_WR UDP socket"); }
#endif
#if defined __WIN_32__
    // Set peer IPv4 Address
    SOCKADDR_IN peer_addr;
    std::memset(&peer_addr, 0, sizeof(peer_addr)); // set buffer to 0
    peer_addr.sin_family = AF_INET; //AF_INET: IPv4 Protocol
    peer_addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY: let the kernel decide the active address
    peer_addr.sin_port = htons(mPeerPort); //set local port
    // Connect the socket and issue a Write shutdown (to make it a
    // reader socket only)
    peer_addr.sin_addr.s_addr = inet_addr(mPeerAddress.toString().toLatin1().constData());
    int con = (::connect(sock_fd, (struct sockaddr *) &peer_addr, sizeof(peer_addr)));
    if ( con < 0)
    {
      fprintf(stderr, "ERROR: Could not connect UDP socket");
      throw std::runtime_error("ERROR: Could not connect UDP socket");
    }
    //cout<<"connect returned: "<<con<<endl;
    int shut_sr = shutdown(sock_fd, SD_SEND);  //shut down sender's receive function
    if ( shut_sr< 0)
    {
      fprintf(stderr, "ERROR: Could not shutdown SD_SEND UDP socket");
      throw std::runtime_error("ERROR: Could not shutdown SD_SEND UDP socket");
    }
#endif

    UdpSocket.setSocketDescriptor(sock_fd, QUdpSocket::ConnectedState,
                                  QUdpSocket::ReadOnly);
    cout << "UDP Socket Receiving in Port: " << mBindPort << endl;
    cout << gPrintSeparator << endl;
  }

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
int UdpDataProtocol::sendPacket(QUdpSocket& UdpSocket, const QHostAddress& PeerAddress,
                                const char* buf, const size_t n)
{
  int n_bytes = UdpSocket.writeDatagram(buf, n, PeerAddress, mPeerPort);
  return n_bytes;
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
  //QObject::connect(this, SIGNAL(signalError(const char*)),
  //                 mJackTrip, SLOT(slotStopProcesses()),
  //                 Qt::QueuedConnection);

  // Creat and bind sockets
  QUdpSocket UdpSocket;
  try {
    bindSocket(UdpSocket); // Bind Socket
  } catch ( const std::exception & e ) {
    emit signalError( e.what() );
    return;
  }


  QHostAddress PeerAddress;
  PeerAddress = mPeerAddress;

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

  bool timeout = false; // Time out flag for packets that arrive too late
  
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
  set_crossplatform_realtime_priority();

  switch ( mRunMode )
  {
  case RECEIVER : {
      // Connect signals and slots for packets arriving too late notifications
      QObject::connect(this, SIGNAL(signalWatingTooLong(int)),
                       this, SLOT(printUdpWaitedTooLong(int)),
                       Qt::QueuedConnection);
      //----------------------------------------------------------------------------------- 
      // Wait for the first packet to be ready and obtain address
      // from that packet
      std::cout << "Waiting for Peer..." << std::endl;
      // This blocks waiting for the first packet
      while ( !UdpSocket.hasPendingDatagrams() ) {
        if (mStopped) { return; }
        QThread::msleep(100);
      }
      int first_packet_size = UdpSocket.pendingDatagramSize();
      // The following line is the same as
      int8_t* first_packet = new int8_t[first_packet_size];
      /// \todo fix this to avoid memory leaks
      // but avoids memory leaks
      //std::tr1::shared_ptr<int8_t> first_packet(new int8_t[first_packet_size]);
      receivePacket( UdpSocket, reinterpret_cast<char*>(first_packet), first_packet_size);
      // Check that peer has the same audio settings
      mJackTrip->checkPeerSettings(first_packet);
      mJackTrip->parseAudioPacket(mFullPacket, mAudioPacket);
      std::cout << "Received Connection for Peer!" << std::endl;
      emit signalReceivedConnectionFromPeer();

      // Redundancy Variables
      // --------------------
      // NOTE: These types need to be the same unsigned integer as the sequence
      // number in the header. That way, they wrap around in the "same place"
      uint16_t current_seq_num = 0; // Store current sequence number
      uint16_t last_seq_num = 0;    // Store last package sequence number
      uint16_t newer_seq_num = 0;   // Store newer sequence number

      while ( !mStopped )
      {
        // Timer to report packets arriving too late
        // This QT method gave me a lot of trouble, so I replaced it with my own 'waitForReady'
        // that uses signals and slots and can also report with packets have not
        // arrive for a longer time
        //timeout = UdpSocket.waitForReadyRead(30);
        timeout = waitForReady(UdpSocket, 60000); //60 seconds

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
      //----------------------------------------------------------------------------------- 
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
        sendPacketRedundancy(UdpSocket,
                             PeerAddress,
                             full_redundant_packet,
                             full_redundant_packet_size,
                             full_packet_size);
      }
      break; }
  }
}


//*******************************************************************************
bool UdpDataProtocol::waitForReady(QUdpSocket& UdpSocket, int timeout_msec)
{
  int loop_resolution_usec = 100; // usecs to wait on each loop
  int emit_resolution_usec = 10000; // 10 milliseconds
  int timeout_usec = timeout_msec * 1000;
  int ellaped_time_usec = 0; // Ellapsed time in milliseconds

  while ( ( !(UdpSocket.hasPendingDatagrams()) && (ellaped_time_usec <= timeout_usec) )
    && !mStopped ){
    if (mStopped) { return false; }
    QThread::usleep(loop_resolution_usec);
    ellaped_time_usec += loop_resolution_usec;
    
    if ( !(ellaped_time_usec % emit_resolution_usec) ) {
      emit signalWatingTooLong(static_cast<int>(ellaped_time_usec/1000));
    }
  }
  
  if ( ellaped_time_usec >= timeout_usec )
  {
    emit signalWatingTooLong(ellaped_time_usec/1000);
    return false;
  }
  return true;
}


//*******************************************************************************
void UdpDataProtocol::printUdpWaitedTooLong(int wait_msec)
{
  int wait_time = 30; // msec
  if ( !(wait_msec%wait_time) ) {
    std::cerr << "UDP waiting too long (more than " << wait_time << "ms)..." << endl;
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
  current_seq_num = newer_seq_num;


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
void UdpDataProtocol::sendPacketRedundancy(QUdpSocket& UdpSocket,
                                           QHostAddress& PeerAddress,
                                           int8_t* full_redundant_packet,
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
  sendPacket( UdpSocket, PeerAddress, reinterpret_cast<char*>(full_redundant_packet),
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
