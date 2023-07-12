//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Julius Smith, Juan-Pablo Caceres, Chris Chafe.
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
 * \file Reverb.cpp
 * \author Julius Smith, based on Limiter.h
 * \date August 2020
 */

#include "Reverb.h"

#include "freeverbdsp.h"  // stereo in and out
#include "freeverbmonodsp.h"  // mono in and out (there is no mono to stereo case in jacktrip as yet)
#include "jacktrip_types.h"
#include "zitarevdsp.h"      // stereo in and out
#include "zitarevmonodsp.h"  // mono in and out

//*******************************************************************************
Reverb::Reverb(int numInChans, int numOutChans, float reverbLevel, bool verboseFlag)
    : mNumInChannels(numInChans), mNumOutChannels(numOutChans), mReverbLevel(reverbLevel)
{
    setVerbose(verboseFlag);
    if (mNumInChannels < 1) {
        std::cerr << "*** Reverb.h: must have at least one input audio channels\n";
        mNumInChannels = 1;
    }
    if (mNumInChannels > 2) {
        std::cerr << "*** Reverb.h: limiting number of audio output channels to 2\n";
        mNumInChannels = 2;
    }
#if 0
std::cout << "Reverb: constructed for "
            << mNumInChannels << " input channels and "
            << mNumOutChannels << " output channels with reverb level = "
            << mReverbLevel << "\n";
#endif

    if (mReverbLevel <= 1.0) {                    // freeverb:
        freeverbStereoP   = new freeverbdsp;      // stereo input and output
        freeverbMonoP     = new freeverbmonodsp;  // mono input, stereo output
        freeverbStereoUIP = new APIUI;            // #included in *dsp.h
        freeverbMonoUIP   = new APIUI;
        static_cast<freeverbdsp*>(freeverbStereoP)
            ->buildUserInterface(static_cast<APIUI*>(freeverbStereoUIP));
        static_cast<freeverbmonodsp*>(freeverbMonoP)
            ->buildUserInterface(static_cast<APIUI*>(freeverbMonoUIP));
        // std::cout << "Using freeverb\n";
    } else {
        zitarevStereoP   = new zitarevdsp;      // stereo input and output
        zitarevMonoP     = new zitarevmonodsp;  // mono input, stereo output
        zitarevStereoUIP = new APIUI;
        zitarevMonoUIP   = new APIUI;
        static_cast<zitarevdsp*>(zitarevStereoP)
            ->buildUserInterface(static_cast<APIUI*>(zitarevStereoUIP));
        static_cast<zitarevmonodsp*>(zitarevMonoP)
            ->buildUserInterface(static_cast<APIUI*>(zitarevMonoUIP));
        // std::cout << "Using zitarev\n";
    }
}

//*******************************************************************************
Reverb::~Reverb()
{
    if (mReverbLevel <= 1.0) {  // freeverb:
        delete static_cast<freeverbdsp*>(freeverbStereoP);
        delete static_cast<freeverbmonodsp*>(freeverbMonoP);
        delete static_cast<APIUI*>(freeverbStereoUIP);
        delete static_cast<APIUI*>(freeverbMonoUIP);
    } else {
        delete static_cast<zitarevdsp*>(zitarevStereoP);
        delete static_cast<zitarevmonodsp*>(zitarevMonoP);
        delete static_cast<APIUI*>(zitarevStereoUIP);
        delete static_cast<APIUI*>(zitarevMonoUIP);
    }
}

//*******************************************************************************
void Reverb::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);
    fs = float(fSamplingFreq);
    if (mReverbLevel <= 1.0) {  // freeverb:
        static_cast<freeverbdsp*>(freeverbStereoP)
            ->init(fs);  // compression filter parameters depend on sampling rate
        static_cast<freeverbmonodsp*>(freeverbMonoP)
            ->init(fs);  // compression filter parameters depend on sampling rate
        int ndx = static_cast<APIUI*>(freeverbStereoUIP)->getParamIndex("Wet");
        static_cast<APIUI*>(freeverbStereoUIP)->setParamValue(ndx, mReverbLevel);
        static_cast<APIUI*>(freeverbMonoUIP)->setParamValue(ndx, mReverbLevel);
    } else {  // zitarev:
        static_cast<zitarevdsp*>(zitarevStereoP)
            ->init(fs);  // compression filter parameters depend on sampling rate
        static_cast<zitarevmonodsp*>(zitarevMonoP)
            ->init(fs);  // compression filter parameters depend on sampling rate
        int ndx = static_cast<APIUI*>(zitarevStereoUIP)->getParamIndex("Wet");
        float zitaLevel =
            mReverbLevel - 1.0f;  // range within zitarev is 0 to 1 (our version only)
        static_cast<APIUI*>(zitarevStereoUIP)->setParamValue(ndx, zitaLevel);
        static_cast<APIUI*>(zitarevMonoUIP)->setParamValue(ndx, zitaLevel);
    }
    inited = true;
}

//*******************************************************************************
void Reverb::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Reverb " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }
    if (mReverbLevel <= 1.0) {
        if (mNumInChannels == 1) {
            static_cast<freeverbmonodsp*>(freeverbMonoP)
                ->compute(nframes, inputs, outputs);
        } else {
            assert(mNumInChannels == 2);
            static_cast<freeverbdsp*>(freeverbStereoP)->compute(nframes, inputs, outputs);
        }
    } else {
        if (mNumInChannels == 1) {
            static_cast<zitarevmonodsp*>(zitarevMonoP)->compute(nframes, inputs, outputs);
        } else {
            assert(mNumInChannels == 2);
            static_cast<zitarevdsp*>(zitarevStereoP)->compute(nframes, inputs, outputs);
        }
    }
}
