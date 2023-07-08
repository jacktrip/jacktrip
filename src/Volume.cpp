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
 * \file Volume.cpp
 * \author Matt Horton
 * \date September 2022
 * \license MIT
 */

#include "Volume.h"

#include <iostream>

#include "jacktrip_types.h"
#include "volumedsp.h"

//*******************************************************************************
Volume::Volume(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);
    for (int i = 0; i < mNumChannels; i++) {
        volumedsp* dsp_ptr = new volumedsp;
        APIUI* ui_ptr      = new APIUI;
        volumeP.push_back(dsp_ptr);
        volumeUIP.push_back(ui_ptr);  // #included in volumedsp.h
        dsp_ptr->buildUserInterface(ui_ptr);
    }
}

//*******************************************************************************
Volume::~Volume()
{
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<volumedsp*>(volumeP[i]);
        delete static_cast<APIUI*>(volumeUIP[i]);
    }
    volumeP.clear();
    volumeUIP.clear();
}

//*******************************************************************************
void Volume::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);
    fs = float(fSamplingFreq);

    for (int i = 0; i < mNumChannels; i++) {
        static_cast<volumedsp*>(volumeP[i])
            ->init(fs);  // compression filter parameters depend on sampling rate
        APIUI* ui_ptr = static_cast<APIUI*>(volumeUIP[i]);
        int ndx       = ui_ptr->getParamIndex("Volume");
        ui_ptr->setParamValue(ndx, mVolMultiplier);
        ndx = ui_ptr->getParamIndex("Mute");
        ui_ptr->setParamValue(ndx, isMuted ? 1 : 0);
    }
    inited = true;
}

//*******************************************************************************
void Volume::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Volume " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    for (int i = 0; i < mNumChannels; i++) {
        /* Run the signal through Faust  */
        static_cast<volumedsp*>(volumeP[i])->compute(nframes, &inputs[i], &outputs[i]);
    }
}

//*******************************************************************************
void Volume::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

void Volume::volumeUpdated(float multiplier)
{
    // maps 0.0-1.0 to a -40 dB to 0 dB range
    // update if volumedsp.dsp and/or volumedsp.h
    // change their ranges
    mVolMultiplier = 40.0 * multiplier - 40.0;
    for (int i = 0; i < mNumChannels; i++) {
        APIUI* ui_ptr = static_cast<APIUI*>(volumeUIP[i]);
        int ndx       = ui_ptr->getParamIndex("Volume");
        ui_ptr->setParamValue(ndx, mVolMultiplier);
    }
}

void Volume::muteUpdated(bool muted)
{
    isMuted = muted;
    for (int i = 0; i < mNumChannels; i++) {
        APIUI* ui_ptr = static_cast<APIUI*>(volumeUIP[i]);
        int ndx       = ui_ptr->getParamIndex("Mute");
        ui_ptr->setParamValue(ndx, isMuted ? 1 : 0);
    }
}