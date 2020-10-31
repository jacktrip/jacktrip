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
//#include "NetKS.h"
#include "Effects.h"

#ifdef WAIR // wair
#include "ap8x2.dsp.h"
#include "Stk16.dsp.h"
#endif // endwhere

//#include "JackTripWorker.h"
#include "jacktrip_globals.h"

#include <iostream>
#include <getopt.h> // for command line parsing
#include <cstdlib>
#include <assert.h>
#include <ctype.h>

//#include "ThreadPoolTest.h"

using std::cout; using std::endl;

int gVerboseFlag = 0;

enum JTLongOptIDS {
  OPT_BUFSTRATEGY = 1001,
  OPT_SIMLOSS,
  OPT_SIMJITTER,
  OPT_BROADCAST,
  OPT_RTUDPPRIORITY,
};

//*******************************************************************************
Settings::Settings() :
    mJackTripMode(JackTrip::SERVER),
    mDataProtocol(JackTrip::UDP),
    mNumChans(2),
    mBufferQueueLength(gDefaultQueueLength),
    mAudioBitResolution(AudioInterface::BIT16),
    mBindPortNum(gDefaultPort), mPeerPortNum(gDefaultPort),
    mServerUdpPortNum(0),
    mUnderrunMode(JackTrip::WAVETABLE),
    mStopOnTimeout(false),
    mBufferStrategy(1),
    mLoopBack(false),
    #ifdef WAIR // WAIR
    mNumNetRevChans(0),
    mWAIR(false),
    #endif // endwhere
    mJamLink(false),
    mEmptyHeader(false),
    mJackTripServer(false),
    mLocalAddress(gDefaultLocalAddress),
    mRedundancy(1),
    mUseJack(true),
    mChanfeDefaultSR(false),
    mChanfeDefaultID(0),
    mChanfeDefaultBS(false),
    mHubConnectionMode(JackTrip::SERVERTOCLIENT),
    mConnectDefaultAudioPorts(true),
    mIOStatTimeout(0),
    mSimulatedLossRate(0.0),
    mSimulatedJitterRate(0.0),
    mSimulatedDelayRel(0.0),
    mBroadcastQueue(0),
    mUseRtUdpPriority(false)
{}

//*******************************************************************************
Settings::~Settings() = default;

//*******************************************************************************
void Settings::parseInput(int argc, char** argv)
{
    // Always use decimal point for floating point numbers
    setlocale( LC_NUMERIC, "C" );
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
        // These options don't set a flag.
        { "numchannels", required_argument, NULL, 'n' }, // Number of input and output channels
#ifdef WAIR // WAIR
        { "wair", no_argument, NULL, 'w' }, // Run in LAIR mode, sets numnetrevchannels
        { "addcombfilterlength", required_argument, NULL, 'N' }, // added comb filter length
        { "combfilterfeedback", required_argument, NULL, 'H' }, // comb filter feedback
#endif // endwhere
        { "server", no_argument, NULL, 's' }, // Run in P2P server mode
        { "client", required_argument, NULL, 'c' }, // Run in P2P client mode, set server IP address
        { "localaddress", required_argument, NULL, 'L' }, // set local address e.g., 127.0.0.2 for second instance on same host
        { "jacktripserver", no_argument, NULL, 'S' }, // Run in JamLink mode
        { "pingtoserver", required_argument, NULL, 'C' }, // Run in ping to server mode, set server IP address
        { "portoffset", required_argument, NULL, 'o' }, // Port Offset from 4464
        { "bindport", required_argument, NULL, 'B' }, // Port Offset from 4464
        { "peerport", required_argument, NULL, 'P' }, // Port Offset from 4464
        { "udpbaseport", required_argument, NULL, 'U' }, // Server udp base port (defaults to 61002)
        { "queue", required_argument, NULL, 'q' }, // Queue Length
        { "redundancy", required_argument, NULL, 'r' }, // Redundancy
        { "bitres", required_argument, NULL, 'b' }, // Audio Bit Resolution
        { "zerounderrun", no_argument, NULL, 'z' }, // Use Underrun to Zeros Mode
        { "timeout", no_argument, NULL, 't' }, // Quit after 10 second network timeout
        { "loopback", no_argument, NULL, 'l' }, // Run in loopback mode
        { "jamlink", no_argument, NULL, 'j' }, // Run in JamLink mode
        { "emptyheader", no_argument, NULL, 'e' }, // Run in JamLink mode
        { "clientname", required_argument, NULL, 'J' }, // Run in JamLink mode
        { "remotename", required_argument, NULL, 'K' }, // Client name on hub server
        { "rtaudio", no_argument, NULL, 'R' }, // Run in JamLink mode
        { "srate", required_argument, NULL, 'T' }, // Set Sample Rate
        { "deviceid", required_argument, NULL, 'd' }, // Set RTAudio device id to use
        { "bufsize", required_argument, NULL, 'F' }, // Set buffer Size
        { "nojackportsconnect" , no_argument, NULL,  'D'}, // Don't connect default Audio Ports
        { "version", no_argument, NULL, 'v' }, // Version Number
        { "verbose", no_argument, NULL, 'V' }, // Verbose mode
        { "hubpatch", required_argument, NULL, 'p' }, // Set hubConnectionMode for auto patch in Jack
        { "iostat", required_argument, NULL, 'I' }, // Set IO stat timeout
        { "iostatlog", required_argument, NULL, 'G' }, // Set IO stat log file
        { "effects", required_argument, NULL, 'f' }, // Turn on outgoing compressor and incoming reverb, reverbLevel arg
        { "overflowlimiting", required_argument, NULL, 'O' }, // Turn On limiter, cases 'i', 'o', 'io'
        { "assumednumclients", required_argument, NULL, 'a' }, // assumed number of clients (sound sources) (otherwise 2)
        { "bufstrategy", required_argument, NULL, OPT_BUFSTRATEGY }, // Set bufstrategy
        { "simloss", required_argument, NULL, OPT_SIMLOSS },
        { "simjitter", required_argument, NULL, OPT_SIMJITTER },
        { "broadcast", required_argument, NULL, OPT_BROADCAST },
        { "udprt", no_argument, NULL, OPT_RTUDPPRIORITY },
        { "help", no_argument, NULL, 'h' }, // Print Help
        { "examine-audio-delay", required_argument, NULL, 'x' }, // test mode - measure audio round-trip latency statistics
        { NULL, 0, NULL, 0 }
    };

    // Parse Command Line Arguments
    //----------------------------------------------------------------------------
    /// \todo Specify mandatory arguments
    int ch;
    while ((ch = getopt_long(argc, argv,
                             "n:N:H:sc:SC:o:B:P:U:q:r:b:ztlwjeJ:K:RTd:F:p:DvVhI:G:f:O:a:x:", longopts, NULL)) != -1)
        switch (ch) {

        case 'n': // Number of input and output channels
            //-------------------------------------------------------
            mNumChans = atoi(optarg);
            break;
        case 'U': // UDP Bind Port
            mServerUdpPortNum = atoi(optarg);
            break;
#ifdef WAIR
        case 'w':
            //-------------------------------------------------------
            mWAIR = true;
            mNumNetRevChans = gDefaultNumNetRevChannels; // fixed amount sets number of network channels and comb filters for WAIR
            break;
        case 'N':
            //-------------------------------------------------------
            mClientAddCombLen = atoi(optarg); // cmd line comb length adjustment
            break;
        case 'H': // comb feedback adjustment
            //-------------------------------------------------------
            mClientRoomSize = atof(optarg); // cmd line comb feedback adjustment
            break;
#endif // endwhere
        case 's': // Run in P2P server mode
            //-------------------------------------------------------
            mJackTripMode = JackTrip::SERVER;
            break;
        case 'S': // Run in Hub server mode
            //-------------------------------------------------------
            mJackTripServer = true;
            break;
        case 'c': // P2P client mode
            //-------------------------------------------------------
            mJackTripMode = JackTrip::CLIENT;
            mPeerAddress = optarg;
            break;
        case 'L': // set optional local host address
            //-------------------------------------------------------
            mLocalAddress = optarg;
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
            if (gVerboseFlag) std::cout << "SETTINGS: argument parsed for TCP Bind Port: " << mBindPortNum << std::endl;
            if (gVerboseFlag) std::cout << "SETTINGS: argument parsed for TCP Peer Port: " << mPeerPortNum << std::endl;
            break;
        case 'B': // Bind Port
            //-------------------------------------------------------
            mBindPortNum = atoi(optarg);
            if (gVerboseFlag) std::cout << "SETTINGS: argument parsed for TCP Bind Port: " << mBindPortNum << std::endl;
            break;
        case 'P': // Peer Port
            //-------------------------------------------------------
            mPeerPortNum = atoi(optarg);
            if (gVerboseFlag) std::cout << "SETTINGS: argument parsed for TCP Peer Port: " << mPeerPortNum << std::endl;
            break;
        case 'b':
            //-------------------------------------------------------
            if (atoi(optarg) == 8) {
                mAudioBitResolution = AudioInterface::BIT8;
            } else if (atoi(optarg) == 16) {
                mAudioBitResolution = AudioInterface::BIT16;
            } else if (atoi(optarg) == 24) {
                mAudioBitResolution = AudioInterface::BIT24;
            } else if (atoi(optarg) == 32) {
                mAudioBitResolution = AudioInterface::BIT32;
            } else {
                printUsage();
                std::cerr << "--bitres ERROR: Bit resolution: "
                          << atoi(optarg) << " is not supported." << endl;
                std::exit(1);
            }
            break;
        case 'q':
            //-------------------------------------------------------
            if (0 == strncmp(optarg, "auto", 4)) {
              mBufferQueueLength = -atoi(optarg+4);
              if (0 == mBufferQueueLength) {
                mBufferQueueLength = -500;
              }
            }
            else if ( atoi(optarg) <= 0 ) {
                printUsage();
                std::cerr << "--queue ERROR: The queue has to be equal or greater than 2" << endl;
                std::exit(1); }
            else {
                mBufferQueueLength = atoi(optarg);
            }
            break;
        case 'r':
            //-------------------------------------------------------
            if ( atoi(optarg) <= 0 ) {
                printUsage();
                std::cerr << "--redundancy ERROR: The redundancy has to be a positive integer" << endl;
                std::exit(1); }
            else {
                mRedundancy = atoi(optarg);
            }
            break;
        case 'z': // underrun to zero
            //-------------------------------------------------------
            mUnderrunMode = JackTrip::ZEROS;
            break;
        case 't': // quit on timeout
            mStopOnTimeout = true;
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
        case 'K': // Set Remote client Name
            //-------------------------------------------------------
            mRemoteClientName = optarg;
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
        case 'd': // RTAudio device id
            //-------------------------------------------------------
            mChanfeDefaultID = true;
            mDeviceID = atoi(optarg);
            break;
        case 'F': // Buffer Size
            //-------------------------------------------------------
            mChanfeDefaultBS = true;
            mAudioBufferSize = atoi(optarg);
            break;
        case 'D':
            //-------------------------------------------------------
            mConnectDefaultAudioPorts = false;
            break;
        case 'v':
            //-------------------------------------------------------
            cout << "JackTrip VERSION: " << gVersion << endl;
            cout << "Copyright (c) 2008-2020 Juan-Pablo Caceres, Chris Chafe." << endl;
            cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
            cout << "" << endl;
            std::exit(0);
            break;
        case 'V':
            //-------------------------------------------------------
            gVerboseFlag = true;
            if (gVerboseFlag) std::cout << "Verbose mode" << std::endl;
            mEffects.setVerboseFlag(gVerboseFlag);
            break;
        case 'p':
            //-------------------------------------------------------
            if ( atoi(optarg) == 0 ) {
                mHubConnectionMode = JackTrip::SERVERTOCLIENT;
            } else if ( atoi(optarg) == 1 ) {
                mHubConnectionMode = JackTrip::CLIENTECHO;
            } else if ( atoi(optarg) == 2 ) {
                mHubConnectionMode = JackTrip::CLIENTFOFI;
            } else if ( atoi(optarg) == 3 ) {
                mHubConnectionMode = JackTrip::RESERVEDMATRIX;
            } else if ( atoi(optarg) == 4 ) {
                mHubConnectionMode = JackTrip::FULLMIX;
            } else if ( atoi(optarg) == 5 ) {
                mHubConnectionMode = JackTrip::NOAUTO;
            } else {
                printUsage();
                std::cerr << "-p ERROR: Wrong HubConnectionMode: "
                          << atoi(optarg) << " is not supported." << endl;
                std::exit(1);
            }
            break;
        case 'I': // IO Stat timeout
            //-------------------------------------------------------
            mIOStatTimeout = atoi(optarg);
            if (0 > mIOStatTimeout) {
                printUsage();
                std::cerr << "--iostat ERROR: negative timeout." << endl;
                std::exit(1);
            }
            break;
        case 'G': // IO Stat log file
            //-------------------------------------------------------
            mIOStatStream.reset(new std::ofstream(optarg));
            if (!mIOStatStream->is_open()) {
                printUsage();
                std::cerr << "--iostatlog FAILED to open " << optarg
                          << " for writing." << endl;
                std::exit(1);
            }
            break;
        case OPT_BUFSTRATEGY: // Buf strategy
            mBufferStrategy = atoi(optarg);
            if (-1 > mBufferStrategy || 2 < mBufferStrategy) {
                std::cerr << "Unsupported buffer strategy " << optarg << endl;
                printUsage();
                std::exit(1);
            }
            break;
        case OPT_SIMLOSS: // Simulate packet loss
            mSimulatedLossRate = atof(optarg);
            break;
        case OPT_SIMJITTER: // Simulate jitter
            char* endp;
            mSimulatedJitterRate = strtod(optarg, &endp);
            if (0 == *endp) {
                mSimulatedDelayRel = 1.0;
            }
            else {
                mSimulatedDelayRel = atof(endp+1);
            }
            break;
        case OPT_BROADCAST: // Broadcast output
            mBroadcastQueue = atoi(optarg);
            break;
        case OPT_RTUDPPRIORITY: // Use RT priority for UDPDataProtocol thread
            mUseRtUdpPriority = true;
            break;
        case 'h':
            //-------------------------------------------------------
            printUsage();
            std::exit(0);
            break;
        case 'O': { // Overflow limiter (i, o, or io)
          //-------------------------------------------------------
          char cmd[] { "--overflowlimiting (-O)" };
          if (gVerboseFlag) {
            printf("%s argument = %s\n",cmd,optarg);
          }
          int returnCode = mEffects.parseLimiterOptArg(cmd,optarg);
          if (returnCode > 1) {
            mEffects.printHelp(cmd,ch);
            std::cerr << cmd << " required argument `" << optarg << "' is malformed\n";
            std::exit(1);
          } else if (returnCode == 1) {
            std::exit(0); // benign but not continuing such as "help"
          }
          break; }
        case 'a': { // assumed number of clients (applies to outgoing limiter)
          //-------------------------------------------------------
          char cmd[] { "--assumednumclients (-a)" };
          if (gVerboseFlag) {
            printf("%s argument = %s\n",cmd,optarg);
          }
          int returnCode = mEffects.parseAssumedNumClientsOptArg(cmd,optarg);
          if (returnCode > 1) {
            mEffects.printHelp(cmd,ch);
            std::cerr << cmd << " required argument `" << optarg << "' is malformed\n";
            std::exit(1);
          } else if (returnCode == 1) {
            std::exit(0); // help printed
          }
          break; }
        case 'f': { // --effects (-f) effectsSpecArg
          //-------------------------------------------------------
          char cmd[] { "--effects (-f)" };
          int returnCode = mEffects.parseEffectsOptArg(cmd,optarg);
          if (returnCode > 1) {
            mEffects.printHelp(cmd,ch);
            std::cerr << cmd << " required argument `" << optarg << "' is malformed\n";
            std::exit(1);
          } else if (returnCode == 1) {
            std::exit(0); // something benign but non-continuing like "help"
          }
          break; }
        case 'x': { // examine connection (test mode)
          //-------------------------------------------------------
          char cmd[] { "--examine-audio-delay (-x)" };
          if (tolower(optarg[0])=='h') {
            mAudioTester.printHelp(cmd,ch);
            std::exit(0);
          }
          mAudioTester.setEnabled(true);
          if (optarg == 0 || optarg[0] == '-' || optarg[0] == 0) { // happens when no -f argument specified
            printUsage();
            std::cerr << cmd << " ERROR: Print-interval argument REQUIRED (set to 0.0 to see every delay)\n";
            std::exit(1);
          }
          mAudioTester.setPrintIntervalSec(atof(optarg));
          break; }
        case ':': {
          printUsage();
          printf("*** Missing option argument *** see above for usage\n\n");
          break; }
        case '?': {
          printUsage();
          printf("*** Unknown, missing, or ambiguous option argument *** see above for usage\n\n");
          std::exit(1);
          break; }
        default: {
            //-------------------------------------------------------
            printUsage();
            printf("*** Unrecognized option -%c *** see above for usage\n",ch);
            std::exit(1);
            break; }
        }

    // Warn user if undefined options where entered
    //----------------------------------------------------------------------------
    if (optind < argc) {
      if (strcmp(argv[optind],"help")!=0) {
        cout << gPrintSeparator << endl;
        cout << "*** Unexpected command-line argument(s): ";
        for( ; optind < argc; optind++) {
          cout << argv[optind] << " ";
        }
        cout << endl << gPrintSeparator << endl;
      }
      printUsage();
      std::exit(1);
    }

    assert(mNumChans>0);
    mAudioTester.setSendChannel(mNumChans-1); // use last channel for latency testing
    // Originally, testing only in the last channel was adopted
    // because channel 0 ("left") was a clap track on CCRMA loopback
    // servers.  Now, however, we also do it in order to easily keep
    // effects in all but the last channel, enabling silent testing
    // in the last channel in parallel with normal operation of the others.

    // Exit if options are incompatible
    //----------------------------------------------------------------------------
    bool haveSomeServerMode = not ((mJackTripMode == JackTrip::CLIENT) || (mJackTripMode == JackTrip::CLIENTTOPINGSERVER));
    if (mEffects.getHaveEffect() && haveSomeServerMode) {
      std::cerr << "*** --effects (-f) ERROR: Effects not yet supported server modes (-S and -s).\n\n";
      std::exit(1);
    }
    if (mEffects.getHaveLimiter() && haveSomeServerMode) {
      if (mEffects.getLimit() != Effects::LIMITER_MODE::LIMITER_OUTGOING) { // default case
        std::cerr << "*** --overflowlimiting (-O) ERROR: Limiters not yet supported server modes (-S and -s).\n\n";
      }
      mEffects.setNoLimiters();
      // don't exit since an outgoing limiter should be the default (could exit for incoming case):
      // std::exit(1);
    }
    if (mAudioTester.getEnabled() && haveSomeServerMode) {
      std::cerr << "*** --examine-audio-delay (-x) ERROR: Audio latency measurement not supported in server modes (-S and -s)\n\n";
      std::exit(1);
    }
    if (mAudioTester.getEnabled()
        && (mAudioBitResolution != AudioInterface::BIT16)
        && (mAudioBitResolution != AudioInterface::BIT32) ) { // BIT32 not tested but should be ok
      // BIT24 should work also, but there's a comment saying it's broken right now, so exclude it
      std::cerr << "*** --examine-audio-delay (-x) ERROR: Only --bitres (-b) 16 and 32 presently supported for audio latency measurement.\n\n";
      std::exit(1);
    }
}

//*******************************************************************************
void Settings::printUsage()
{
    cout << "" << endl;
    cout << "JackTrip: A System for High-Quality Audio Network Performance" << endl;
    cout << "over the Internet" << endl;
    cout << "Copyright (c) 2008-2020 Juan-Pablo Caceres, Chris Chafe." << endl;
    cout << "SoundWIRE group at CCRMA, Stanford University" << endl;
    cout << "VERSION: " << gVersion << endl;
    cout << "" << endl;
    cout << "Usage: jacktrip [-s|-c|-S|-C hostIPAddressOrURL] [options]" << endl;
    cout << "" << endl;
    cout << "Options: " << endl;
    cout << "REQUIRED ARGUMENTS: One of:" << endl;
    cout << " -s, --server                             Run in P2P Server Mode" << endl;
    cout << " -c, --client <peer_hostname_or_IP_num>   Run in P2P Client Mode" << endl;
    cout << " -S, --jacktripserver                     Run in Hub Server Mode" << endl;
    cout << " -C, --pingtoserver <peer_name_or_IP>     Run in Hub Client Mode" << endl;
    cout << endl;
    cout << "OPTIONAL ARGUMENTS: " << endl;
    cout << " -n, --numchannels #                      Number of Input and Output Channels (default: "
         << 2 << ")" << endl;
#ifdef WAIR // WAIR
    cout << " -w, --wair                               Run in WAIR Mode" << endl;
    cout << " -N, --addcombfilterlength #              comb length adjustment for WAIR (default "
         << gDefaultAddCombFilterLength << ")" << endl;
    cout << " -H, --combfilterfeedback # (roomSize)    comb feedback adjustment for WAIR (default "
         << gDefaultCombFilterFeedback << ")" << endl;
#endif // endwhere
    cout << " -q, --queue       # (2 or more)          Queue Buffer Length, in Packet Size (default: "
         << gDefaultQueueLength << ")" << endl;
    cout << " -r, --redundancy  # (1 or more)          Packet Redundancy to avoid glitches with packet losses (default: 1)"
         << endl;
    cout << " -o, --portoffset  #                      Receiving bind port and peer port offset from default " << gDefaultPort << endl;
    cout << " -B, --bindport        #                  Set only the bind port number (default: " << gDefaultPort << ")" << endl;
    cout << " -P, --peerport        #                  Set only the peer port number (default: " << gDefaultPort << ")" << endl;
    cout << " -U, --udpbaseport                        Set only the server udp base port number (default: 61002)" << endl;
    cout << " -b, --bitres      # (8, 16, 24, 32)      Audio Bit Rate Resolutions (default: 16, 32 uses floating-point)" << endl;
    cout << " -p, --hubpatch    # (0, 1, 2, 3, 4, 5)   Hub auto audio patch, only has effect if running HUB SERVER mode, 0=server-to-clients, 1=client loopback, 2=client fan out/in but not loopback, 3=reserved for TUB, 4=full mix, 5=no auto patching (default: 0)" << endl;
    cout << " -z, --zerounderrun                       Set buffer to zeros when underrun occurs (default: wavetable)" << endl;
    cout << " -t, --timeout                            Quit after 10 seconds of no network activity" << endl;
    cout << " -l, --loopback                           Run in Loop-Back Mode" << endl;
    cout << " -j, --jamlink                            Run in JamLink Mode (Connect to a JamLink Box)" << endl;
    cout << " -J, --clientname                         Change default client name (default: JackTrip)" << endl;
    cout << " -K, --remotename                         Change default remote client name when connecting to a hub server (the default is derived from this computer's external facing IP address)" << endl;
    cout << " -L, --localaddress                       Change default local host IP address (default: 127.0.0.1)" << endl;
    cout << " -D, --nojackportsconnect                 Don't connect default audio ports in jack" << endl;
    cout << " --bufstrategy     # (0, 1, 2)            Use alternative jitter buffer" << endl;
    cout << " --broadcast <broadcast_queue>            Turn on broadcast output ports with extra queue (requires new jitter buffer)" << endl;
    cout << " --udprt                                  Use RT thread priority for network I/O" << endl;
    cout << endl;
    cout << "OPTIONAL SIGNAL PROCESSING: " << endl;
    cout << " -f, --effects # | paramString | help     Turn on incoming and/or outgoing compressor and/or reverb in Client - see `-f help' for details" << endl;
    cout << " -O, --overflowlimiting  i|o|io|n|help    Turn on audio limiter in Client, i=incoming from network, o=outgoing to network, io=both, n=no limiters (default=n)" << endl;
    cout << " -a, --assumednumclients help|# (1,2,...) Assumed number of Clients (sources) mixing at Hub Server (otherwise 2 assumed by -O)" << endl;
    cout << endl;
    cout << "ARGUMENTS TO USE JACKTRIP WITHOUT JACK:" << endl;
    cout << " -R, --rtaudio                            Use system's default sound system instead of Jack" << endl;
    cout << " -T, --srate         #                    Set the sampling rate, works on --rtaudio mode only (default: 48000)" << endl;
    cout << " -F, --bufsize       #                    Set the buffer size, works on --rtaudio mode only (default: 128)" << endl;
    cout << " -d, --deviceid      #                    The rtaudio device id --rtaudio mode only (default: 0)" << endl;
    cout << endl;
    cout << "ARGUMENTS TO DISPLAY IO STATISTICS:" << endl;
    cout << " -I, --iostat <time_in_secs>              Turn on IO stat reporting with specified interval (in seconds)" << endl;
    cout << " -G, --iostatlog <log_file>               Save stat log into a file (default: print in stdout)" << endl;
    cout << " -x, --examine-audio-delay <print_interval_in_secs> | help\n";
    cout << "                                          Print round-trip audio delay statistics. See `-x help' for details." << endl;
    cout << endl;
    cout << "ARGUMENTS TO SIMULATE NETWORK ISSUES:" << endl;
    cout << " --simloss <rate>                         Simulate packet loss" << endl;
    cout << " --simjitter <rate>,<d>                   Simulate jitter, d is max delay in packets" << endl;
    cout << endl;
    cout << "HELP ARGUMENTS: " << endl;
    cout << " -v, --version                            Prints Version Number" << endl;
    cout << " -V, --verbose                            Verbose mode, prints debug messages" << endl;
    cout << " -h, --help                               Prints this Help" << endl;
    cout << "" << endl;
}


//*******************************************************************************
UdpHubListener *Settings::getConfiguredHubServer()
{
    if (gVerboseFlag) std::cout << "JackTrip HUB SERVER TCP Bind Port: " << mBindPortNum << std::endl;
    UdpHubListener *udpHub = new UdpHubListener(mBindPortNum, mServerUdpPortNum);
    //udpHub->setSettings(this);
#ifdef WAIR // WAIR
    udpHub->setWAIR(mWAIR);
#endif // endwhere
    udpHub->setHubPatch(mHubConnectionMode);
    if (mHubConnectionMode == JackTrip::NOAUTO) {
        udpHub->setConnectDefaultAudioPorts(false);
    } else {
        udpHub->setConnectDefaultAudioPorts(mConnectDefaultAudioPorts);
    }
    // Set buffers to zero when underrun
    if ( mUnderrunMode == JackTrip::ZEROS ) {
        cout << "Setting buffers to zero when underrun..." << endl;
        cout << gPrintSeparator << std::endl;
        udpHub->setUnderRunMode(mUnderrunMode);
    }
    udpHub->setBufferQueueLength(mBufferQueueLength);

    udpHub->setBufferStrategy(mBufferStrategy);
    udpHub->setNetIssuesSimulation(mSimulatedLossRate,
        mSimulatedJitterRate, mSimulatedDelayRel);
    udpHub->setBroadcast(mBroadcastQueue);
    udpHub->setUseRtUdpPriority(mUseRtUdpPriority);
    
    if (mIOStatTimeout > 0) {
        udpHub->setIOStatTimeout(mIOStatTimeout);
        udpHub->setIOStatStream(mIOStatStream);
    }
    return udpHub;
}

JackTrip *Settings::getConfiguredJackTrip()
{
#ifdef WAIR // WAIR
    if (gVerboseFlag) std::cout << "Settings:startJackTrip mNumNetRevChans = " << mNumNetRevChans << std::endl;
#endif // endwhere
    if (gVerboseFlag) std::cout << "Settings:startJackTrip before new JackTrip" << std::endl;
    JackTrip *jackTrip = new JackTrip(mJackTripMode, mDataProtocol, mNumChans,
#ifdef WAIR // wair
                                      mNumNetRevChans,
#endif // endwhere
                                      mBufferQueueLength, mRedundancy, mAudioBitResolution,
                                      /*DataProtocol::packetHeaderTypeT PacketHeaderType = */DataProtocol::DEFAULT,
                                      /*underrunModeT UnderRunMode = */ mUnderrunMode,
                                      /* int receiver_bind_port = */ mBindPortNum,
                                      /*int sender_bind_port = */ mBindPortNum,
                                      /*int receiver_peer_port = */ mPeerPortNum,
                                      /* int sender_peer_port = */ mPeerPortNum,
                                      mPeerPortNum
                                      );
    // Set connect or not default audio ports. Only work for jack
    jackTrip->setConnectDefaultAudioPorts(mConnectDefaultAudioPorts);

    // Change client name if different from default
    if (!mClientName.isEmpty()) {
        jackTrip->setClientName(mClientName);
    }

    if (!mRemoteClientName.isEmpty() && (mJackTripMode == JackTrip::CLIENTTOPINGSERVER)) {
        jackTrip->setRemoteClientName(mRemoteClientName);
    }

    // Set buffers to zero when underrun (Actual setting is handled in constructor.)
    if (mUnderrunMode == JackTrip::ZEROS) {
        cout << "Setting buffers to zero when underrun..." << endl;
        cout << gPrintSeparator << std::endl;
    }

    jackTrip->setStopOnTimeout(mStopOnTimeout);

    // Set peer address in server mode
    if (mJackTripMode == JackTrip::CLIENT || mJackTripMode == JackTrip::CLIENTTOPINGSERVER) {
        jackTrip->setPeerAddress(mPeerAddress); }

    //        if(mLocalAddress!=QString()) // default
    //            mJackTrip->setLocalAddress(QHostAddress(mLocalAddress.toLatin1().data()));
    //        else
    //            mJackTrip->setLocalAddress(QHostAddress::Any);

    // Set Ports - Done in constructor now.
    //cout << "SETTING ALL PORTS" << endl;
    /*if (gVerboseFlag) std::cout << "Settings:startJackTrip before mJackTrip->setBindPorts" << std::endl;
    jackTrip->setBindPorts(mBindPortNum);
    if (gVerboseFlag) std::cout << "Settings:startJackTrip before mJackTrip->setPeerPorts" << std::endl;
    jackTrip->setPeerPorts(mPeerPortNum);*/

    // Set in JamLink Mode
    if ( mJamLink ) {
        cout << "Running in JamLink Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        jackTrip->setPacketHeaderType(DataProtocol::JAMLINK);
    }

    // Set in EmptyHeader Mode
    if (mEmptyHeader) {
        cout << "Running in EmptyHeader Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        jackTrip->setPacketHeaderType(DataProtocol::EMPTY);
    }

    // Set RtAudio
#ifdef __RT_AUDIO__
    if (!mUseJack) {
        mJackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
    }
#endif

    // Chanfe default Sampling Rate
    if (mChanfeDefaultSR) {
        jackTrip->setSampleRate(mSampleRate);
    }

    // Chanfe defualt device ID
    if (mChanfeDefaultID) {
        jackTrip->setDeviceID(mDeviceID);
    }

    // Chanfe default Buffer Size
    if (mChanfeDefaultBS) {
        jackTrip->setAudioBufferSizeInSamples(mAudioBufferSize);
    }
    jackTrip->setBufferStrategy(mBufferStrategy);
    jackTrip->setNetIssuesSimulation(mSimulatedLossRate,
        mSimulatedJitterRate, mSimulatedDelayRel);
    jackTrip->setBroadcast(mBroadcastQueue);
    jackTrip->setUseRtUdpPriority(mUseRtUdpPriority);

    // Add Plugins
    if (mLoopBack) {
        cout << "Running in Loop-Back Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        //std::tr1::shared_ptr<LoopBack> loopback(new LoopBack(mNumChans));
        //mJackTrip->appendProcessPlugin(loopback.get());

#if 0 // previous technique:
        LoopBack* loopback = new LoopBack(mNumChans);
        jackTrip->appendProcessPlugin(loopback);
#else // simpler method ( see AudioInterface.cpp callback() ):
        jackTrip->setLoopBack(true);
#endif

        // ----- Test Karplus Strong -----------------------------------
        //std::tr1::shared_ptr<NetKS> loopback(new NetKS());
        //mJackTrip->appendProcessPlugin(loopback);
        //loopback->play();
        //NetKS* netks = new NetKS;
        //mJackTrip->appendProcessPlugin(netks);
        //netks->play();
        // -------------------------------------------------------------
    }

    if (mIOStatTimeout > 0) {
        jackTrip->setIOStatTimeout(mIOStatTimeout);
        jackTrip->setIOStatStream(mIOStatStream);
    }

    jackTrip->setAudioTesterP(&mAudioTester);

    // Allocate audio effects in client, if any:
    int nReservedChans = mAudioTester.getEnabled() ? 1 : 0; // no fx allowed on tester channel
    std::vector<ProcessPlugin*> outgoingEffects = mEffects.allocateOutgoingEffects(mNumChans-nReservedChans);
    for (auto p : outgoingEffects) {
      jackTrip->appendProcessPluginToNetwork( p );
    }
    std::vector<ProcessPlugin*> incomingEffects = mEffects.allocateIncomingEffects(mNumChans-nReservedChans);
    for (auto p : incomingEffects) {
      jackTrip->appendProcessPluginFromNetwork( p );
    }

#ifdef WAIR // WAIR
    if ( mWAIR ) {
        cout << "Running in WAIR Mode..." << endl;
        cout << gPrintSeparator << std::endl;
        switch ( mNumNetRevChans )
        {
        case 16 :
        {
            jackTrip->appendProcessPluginFromNetwork(new ap8x2(mNumChans)); // plugin slot 0
            /////////////////////////////////////////////////////////
            Stk16* plugin = new Stk16(mNumNetRevChans);
            plugin->Stk16::initCombClient(mClientAddCombLen, mClientRoomSize);
            jackTrip->appendProcessPluginFromNetwork(plugin); // plugin slot 1
        }
            break;
        default:
            throw std::invalid_argument("Settings: mNumNetRevChans doesn't correspond to Faust plugin");
            break;
        }
            break;
        default:
            throw std::invalid_argument("Settings: mNumNetRevChans doesn't correspond to Faust plugin");
            break;
        }
    }
#endif // endwhere

    return jackTrip;
}
