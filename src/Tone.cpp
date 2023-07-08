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
 * \file Tone.cpp
 * \author Nelson Wang
 * \date October 2022
 * \license MIT
 */

#include "Tone.h"

#include <iostream>

#include "jacktrip_types.h"
#include "tonedsp.h"

//*******************************************************************************
Tone::Tone(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);
    for (int i = 0; i < mNumChannels; i++) {
        tonedsp* dsp_ptr = new tonedsp;
        APIUI* ui_ptr    = new APIUI;
        toneP.push_back(dsp_ptr);
        toneUIP.push_back(ui_ptr);
        dsp_ptr->buildUserInterface(ui_ptr);
    }
}

//*******************************************************************************
Tone::~Tone()
{
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<tonedsp*>(toneP[i]);
        delete static_cast<APIUI*>(toneUIP[i]);
    }
    toneP.clear();
    toneUIP.clear();
}

//*******************************************************************************
void Tone::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);
    fs = float(fSamplingFreq);

    for (int i = 0; i < mNumChannels; i++) {
        static_cast<tonedsp*>(toneP[i])->init(
            fs);  // compression filter parameters depend on sampling rate
    }
    inited = true;
}

//*******************************************************************************
void Tone::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Tone " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    for (int i = 0; i < mNumChannels; i++) {
        /* Run the signal through Faust  */
        static_cast<tonedsp*>(toneP[i])->compute(nframes, &inputs[i], &outputs[i]);
    }
}

//*******************************************************************************
void Tone::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

void Tone::triggerPlayback()
{
    for (int i = 0; i < mNumChannels; i++) {
        APIUI* ui_ptr = static_cast<APIUI*>(toneUIP[i]);
        int ndx       = ui_ptr->getParamIndex("gate");
        int v         = ui_ptr->getParamValue(ndx);
        ui_ptr->setParamValue(ndx, v + 1);
    }
}
