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
 * \file Settings.cpp
 * \author Juan-Pablo Caceres
 * \date July 2008
 */

#include "Settings.h"
#include "LoopBack.h"
#include "NetKS.h"
#include "UdpMasterListener.h"
#include "JackTripWorker.h"
#include "jacktrip_globals.h"

#include <iostream>
#include <getopt.h> // for command line parsing
#include <cstdlib>

#include "ThreadPoolTest.h"

using std::cout; using std::endl;

int gVerboseFlag = 0;


//*******************************************************************************
Settings::Settings() :
  mJackTrip(NULL),
  mJackTripMode(JackTrip::SERVER),
  mDataProtocol(JackTrip::UDP),
  mNumChans(2),
  mBufferQueueLength(gDefaultQueueLength),
  mAudioBitResolution(AudioInterface::BIT16),
  mBindPortNum(gDefaultPort), mPeerPortNum(gDefaultPort),
  mClientName(NULL),
  mUnderrrunZero(false),
  mLoopBack(false),
  mJamLink(false),
  mEmptyHeader(false),
  mJackTripServer(false),
  mRedundancy(1),
  mUseJack(true),
  mChanfeDefaultSR(false),
  mChanfeDefaultBS(false)
{}

//*******************************************************************************
Settings::~Settings()
{
  stopJackTrip();
  delete mJackTrip;
}

//*******************************************************************************
void Settings::parseInput(int argc, char** argv)
{
  // If no command arguments are given, print instructions
  if(argc == 1) {
    printUsage();
    std::exit(0);
  }

  // Usage example at:
  // http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
  // options descriptor
  //----------------------------------------------------------------------------
  static struct option longopts[] = {
    // These options set a flag, has to be sepcified as a long option --verbose
    { "verbose", no_argument, &gVerboseFlag, 1 },
    // These options don't set a flag.
    { "numchannels", required_argument, NULL, 'n' }, // Number of input and output channels
    { "server", no_argument, NULL, 's' }, // Run in server mode
    { "client", required_argument, NULL, 'c' }, // Run in client mode, set server IP address
    { "jacktripserver", no_argument, NULL, 'S' }, // Run in JamLink mode
    { "pingtoserver", required_argument, NULL, 'C' }, // Run in ping to server mode, set server IP address
    { "portoffset", required_argument, NULL, 'o' }, // Port Offset from 4464
    { "bindport", required_argument, NULL, 'B' }, // Port Offset from 4464
    { "peerport", required_argument, NULL, 'P' }, // Port Offset from 4464
    { "queue", required_argument, NULL, 'q' }, // Queue Length
    { "redundancy", required_argument, NULL, 'r' }, // Redundancy
    { "bitres", required_argument, NULL, 'b' }, // Audio Bit Resolution
    { "zerounderrun", no_argument, NULL, 'z' }, // Use Underrun to Zeros Mode
    { "loopback", no_argument, NULL, 'l' }, // Run in loopback mode
    { "jamlink", no_argument, NULL, 'j' }, // Run in JamLink mode
    { "emptyheader", no_argument, NULL, 'e' }, // Run in JamLink mode
    { "clientname", required_argument, NULL, 'J' }, // Run in JamLink mode
    { "rtaudio", no_argument, NULL, 'R' }, // Run in JamLink mode
    { "srate", required_argument, NULL, 'T' }, // Set Sample Rate
    { "bufsize", required_argument, NULL, 'F' }, // Set buffer Size
    { "version", no_argument, NULL, 'v' }, // Version Number
    { "help", no_argument, NULL, 'h' }, // Print Help
    { NULL, 0, NULL, 0 }
  };

  // Parse Command Line Arguments
  //----------------------------------------------------------------------------
  /// \todo Specify mandatory arguments
  int ch;
  while ( (ch = getopt_long(argc, argv,
                            "n:sc:SC:o:B:P:q:r:b:zljeJ:RT:F:vh", longopts, NULL)) != -1 )
    switch (ch) {
      
    case 'n': // Number of input and output channels
      //-------------------------------------------------------
      mNumChans = atoi(optarg);
      break;
    case 's': // Run in server mode
      //-------------------------------------------------------
      mJackTripMode = JackTrip::SERVER;
      break;
    case 'S': // Run in jacktripserver mode
      //-------------------------------------------------------
      mJackTripServer = true;
      break;
    case 'c': // Client mode
      //-------------------------------------------------------
      mJackTripMode = JackTrip::CLIENT;
      mPeerAddress = optarg;
      break;
    case 'C': // Ping to server
      //-------------------------------------------------------
      mJackTripMode = JackTrip::CLIENTTOPINGSERVER;
      mPeerAddress = optarg;
      break;
    case 'o': // Port Offset
      //-------------------------------------------------------
      mBindPortNum += atoi(optarg);
      mPeerPortNum += atoi(optarg);
      break;
    case 'B': // Bind Port
      //-------------------------------------------------------
      mBindPortNum = atoi(optarg);
      break;
    case 'P': // Peer Port
      //-------------------------------------------------------
      mPeerPortNum = atoi(optarg);
      break;
    case 'b':
      //-------------------------------------------------------
      if      ( atoi(optarg) == 8 ) {
  mAudioBitResolution = AudioInterface::BIT8; }
      else if ( atoi(optarg) == 16 ) {
  mAudioBitResolution = AudioInterface::BIT16; }
      else if ( atoi(optarg) == 24 ) {
  mAudioBitResolution = AudioInterface::BIT24; }
      else if ( atoi(optarg) == 32 ) {
  mAudioBitResolution = AudioInterface::BIT32; }
      else {
	std::cerr << "--bitres ERROR: Wrong bit resolutions: " 
		  << atoi(optarg) << " is not supported." << endl;
	printUsage();
	std::exit(1); }
      break;
    case 'q':
      //-------------------------------------------------------
      if ( atoi(optarg) <= 0 ) {
  std::cerr << "--queue ERROR: The queue has to be equal or greater that 2" << endl;
	printUsage();
	std::exit(1); }
      else {
	mBufferQueueLength = atoi(optarg);
      }
      break;
    case 'r':
      //-------------------------------------------------------
      if ( atoi(optarg) <= 0 ) {
  std::cerr << "--redundancy ERROR: The reduncancy has to be a positive integer" << endl;
	printUsage();
	std::exit(1); }
      else {
	mRedundancy = atoi(optarg);
      }
      break;
    case 'z': // underrun to zero
      //-------------------------------------------------------
      mUnderrrunZero = true;
      break;
    case 'l': // loopback
      //-------------------------------------------------------
      mLoopBack = true;
      break;
    case 'e': // jamlink
      //-------------------------------------------------------
      mEmptyHeader = true;
      break;
    case 'j': // jamlink
      //-------------------------------------------------------
      mJamLink = true;
      break;
    case 'J': // Set client Name
      //-------------------------------------------------------
      mClientName = optarg;
      break;
    case 'R': // RtAudio
      //-------------------------------------------------------
      mUseJack = false;
      break;
    case 'T': // Sampling Rate
      //-------------------------------------------------------
      mChanfeDefaultSR = true;
      mSampleRate = atoi(optarg);
      break;
    case 'F': // Buffer Size
      //-------------------------------------------------------
      mChanfeDefaultBS = true;
      mAudioBufferSize = atoi(optarg);
      break;
    case 'v':
      //-------------------------------------------------------
      cout << "JackTrip VERSION: " << gVersion << endl;
      cout << "Copyright (c) 2008-2009 Juan-Pablo Caceres, Chris Chafe." << endl;
      cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
      cout << "" << endl;
      std::exit(0);
      break;
    case 'h':
      //-------------------------------------------------------
      printUsage();
      std::exit(0);
      break;
    default:
      //-------------------------------------------------------
      printUsage();
      std::exit(0);
      break;
    }

  // Warn user if undefined options where entered
  //----------------------------------------------------------------------------
  if (optind < argc) {
    cout << gPrintSeparator << endl;
    cout << "WARINING: The following entered options have no effect" << endl;
    cout << "          They will be ignored!" << endl;
    cout << "          Type jacktrip to see options." << endl;
    for( ; optind < argc; optind++) {
      printf("argument: %s\n", argv[optind]);
    }
    cout << gPrintSeparator << endl;
  }
}


//*******************************************************************************
void Settings::printUsage()
{
  cout << "" << endl;
  cout << "JackTrip: A System for High-Quality Audio Network Performance" << endl;
  cout << "over the Internet" << endl;
  cout << "Copyright (c) 2008-2009 Juan-Pablo Caceres, Chris Chafe." << endl;
  cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
  cout << "VERSION: " << gVersion << endl;
  cout << "-----------------------------------------------------------------------------" << endl;
  cout << "" << endl;
  cout << "Usage: jacktrip [-s|-c host] [options]" << endl;
  cout << "" << endl;
  cout << "Options: " << endl;
  cout << "REQUIRED ARGUMENTS: " << endl;
  cout << "===================" << endl;
  cout << " -s, --server                             Run in Server Mode" << endl;
  cout << " -c, --client      <peer_host_IP_number>  Run in Client Mode" << endl;
  cout << endl;
  cout << "OPTIONAL ARGUMENTS: " << endl;
  cout << "===================" << endl;
  cout << " -n, --numchannels #                      Number of Input and Output Channels (default "
       << 2 << ")" << endl;
  cout << " -q, --queue       # (2 or more)          Queue Buffer Length, in Packet Size (default "
       << gDefaultQueueLength << ")" << endl;
  cout << " -r, --redundancy  # (1 or more)          Packet Redundancy to avoid glitches with packet losses (defaul 1)" 
       << endl;
  cout << " -o, --portoffset  #                      Receiving port offset from base port " << gDefaultPort << endl;
  cout << " --bindport        #                      Set only the bind port number (default to 4464)" << endl;
  cout << " --peerport        #                      Set only the Peer port number (default to 4464)" << endl;
  cout << " -b, --bitres      # (8, 16, 24, 32)      Audio Bit Rate Resolutions (default 16)" << endl;
  cout << " -z, --zerounderrun                       Set buffer to zeros when underrun occurs (defaults to wavetable)" << endl;
  cout << " -l, --loopback                           Run in Loop-Back Mode" << endl;
  cout << " -j, --jamlink                            Run in JamLink Mode (Connect to a JamLink Box)" << endl;
  cout << " --clientname                             Change default client name (default is JackTrip)" << endl;
  cout << endl;
  cout << "ARGUMENTS TO USE IT WITHOUT JACK:" << endl;
  cout << "=================================" << endl;
  cout << " --rtaudio                                Use defaul sound system instead of Jack" << endl;
  cout << "   --srate         #                      Set the sampling rate, works on --rtaudio mode only (defaults 48000)" << endl;
  cout << "   --bufsize       #                      Set the buffer size, works on --rtaudio mode only (defaults 128)" << endl;
  cout << endl;
  cout << "HELP ARGUMENTS: " << endl;
  cout << "===============" << endl;
  cout << " -v, --version                            Prints Version Number" << endl;
  cout << " -h, --help                               Prints this Help" << endl;
  cout << "" << endl;
}


//*******************************************************************************
void Settings::startJackTrip()
{

  /// \todo Change this, just here to test
  if ( mJackTripServer ) {
    UdpMasterListener* udpmaster = new UdpMasterListener;
    udpmaster->start();
    
    //---Thread Pool Test--------------------------------------------
    /*
    cout << "BEFORE START" << endl;
    ThreadPoolTest* thtest = new ThreadPoolTest();
    // QThreadPool takes ownership and deletes 'hello' automatically
    QThreadPool::globalInstance()->start(thtest);

    cout << "AFTER START" << endl;
    sleep(2);
    thtest->stop();
    QThreadPool::globalInstance()->waitForDone();
    */
    //---------------------------------------------------------------
  }
  
  else {
    
    //JackTrip jacktrip(mJackTripMode, mDataProtocol, mNumChans,
    //	    mBufferQueueLength, mAudioBitResolution);
    mJackTrip = new JackTrip(mJackTripMode, mDataProtocol, mNumChans,
                             mBufferQueueLength, mRedundancy, mAudioBitResolution);

    // Connect Signals and Slots
    QObject::connect(mJackTrip, SIGNAL( signalProcessesStopped() ),
                     this, SLOT( slotExitProgram() ));
    
    // Change client name if different from default
    if (mClientName != NULL) {
      mJackTrip->setClientName(mClientName);
    }

    // Set buffers to zero when underrun
    if ( mUnderrrunZero ) {
      cout << "Setting buffers to zero when underrun..." << endl;
      cout << gPrintSeparator << std::endl;
      mJackTrip->setUnderRunMode(JackTrip::ZEROS);
    }
    
    // Set peer address in server mode
    if ( mJackTripMode == JackTrip::CLIENT || mJackTripMode == JackTrip::CLIENTTOPINGSERVER ) {
      mJackTrip->setPeerAddress(mPeerAddress.toLatin1().data()); }
    
    // Set Ports
    //cout << "SETTING ALL PORTS" << endl;
    mJackTrip->setBindPorts(mBindPortNum);
    mJackTrip->setPeerPorts(mPeerPortNum);

    // Set in JamLink Mode
    if ( mJamLink ) {
      cout << "Running in JamLink Mode..." << endl;
      cout << gPrintSeparator << std::endl;
      mJackTrip->setPacketHeaderType(DataProtocol::JAMLINK);
    }

    // Set in EmptyHeader Mode
    if ( mEmptyHeader ) {
      cout << "Running in EmptyHeader Mode..." << endl;
      cout << gPrintSeparator << std::endl;
      mJackTrip->setPacketHeaderType(DataProtocol::EMPTY);
    }

    // Set RtAudio
    if (!mUseJack) {
      mJackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
    }

    // Chanfe default Sampling Rate
    if (mChanfeDefaultSR) {
      mJackTrip->setSampleRate(mSampleRate);
    }

    // Chanfe default Buffer Size
    if (mChanfeDefaultBS) {
      mJackTrip->setAudioBufferSizeInSamples(mAudioBufferSize);
    }

    // Add Plugins
    if ( mLoopBack ) {
      cout << "Running in Loop-Back Mode..." << endl;
      cout << gPrintSeparator << std::endl;
      //std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(mNumChans));
      //mJackTrip->appendProcessPlugin(loopback.get());

      LoopBack* loopback = new LoopBack(mNumChans);
      mJackTrip->appendProcessPlugin(loopback);

      // ----- Test Karplus Strong -----------------------------------
      //std::tr1::shared_ptr<NetKS> loopback(new NetKS());
      //mJackTrip->appendProcessPlugin(loopback);
      //loopback->play();
      //NetKS* netks = new NetKS;
      //mJackTrip->appendProcessPlugin(netks);
      //netks->play();
      // -------------------------------------------------------------
    }
    
    // Start JackTrip
    mJackTrip->startProcess();
    mJackTrip->start();
    
    /* 
       sleep(10);
       cout << "Stoping JackTrip..." << endl;
       mJackTrip->stop();
    */
  }
}


//*******************************************************************************
void Settings::stopJackTrip()
{
  mJackTrip->stop();
}
