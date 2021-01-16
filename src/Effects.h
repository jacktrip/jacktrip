//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Julius Smith.
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
 * \file Effects.h
 * \author Julius Smith
 * \date Aug 2020
 */

#pragma once

#include "ProcessPlugin.h"
#include "Limiter.h"
#include "Compressor.h"
#include "CompressorPresets.h"
#include "Reverb.h"
#include <assert.h>
#include <vector>

class Effects
{
  int mNumIncomingChans;
  int mNumOutgoingChans;
  int gVerboseFlag = 0;
public:
  enum LIMITER_MODE {
                     LIMITER_NONE,
                     LIMITER_INCOMING, // from network
                     LIMITER_OUTGOING, // to network
                     LIMITER_BOTH
  };
private:
  LIMITER_MODE mLimit; ///< audio limiter controls
  unsigned int mNumClientsAssumed; ///< assumed number of clients (audio sources)
  double limiterWarningAmplitude;

  enum InOrOut { IO_NEITHER, IO_IN, IO_OUT } io;
  bool inCompressor = false;
  bool outCompressor = false;
  bool inZitarev = false;
  bool outZitarev = false;
  bool inFreeverb = false;
  bool outFreeverb = false;
  bool incomingEffectsAllocated = false;
  bool outgoingEffectsAllocated = false;
  Compressor* inCompressorP = nullptr;
  Compressor* outCompressorP = nullptr;
  CompressorPreset inCompressorPreset = CompressorPresets::voice; // ./CompressorPresets.h
  CompressorPreset outCompressorPreset = CompressorPresets::voice;
  Reverb* inZitarevP = nullptr;
  Reverb* outZitarevP = nullptr;
  Reverb* inFreeverbP = nullptr;
  Reverb* outFreeverbP = nullptr;
  int parenLevel = 0;
  char lastEffect = '\0';
  float zitarevInLevel = 1.0f; // "Level" = wetness from 0 to 1
  float freeverbInLevel = 1.0f;
  float zitarevOutLevel = 1.0f;
  float freeverbOutLevel = 1.0f;
  float mReverbLevel; // for backward compatibility: 0-1 Freeverb, 1-2 Zitarev
  Limiter* inLimiterP = nullptr;
  Limiter* outLimiterP = nullptr;

public:

  Effects(bool outGoingLimiterOn=true) :
    mNumIncomingChans(2),
    mNumOutgoingChans(2),
    mLimit(outGoingLimiterOn ? LIMITER_OUTGOING : LIMITER_NONE),
    mNumClientsAssumed(2),
    limiterWarningAmplitude(0.0)
  {}

  ~Effects() {
    /*
      Plugin ownership presently passes to JackTrip,
      and deletion occurs in AudioInterface.cpp. See
        delete mProcessPluginsFromNetwork[i];
        delete mProcessPluginsToNetwork[i];
      there.  If/when we ever do it here:
        if (inCompressor) { delete inCompressorP; }
        if (outCompressor) { delete outCompressorP; }
        if (inZitarev) { delete inZitarevP; }
        if (outZitarev) { delete outZitarevP; }
        if (inFreeverb) { delete inFreeverbP; }
        if (outFreeverb) { delete outFreeverbP; }
      but if everyone can compile C++11,
      let's switch to using std::unique_ptr.
    */
  }

  unsigned int getNumClientsAssumed() { return mNumClientsAssumed; }

  LIMITER_MODE getLimit() { return mLimit; }
  void setNoLimiters() { mLimit = LIMITER_NONE; }

  ProcessPlugin* getInCompressor() { return inCompressorP; }
  ProcessPlugin* getOutCompressor() { return outCompressorP; }
  ProcessPlugin* getInZitarev() { return inZitarevP; }
  ProcessPlugin* getOutZitarev() { return outZitarevP; }
  ProcessPlugin* getInFreeverb() { return inFreeverbP; }
  ProcessPlugin* getOutFreeverb() { return outFreeverbP; }
  ProcessPlugin* getInLimiter() { return inLimiterP; }
  ProcessPlugin* getOutLimiter() { return outLimiterP; }

  bool getHaveEffect() {
    return
      inCompressor || outCompressor ||
      inZitarev || outZitarev ||
      inFreeverb || outFreeverb ;
  }

  bool getHaveLimiter() {
    return mLimit != LIMITER_NONE;
  }

  void setVerboseFlag(int v) {
    gVerboseFlag = v;
  }

  int getNumIncomingChans() {
    return mNumIncomingChans;
  }

  int getOutgoingNumChans() {
    return mNumOutgoingChans;
  }

  // call these next two after it is decided what effects we will be using for the duration:

  std::vector<ProcessPlugin*> allocateIncomingEffects(int nIncomingChans) {
    mNumIncomingChans = nIncomingChans;
    if (incomingEffectsAllocated) {
      std::cerr << "*** Effects.h: attempt to allocate incoming effects more than once\n";
      std::exit(1);
    }
    std::vector<ProcessPlugin*> incomingEffects;
    if (inCompressor) {
      assert(inCompressorP == nullptr);
      inCompressorP = new Compressor(mNumIncomingChans, gVerboseFlag, inCompressorPreset);
      if (gVerboseFlag) { std::cout << "Set up INCOMING COMPRESSOR\n"; }
      incomingEffects.push_back(inCompressorP);
    }
    if (inZitarev) {
      assert(inZitarevP == nullptr);
      inZitarevP = new Reverb(mNumIncomingChans,mNumIncomingChans, 1.0 + zitarevInLevel);
      if (gVerboseFlag) { std::cout << "Set up INCOMING REVERB (Zitarev)\n"; }
      incomingEffects.push_back(inZitarevP);
    }
    if (inFreeverb) {
      assert(inFreeverbP == nullptr);
      inFreeverbP = new Reverb(mNumIncomingChans, mNumIncomingChans, freeverbInLevel);
      if (gVerboseFlag) { std::cout << "Set up INCOMING REVERB (Freeverb)\n"; }
      incomingEffects.push_back(inFreeverbP);
    }
    // LIMITER MUST GO LAST:
    if ( mLimit == LIMITER_INCOMING || mLimit == LIMITER_BOTH) {
      if (gVerboseFlag) {
        std::cout << "Set up INCOMING LIMITER for " << mNumIncomingChans << " input channels\n";
      }
      assert(inLimiterP == nullptr);
      inLimiterP = new Limiter(mNumIncomingChans, 1, gVerboseFlag); // mNumClientsAssumed not needed this direction
      // Never needed in normal practice for incoming limiter: inLimiterP->setWarningAmplitude(limiterWarningAmplitude);
      incomingEffects.push_back(inLimiterP);
    }
    incomingEffectsAllocated = true;
    return incomingEffects;
  }

  std::vector<ProcessPlugin*> allocateOutgoingEffects(int nOutgoingChans) {
    mNumOutgoingChans = nOutgoingChans;
    if (outgoingEffectsAllocated) {
      std::cerr << "*** Effects.h: attempt to allocate outgoing effects more than once\n";
      std::exit(1);
    }
    std::vector<ProcessPlugin*> outgoingEffects;
    if (outCompressor) {
      assert(outCompressorP == nullptr);
      outCompressorP = new Compressor(mNumOutgoingChans, gVerboseFlag, outCompressorPreset);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING COMPRESSOR\n"; }
      outgoingEffects.push_back(outCompressorP);
    }
    if (outZitarev) {
      assert(outZitarevP == nullptr);
      outZitarevP = new Reverb(mNumOutgoingChans, mNumOutgoingChans, 1.0 + zitarevOutLevel);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING REVERB (Zitarev)\n"; }
      outgoingEffects.push_back(outZitarevP);
    }
    if (outFreeverb) {
      assert(outFreeverbP == nullptr);
      outFreeverbP = new Reverb(mNumOutgoingChans, mNumOutgoingChans, freeverbOutLevel);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING REVERB (Freeverb)\n"; }
      outgoingEffects.push_back(outFreeverbP);
    }
    // LIMITER MUST GO LAST:
    if ( mLimit != LIMITER_NONE) {
      if ( mLimit == LIMITER_OUTGOING || mLimit == LIMITER_BOTH) {
        if (gVerboseFlag) {
          std::cout << "Set up OUTGOING LIMITER for "
                    << mNumOutgoingChans << " output channels and "
                    << mNumClientsAssumed << " assumed client(s) ...\n";
        }
        assert(outLimiterP == nullptr);
        outLimiterP = new Limiter(mNumOutgoingChans,mNumClientsAssumed);
        outLimiterP->setWarningAmplitude(limiterWarningAmplitude);
        // do not have mSampleRate yet, so cannot call limiter->init(mSampleRate) here
        outgoingEffects.push_back(outLimiterP);
      }
    }
    outgoingEffectsAllocated = true;
    return outgoingEffects;
  }

  void printHelp(char* command, char helpCase) {
    std::cout << "HELP for `" << command << "' (end-of-line comments start with `//')\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "\n";
    if (helpCase == 0 || helpCase == 'f') { //
      std::cout << command << " 0.3 // add a default outgoing compressor (for voice) and incoming reverb (freeverb) with wetness 0.3 (wetness from 0 to 1)\n";
      std::cout << command << " 1.3 // add a default outgoing compressor (for voice) and incoming reverb (zitarev) with wetness 0.3 = 1.3-1 (i.e., 1+ to 2 is for zitarev)\n";
      std::cout << "\n";
      std::cout << command << " \"o:c i:f(0.3)\" // outgoing-compressor and incoming-freeverb example above using more general string argument\n";
      std::cout << command << " \"o:c i:z(0.3)\" // outgoing-compressor and incoming-zitarev example above using more general string argument\n";
      std::cout << command << " \"o:c(1)\" // outgoing compressor, using preset 1 (designed for voice - see below for details)\n";
      std::cout << command << " \"o:c(2)\" // outgoing compressor, using preset 2 (for horns)\n";
      std::cout << command << " \"o:c(3)\" // outgoing compressor, using preset 3 (for snare)\n";
      std::cout << command << " \"o:c(c:compressionRatio t:thresholdDB a:attackTimeMS r:releaseTimeMS g:makeUpGainDB)\" // general compression parameter specification (all floats)\n";
      std::cout << command << " \"o:c(c:2 t:-24 a:15 r:40 g:2)\"   // outgoing compressor, preset 1 details\n";
      std::cout << command << " \"o:c(c:3 t:-10 a:100 r:250 g:2)\" // outgoing compressor, preset 2 details\n";
      std::cout << command << " \"o:c(c:5 t:-4 a:5 r:150 g:3)\"    // outgoing compressor, preset 3 details\n";
      std::cout << "  For these and more suggested compression settings, see http://www.anythingpeaceful.org/sonar/settings/comp.html\n";
      std::cout << "\n";
    }
    if (helpCase == 0 || helpCase == 'O') { // limiter (-O option most likely)
      std::cout << command << " i   // add limiter to INCOMING audio from network (only helpful for floats, i.e., -b32 used by server)\n";
      std::cout << command << " o   // add limiter to OUTGOING audio to network (prevents your sound from harshly clipping going out)\n";
      std::cout << command << " ow  // also warn and advise on levels when outgoing limiter compresses audio near clipping\n";
      std::cout << command << " io  // add limiter to both INCOMING and OUTGOING audio\n";
      std::cout << command << " iow // limiters both ways and compression warnings on outgoing direction only\n";
      std::cout << "\n";
    }
    if (helpCase == 0 || helpCase == 'a') { // assumedNumClients (-a option)
      std::cout << command << " 1 // assume 1 client - fine for loopback test, or if only one client plays at a time, or server uses -b32 and -Oi is used\n";
      std::cout << command << " 2 // assume 2 clients possibly playing at the same time\n";
      std::cout << command << " N // any integer N>0 can be used - the outgoing limiter will divide final amplitude by 1/sqrt(N) to reduce overages in server\n";
      std::cout << "\n";
    }
  }

  // ----------- Compressor stuff --------------

  int setCompressorPresetIndexFrom1(unsigned long presetIndexFrom1, InOrOut io) {
    int returnCode = 0;
    if (presetIndexFrom1 <= 0 || presetIndexFrom1 > CompressorPresets::numPresets) {
      std::cerr << "*** Effects.h: setCompressorPresetFrom1: Index " << presetIndexFrom1 << " out of range\n";
      returnCode = 1;
    } else {
      CompressorPreset stdPreset = CompressorPresets::standardPresets[presetIndexFrom1-1];
      if (io == IO_IN) {
        inCompressorPreset = stdPreset;
      } else if (io == IO_OUT) {
        outCompressorPreset = stdPreset;
      } else if (io != IO_NEITHER) {
        std::cerr << "*** Effects.h: setCompressorPresetFrom1: Invalid InOrOut value " << io << "\n";
        returnCode = 1;
      }
    }
    return returnCode;
  }

  int parseCompresserArgs(char* args, InOrOut inOrOut) {
    // args can be integerPresetNumberFrom1 or (all optional, any order):
    // c:compressionRatio, a:attackTimeMS, r:releaseTimeMS, g:makeUpGain
    int returnCode = 0;
    if (not isalpha(args[0])) {
      int presetIndexFrom1 = atoi(args);
      setCompressorPresetIndexFrom1(presetIndexFrom1,inOrOut);
    } else {
      // args can be presetIndexFrom1, handled above, or (all optional, any order):
      // c(c:compressionRatio, t:thresholdDB, a:attackTimeMS, r:releaseTimeMS, g:makeUpGainDB)
      // See ./CompressorPresets.h for example settings.
      if (gVerboseFlag) {
        std::cout << "parseCompressorArgs = " << args << std::endl;
      }
      ulong argLen = strlen(args);
      char lastParam = '\0';

      CompressorPreset newPreset(CompressorPresets::voice); // Anything unset gets voice value (most gentle)

      int nSkip = 0;
      for (ulong i=0; i<argLen; i++) {
        if (nSkip > 0) {
          nSkip--;
          continue;
        }
        char ch = args[i];
        switch(ch) {
        case ' ': break;
        case '\t': break;
        case 'c': case 't': case 'a': case 'r': case 'g':
          lastParam = ch;
          break;
        case ':': break;
        default: // must be a floating-point number at this point:
          if (ch!='-' && isalpha(ch)) {
            std::cerr << "*** Effects.h: parseCompressorArgs: " << ch << " not recognized in args = " << args << "\n";
            returnCode = 2;
          } else { // must have a digit or '-' or '.'
            assert(ch=='-'||ch=='.'||isdigit(ch));
            float paramValue = -1.0e10;
            for (ulong j=i; j<argLen; j++) { // scan ahead for end of number
              if (args[j] == ',' || args[j] == ' ' || j==argLen-1) { // comma or space required between parameters
                char argsj = args[j];
                if (j<argLen-1) { // there's more
                  args[j] = '\0';
                }
                paramValue = atof(&args[i]);
                args[j] = argsj;
                nSkip = j-i;
                break;
              }
            }
            if (paramValue == -1.0e10) {
              std::cerr << "*** Effects.h: parseCompressorArgs: Could not find parameter for "
                        << lastParam << " in args = " << args << "\n";
              returnCode = 2;
            } else {
              switch (lastParam) {
              case 'c':
                newPreset.ratio = paramValue;
                break;
              case 't':
                  newPreset.thresholdDB = paramValue;
                break;
              case 'a':
                  newPreset.attackMS = paramValue;
                break;
              case 'r':
                  newPreset.releaseMS = paramValue;
                break;
              case 'g':
                  newPreset.makeUpGainDB = paramValue;
                break;
              default: // cannot happen:
                std::cerr << "*** Effects.h: parseCompressorArgs: lastParam " << lastParam << " invalid\n";
                returnCode = 3; // "reality failure"
              } // switch(lastParam)
            } // have valid parameter from atof
          } // have valid non-alpha char for parameter
        } // switch(ch)
      } // for (ulong i=0; i<argLen; i++) {
      if (inOrOut == IO_IN) {
        inCompressorPreset = newPreset;
      } else if (inOrOut == IO_OUT) {
        outCompressorPreset = newPreset;
      } else if (inOrOut != IO_NEITHER) {
        std::cerr << "*** Effects.h: parseCompressorArgs: invalid InOrOut value " << inOrOut << "\n";
        returnCode = 2;
      }
    } // long-form compressor args
    return returnCode;
  } // int parseCompresserArgs(char* args, InOrOut inOrOut)

  // ============== General argument processing for all effects =================

  int parseEffectsOptArg(char* cmd, char* optarg) {
    int returnCode = 0; // 0 means go, 1 means exit without error, higher => error exit

    char c = optarg[0];
    if (c == '-' || c==0) {
      // happens when no -f argument specified
      returnCode = 2;
    } else if (not isalpha(c)) { // backward compatibility why not?, e.g., "-f 0.5"
      // -f reverbLevelFloat
      mReverbLevel = atof(optarg);
      outCompressor = true;
      inZitarev = mReverbLevel > 1.0;
      inFreeverb = mReverbLevel <= 1.0;
      if (inZitarev) {
        zitarevInLevel = mReverbLevel - 1.0; // wetness from 0 to 1
      }
      if (inFreeverb) {
        freeverbInLevel = mReverbLevel; // wetness from 0 to 1
      }
    } else { // long-form argument:
      // -f "i:[c][f|z][(reverbLevel)]], o:[c][f|z][(rl)]"
      // c can be c(integerPresetNumberFrom1) or (all optional, any order):
      // c(c:compressionRatio, a:attackTimeMS, r:releaseTimeMS, g:makeUpGain)
      if (gVerboseFlag) {
        std::cout << cmd << " argument = " << optarg << std::endl;
      }
      ulong argLen = strlen(optarg);

      for (ulong i=0; i<argLen; i++) {
        if (optarg[i]!=')' && parenLevel>0) { continue; }
        switch(optarg[i]) {
        case ' ': break;
        case ',': break;
        case ';': break;
        case '\t': break;
        case 'h': printHelp(cmd,'f'); returnCode = 1; break;
        case 'i': io=IO_IN; break;
        case 'o': io=IO_OUT; break;
        case ':': break;
        case 'c': if (io==IO_IN) { inCompressor = true; } else if (io==IO_OUT) { outCompressor = true; }
          else { std::cerr << "-f arg `" << optarg << "' malformed\n"; exit(1); }
          lastEffect = 'c';
          break;
        case 'f': if (io==IO_IN) { inFreeverb = true; } else if (io==IO_OUT) { outFreeverb = true; }
          else { std::cerr << "-f arg `" << optarg << "' malformed\n"; exit(1); }
          lastEffect = 'f';
          break;
        case 'z': if (io==IO_IN) { inZitarev = true; } else if (io==IO_OUT) { outZitarev = true; }
          else { std::cerr << "-f arg `" << optarg << "' malformed\n"; exit(1); }
          lastEffect = 'z';
          break;
        case '(': parenLevel++;
          for (ulong j=i+1; j<argLen; j++) {
            if (optarg[j] == ')') {
              optarg[j] = '\0';
              switch(lastEffect) {
              case 'c': {
                returnCode += parseCompresserArgs(&optarg[i+1],io);
                break; }
              case 'z': {
              float farg = atof(&optarg[i+1]);
              if (io==IO_IN) {
                  zitarevInLevel = farg;
              } else if (io==IO_OUT) {
                  zitarevOutLevel = farg;
                } // else ignore the argument
                break; }
              case 'f': {
                float farg = atof(&optarg[i+1]);
                if (io==IO_IN) {
                  freeverbInLevel = farg;
                } else if (io==IO_OUT) {
                  freeverbOutLevel = farg;
                } // else ignore the argument
                break; }
              default: { // ignore
                break; }
              }
              optarg[j] = ')';
              break;
            }
          }
          break;
        case ')': parenLevel--;
          break;
        default:
          break; // ignore
        } // switch(optarg[i])
      }
    }
    return returnCode;
  }

  int parseLimiterOptArg(char* cmd, char* optarg) {
    int returnCode = 0;
    lastEffect = 'O'; // OverflowLimiter
    char ch = tolower(optarg[0]);
    if (ch == '-' || ch == 0) {
      std::cerr << cmd << " argument i, o, or io is REQUIRED\n";
      returnCode = 2;
    } else if (ch == 'h') {
      printHelp(cmd,'O');
      returnCode = 1;
    } else {
      bool haveIncoming = false;
      bool haveOutgoing = false;
      bool haveWarnings = false;
      for (int i=0; i<strlen(optarg); i++) {
        ch = tolower(optarg[i]);
        switch(ch) {
        case ' ': break;
        case '\t': break;
        case 'i':
          haveIncoming = true;
          break;
        case 'o':
          haveOutgoing = true;
          break;
        case 'w':
          haveWarnings = true;
          break;
        case 'n':
          haveIncoming = false;
          haveOutgoing = false;
          break;
        default:
          std::cerr << "*** Effects.h: parseLimiterOptArg: Unrecognized option " << ch << "\n";
          returnCode = 2;
        } // switch(ch)
      } // process optarg char ch
      mLimit = (haveIncoming && haveOutgoing ? LIMITER_BOTH
                : (haveIncoming ? LIMITER_INCOMING
                   : (haveOutgoing ? LIMITER_OUTGOING : LIMITER_NONE)));
      if (haveWarnings) {
        limiterWarningAmplitude = 0.5; // KEEP IN SYNC WITH LIMITER THRESHOLD/CEILING 'softClipLevel' in ../faust-src/limiterdsp.dsp
        // the warning amplitude and limiter compression threshold can of course be brought as a parameters, e.g. w(0.5)
      }
      if (gVerboseFlag) {
        if(haveIncoming) {
          std::cout << "Set up INCOMING Overflow Limiter\n";
        }
        if(haveOutgoing) {
          std::cout << "Set up OUTGOING Overflow Limiter\n";
        }
        if(haveWarnings) {
          std::cout << "Enable DISTORTION WARNINGS in Overflow Limiters\n";
        }
        if(not haveIncoming and not haveOutgoing) {
          std::cout << "Set up NO Overflow Limiters\n";
        }
      } // gVerboseFlag
    } // optarg cases
    return returnCode;
  } // parseLimiterOptArg()

  int parseAssumedNumClientsOptArg(char* cmd, char* optarg) {
    int returnCode = 0;
    lastEffect = 'a'; // assumedNumClients
    char ch = optarg[0];
    if (ch == 'h') {
      printHelp(cmd,'a');
      returnCode = 1;
    } else if (ch == '-' || isalpha(ch) || ch == 0) {
      std::cerr << cmd << " argument help or integer > 0 is REQUIRED\n";
      returnCode = 2;
    } else {
    mNumClientsAssumed = atoi(optarg);
    if(mNumClientsAssumed < 1) {
      std::cerr << "-p ERROR: Must have at least one assumed sound source: "
                << atoi(optarg) << " is not supported." << std::endl;
        returnCode = 2;
      }
    }
    return returnCode;
  }

};
