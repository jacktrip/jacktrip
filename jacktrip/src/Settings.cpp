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
#include "jacktrip_globals.h"

#include <iostream>
#include <getopt.h> // for command line parsing
#include <cstdlib>

using std::cout; using std::endl;

int gVerboseFlag = 0;


//*******************************************************************************
Settings::Settings() :
  mJackTrip(NULL),
  mJackTripMode(JackTrip::CLIENT),
  mDataProtocol(JackTrip::UDP),
  mNumChans(2),
  mBufferQueueLength(gDefaultQueueLength),
  mAudioBitResolution(JackAudioInterface::BIT16),
  mPortNum(gInputPort_0),
  mUnderrrunZero(false),
  mLoopBack(false),
  mJamLink(false)
{}

//*******************************************************************************
Settings::~Settings()
{
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
    { "pingtoserver", required_argument, NULL, 'C' }, // Run in ping to server mode, set server IP address
    { "portoffset", required_argument, NULL, 'o' }, // Port Offset from 4464
    { "queue", required_argument, NULL, 'q' }, // Queue Length
    { "bitres", required_argument, NULL, 'b' }, // Audio Bit Resolution
    { "zerounderrun", no_argument, NULL, 'z' }, // Use Underrun to Zeros Mode
    { "loopback", no_argument, NULL, 'l' }, // Run in loopback mode
    { "jamlink", no_argument, NULL, 'j' }, // Run in JamLink mode
    { "help", no_argument, NULL, 'h' }, // Print Help
    { NULL, 0, NULL, 0 }
  };

  // Parse Command Line Arguments
  //----------------------------------------------------------------------------
  /// \todo Specify mandatory arguments
  int ch;
  while ( (ch = getopt_long(argc, argv, "n:sc:C:o:q:b:zljh", longopts, NULL)) != -1 )
    switch (ch) {
      
    case 'n': // Number of input and output channels
      //-------------------------------------------------------
      mNumChans = atoi(optarg);
      break;
    case 's': // Run in server mode
      //-------------------------------------------------------
      mJackTripMode = JackTrip::SERVER;
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
      mPortNum += atoi(optarg);
      break;
    case 'b':
      //-------------------------------------------------------
      if      ( atoi(optarg) == 8 ) {
	mAudioBitResolution = JackAudioInterface::BIT8; }
      else if ( atoi(optarg) == 16 ) {
	mAudioBitResolution = JackAudioInterface::BIT16; }
      else if ( atoi(optarg) == 24 ) {
	mAudioBitResolution = JackAudioInterface::BIT16; }
      else if ( atoi(optarg) == 32 ) {
	mAudioBitResolution = JackAudioInterface::BIT32; }
      else {
	std::cerr << "--bitres ERROR: Wrong bit resolutions: " 
		  << atoi(optarg) << " is not supported." << endl;
	printUsage();
	std::exit(1); }
      break;
    case 'q':
      //-------------------------------------------------------
      if ( atoi(optarg) <= 0 ) {
	std::cerr << "--queue ERROR: The queue has to be a positive integer" << endl;
	printUsage();
	std::exit(1); }
      else {
	mBufferQueueLength = atoi(optarg);
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
    case 'j': // jamlink
      //-------------------------------------------------------
      mJamLink = true;
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
  cout << "Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe." << endl;
  cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
  cout << "-----------------------------------------------------------------------------" << endl;
  cout << "" << endl;
  cout << "Usage: jacktrip [-s|-c host] [options]" << endl;
  cout << "" << endl;
  cout << "Options: " << endl;
  cout << " -s, --server                             Run in Server Mode" << endl;
  cout << " -c, --client      <peer_host_IP_or_name> Run in Client Mode" << endl;
  cout << " -n, --numchannels #                      Number of Input and Output Channels (default "
       << 2 << ")" << endl;
  cout << " -q, --queue       # (1 or more)          Queue Buffer Length, in Packet Size (default " 
       << gDefaultQueueLength << ")" << endl;
  cout << " -o, --portoffset  #                      Receiving port offset from base port " << gInputPort_0 << endl;
  cout << " -b, --bitres      # (8, 16, 24, 32)      Audio Bit Rate Resolutions (default 16)" << endl;
  cout << " -z, --zerounderrun                       Set buffer to zeros when underrun occurs (defaults to wavetable)" << endl;
  cout << " -l, --loopback                           Run in Loop-Back Mode" << endl;
  cout << " -j, --jamlink                            Run in JamLink Mode (Connect to a JamLink Box)" << endl;
  cout << " -h, --help                               Prints this help" << endl;
  cout << "" << endl;
}


//*******************************************************************************
void Settings::startJackTrip()
{
  //JackTrip jacktrip(mJackTripMode, mDataProtocol, mNumChans,
  //	    mBufferQueueLength, mAudioBitResolution);
  mJackTrip = new JackTrip(mJackTripMode, mDataProtocol, mNumChans,
			   mBufferQueueLength, mAudioBitResolution);

  // Set buffers to zero when underrun
  if ( mUnderrrunZero ) {
    cout << "Setting buffers to zero when underrun..." << endl;
    mJackTrip->setUnderRunMode(JackTrip::ZEROS);
  }

  // Set peer address in server mode
  if ( mJackTripMode == JackTrip::CLIENT || mJackTripMode == JackTrip::CLIENTTOPINGSERVER ) {
    mJackTrip->setPeerAddress(mPeerAddress.toLatin1().data()); }

  // Set Ports
  mJackTrip->setAllPorts(mPortNum);

  // Set in JamLink Mode
  if ( mJamLink ) {
    cout << "Running in JamLink Mode..." << endl;
    mJackTrip->setPacketHeaderType(DataProtocol::JAMLINK); }

  // Add Plugins
  if ( mLoopBack ) {
    cout << "Running in Loop-Back Mode..." << endl;
    std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(mNumChans));
    mJackTrip->appendProcessPlugin(loopback);
  }

  // Start JackTrip
  mJackTrip->start();

  /* 
    sleep(10);
    cout << "Stoping JackTrip..." << endl;
    mJackTrip->stop();
  */
}
