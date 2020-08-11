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
#include "Reverb.h"

class Effects
{
  int mNumChans;
  int gVerboseFlag = 0;
  enum LIMITER_MODE {
                     LIMITER_NONE,
                     LIMITER_INCOMING, // from network
                     LIMITER_OUTGOING, // to network
                     LIMITER_BOTH
  };
  LIMITER_MODE mLimit; ///< audio limiter controls
  unsigned int mNumClientsAssumed; ///< assumed number of clients (audio sources)

  enum InOrOut { IO_NEITHER, IO_IN, IO_OUT } io;
  bool inCompressor = false;
  bool outCompressor = false;
  bool inZitarev = false;
  bool outZitarev = false;
  bool inFreeverb = false;
  bool outFreeverb = false;
  Compressor* inCompressorP = nullptr;
  Compressor* outCompressorP = nullptr;
  Reverb* inZitarevP = nullptr;
  Reverb* outZitarevP = nullptr;
  Reverb* inFreeverbP = nullptr;
  Reverb* outFreeverbP = nullptr;
  int parenLevel = 0;
  char lastEffect = NULL;
  float compressorInLevelChange = 0;
  float compressorOutLevelChange = 0;
  float zitarevInLevel = 1.0f; // "Level" = wetness from 0 to 1
  float freeverbInLevel = 1.0f;
  float zitarevOutLevel = 1.0f;
  float freeverbOutLevel = 1.0f;
  float mReverbLevel; // for backward compatibility: 0-1 Freeverb, 1-2 Zitarev
  Limiter* inLimiterP = nullptr;
  Limiter* outLimiterP = nullptr;

public:

  Effects() :
    mNumChans(2),
    mLimit(LIMITER_NONE),
    mNumClientsAssumed(2)
  {}
     
  ~Effects() {
    if (inCompressor) { delete inCompressorP; }
    if (outCompressor) { delete outCompressorP; }
    if (inZitarev) { delete inZitarevP; }
    if (outZitarev) { delete outZitarevP; }
    if (inFreeverb) { delete inFreeverbP; }
    if (outFreeverb) { delete outFreeverbP; }
  }

  unsigned int getNumClientsAssumed() { return mNumClientsAssumed; }

  LIMITER_MODE getLimit() { return mLimit; }

  ProcessPlugin* getInCompressor() { return inCompressorP; }
  ProcessPlugin* getOutCompressor() { return outCompressorP; }
  ProcessPlugin* getInZitarev() { return inZitarevP; }
  ProcessPlugin* getOutZitarev() { return outZitarevP; }
  ProcessPlugin* getInFreeverb() { return inFreeverbP; }
  ProcessPlugin* getOutFreeverb() { return outFreeverbP; }
  ProcessPlugin* getInLimiter() { return inLimiterP; }
  ProcessPlugin* getOutLimiter() { return outLimiterP; }

  void setVerboseFlag(int v) {
    gVerboseFlag = v;
  }

  void allocateEffects(int nc) {
    mNumChans = nc;
    if (inCompressor) {
      inCompressorP = new Compressor(mNumChans);
      if (gVerboseFlag) { std::cout << "Set up INCOMING COMPRESSOR\n"; }
    }
    if (outCompressor) {
      outCompressorP = new Compressor(mNumChans);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING COMPRESSOR\n"; }
    }
    if (inZitarev) {
      inZitarevP = new Reverb(mNumChans,mNumChans, 1.0 + zitarevInLevel);
      if (gVerboseFlag) { std::cout << "Set up INCOMING REVERB (Zitarev)\n"; }
    }
    if (outZitarev) {
      outZitarevP = new Reverb(mNumChans, mNumChans, 1.0 + zitarevOutLevel);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING REVERB (Zitarev)\n"; }
    }
    if (inFreeverb) {
      inFreeverbP = new Reverb(mNumChans, mNumChans, freeverbInLevel);
      if (gVerboseFlag) { std::cout << "Set up INCOMING REVERB (Freeverb)\n"; }
    }
    if (outFreeverb) {
      outFreeverbP = new Reverb(mNumChans, mNumChans, freeverbOutLevel);
      if (gVerboseFlag) { std::cout << "Set up OUTGOING REVERB (Freeverb)\n"; }
    }
    if ( mLimit != LIMITER_NONE) {
      if ( mLimit == LIMITER_OUTGOING || mLimit == LIMITER_BOTH) {
        if (gVerboseFlag) {
          std::cout << "Set up OUTGOING LIMITER for "
                    << mNumChans << " output channels and "
                    << mNumClientsAssumed << " assumed client(s) ...\n";
        }
        outLimiterP = new Limiter(mNumChans,mNumClientsAssumed);
        // do not have mSampleRate yet, so cannot call limiter->init(mSampleRate) here
      }
      if ( mLimit == LIMITER_INCOMING || mLimit == LIMITER_BOTH) {
        if (gVerboseFlag) {
          std::cout << "Set up INCOMING LIMITER for " << mNumChans << " input channels\n";
        }
        inLimiterP = new Limiter(mNumChans,1); // mNumClientsAssumed not needed this direction
      }
    }
  }

  int parseEffectsOptArg(char* optarg) {
    int returnCode = 0;

    char c = optarg[0];
    if (not isalpha(c)) { // backward compatibility why not?, e.g., "-f 0.5"
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
    } else {
      // -f "i:[c][f|z][(reverbLevel)]], o:[c][f|z][(rl)]"
      if (gVerboseFlag) {
        std::cout << "-f (--effects) arg = " << optarg << std::endl;
      }
      ulong nac = strlen(optarg);
    
      for (ulong i=0; i<nac; i++) {
        if (optarg[i]!=')' && parenLevel>0) { continue; }
        switch(optarg[i]) {
        case ' ': break;
        case ',': break;
        case ';': break;
        case '\t': break;
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
          for (ulong j=i+1; j<nac; j++) {
            if (optarg[j] == ')') {
              optarg[j] = '\0';
              float farg = atof(&optarg[i+1]);
              optarg[j] = ')';
              if (io==IO_IN) {
                if (lastEffect == 'c') {
                  compressorInLevelChange = farg;
                } else if (lastEffect == 'z') {
                  zitarevInLevel = farg;
                } else if (lastEffect == 'f') {
                  freeverbInLevel = farg;
                } // else ignore the argument
              } else if (io==IO_OUT) {
                if (lastEffect == 'c') {
                  compressorOutLevelChange = farg;
                } else if (lastEffect == 'z') {
                  zitarevOutLevel = farg;
                } else if (lastEffect == 'f') {
                  freeverbOutLevel = farg;
                } // else ignore the argument
              } // else ignore the argument
              break;
            }
          }
          break;
        case ')': parenLevel--;
          break;
        default:
          break;
        }
      }
    }
    return returnCode;
  }
  
  int parseLimiterOptArg(char* optarg) {
    char c1 = tolower(optarg[0]);
    if (c1 == '-') {
      std::cerr << "--overflowlimiting (-O) argument i, o, or io is REQUIRED\n";
      return 1;
    }
    char c2 = (strlen(optarg)>1 ? tolower(optarg[1]) : '\0');
    if ((c1 == 'i' && c2 == 'o') || (c1 == 'o' && c2 == 'i')) { 
      mLimit = LIMITER_BOTH;
      if (gVerboseFlag) {
        std::cout << "Set up Overflow Limiter for both INCOMING and OUTGOING\n";
      }
    } else if (c1 == 'i') {
      mLimit = LIMITER_INCOMING;
      if (gVerboseFlag) {
        std::cout << "Set up Overflow Limiter for INCOMING from network\n";
      }
    } else if (c1 == 'o') {
      mLimit = LIMITER_OUTGOING;
      if (gVerboseFlag) {
        std::cout << "Set up Overflow Limiter for OUTGOING to network\n";
      }
    } else {
      mLimit = LIMITER_OUTGOING;
      if (gVerboseFlag) {
        std::cout << "Set up Overflow Limiter for OUTGOING to network\n";
      }
    }
    return 0;
  }

  int parseAssumedNumClientsOptArg(char* optarg) {
    if (optarg[0] == '-') {
      std::cerr << "--assumednumclients (-a) integer argument > 0 is REQUIRED\n";
      return 1;
    }
    mNumClientsAssumed = atoi(optarg);
    if(mNumClientsAssumed < 1) {
      std::cerr << "-p ERROR: Must have at least one assumed sound source: "
                << atoi(optarg) << " is not supported." << endl;
      return 1;
    }
  }
};

