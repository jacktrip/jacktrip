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
#include "globals.h"

#include <iostream>
#include <getopt.h> // for command line parsing

using std::cout; using std::endl;

int gVerboseFlag = 0;


//*******************************************************************************
Settings::Settings() :
  mNumInChans(gDefaultNumInChannels),
  mNumOutChans(gDefaultNumOutChannels),
  mRunMode(DataProtocol::SENDER),
  mLoopBack(false)
{}


//*******************************************************************************
void Settings::parseInput(int argc, char** argv)
{
  // If no command arguments are given, print instructions
  if(argc == 1) {
    printUsage();
    std::exit(0);
  }

  // Usage example at:
  //http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
  // options descriptor
  //----------------------------------------------------------------------------
  static struct option longopts[] = {
    // These options set a flag, has to be sepcified as a long option --verbose
    { "verbose", no_argument, &gVerboseFlag, 1 },
    // These options don't set a flag.
    { "numchannels", required_argument, NULL, 'n' }, // Number of input and output channels
    { "server", no_argument, NULL, 's' }, // run in server mode
    { "client", required_argument, NULL, 'c' }, //run in client mode, set server IP address
    { "loopback", required_argument, NULL, 'l' }, //run in loopback mode
    { "help", no_argument, NULL, 'h' }, // Print Help
    { NULL, 0, NULL, 0 }
  };

  // Parse Command Line Arguments
  //----------------------------------------------------------------------------
  /// \todo Specify mandatory arguments
  int ch;
  while ( (ch = getopt_long(argc, argv, "n:sc:lh", longopts, NULL)) != -1 )
    switch (ch) {
    case 'n':
      mNumInChans = atoi(optarg);
      mNumOutChans = atoi(optarg);
      break;
    case 's':
      mRunMode = DataProtocol::RECEIVER; /// \todo change this to SERVER
      break;
    case 'c':
      mRunMode = DataProtocol::SENDER; /// \todo change this to CLIENT
      mPeerHostOrIP = optarg;
      break;
    case 'l': //loopback
      mLoopBack = true;
      break;
    case 'h':
      printUsage();
      std::exit(0);
      break;
    default:
      printUsage();
      std::exit(0);
      break;
    }

  // Warn user if undefined options where entered
  //----------------------------------------------------------------------------
  if (optind < argc) {
    cout << SEPARATOR << endl;
    cout << "WARINING: The following entered options have no effect" << endl;
    cout << "          They will be ignored!" << endl;
    cout << "          Type paultrip to see options." << endl;
    for( ; optind < argc; optind++) {
      printf("argument: %s\n", argv[optind]);
    }
    cout << SEPARATOR << endl;
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
  cout << "Usage: paultrip [-s|-c host] [options]" << endl;
  cout << "" << endl;
  cout << "Options: " << endl;
  cout << " -n, --numchannels #                      Number of Input and Output Channels" << endl;
  cout << " -s, --server                             Run in Server Mode" << endl;
  cout << " -c, --client      <peer_host_IP_or_name> Run in Client Mode" << endl;
  cout << " -l, --loopback                           Run in Loop-Back Mode" << endl;
  cout << " -h, --help                               Prints this help" << endl;
  cout << "" << endl;
}
