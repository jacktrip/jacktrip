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
 * \file Monitor.cpp
 * \author Dominick Hing
 * \date May 2023
 * \license MIT
 */

#include "Monitor.h"

#include <iostream>

#include "jacktrip_types.h"
#include "monitordsp.h"

//*******************************************************************************
Monitor::Monitor(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);
    for (int i = 0; i < mNumChannels; i++) {
        monitordsp* dsp_ptr = new monitordsp;
        APIUI* ui_ptr       = new APIUI;
        monitorP.push_back(dsp_ptr);
        monitorUIP.push_back(ui_ptr);  // #included in monitordsp.h
        dsp_ptr->buildUserInterface(ui_ptr);
    }
}

//*******************************************************************************
Monitor::~Monitor()
{
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<monitordsp*>(monitorP[i]);
        delete static_cast<APIUI*>(monitorUIP[i]);
    }
    monitorP.clear();
    monitorUIP.clear();
}

//*******************************************************************************
void Monitor::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }
    fs = float(fSamplingFreq);

    for (int i = 0; i < mNumChannels; i++) {
        static_cast<monitordsp*>(monitorP[i])
            ->init(fs);  // compression filter parameters depend on sampling rate
        APIUI* ui_ptr = static_cast<APIUI*>(monitorUIP[i]);
        int ndx       = ui_ptr->getParamIndex("Volume");
        ui_ptr->setParamValue(ndx, mVolMultiplier);
        ndx = ui_ptr->getParamIndex("Mute");
        ui_ptr->setParamValue(ndx, 0);
    }
    inited = true;
}

//*******************************************************************************
void Monitor::compute(int nframes, float** inputs, float** outputs)
{
    if (not inited) {
        std::cerr << "*** Monitor " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Monitor " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }

    if (mBufSize < nframes) {
        if (mOutBufferInput) {
            delete mOutBufferInput;
        }

        if (mInBufferInput) {
            delete mInBufferInput;
        }

        mBufSize        = nframes;
        mOutBufferInput = new float[mBufSize];
        mInBufferInput  = new float[mBufSize];
    }

    std::vector<float*> buffer{mInBufferInput, mOutBufferInput};
    for (int i = 0; i < mNumChannels; i++) {
        // copy inputs and outputs into a separate memory buffer
        memcpy(mInBufferInput, inputs[i], nframes * sizeof(float));
        memcpy(mOutBufferInput, outputs[i], nframes * sizeof(float));

        /* Run the signal through Faust  */
        static_cast<monitordsp*>(monitorP[i])
            ->compute(nframes, buffer.data(), &outputs[i]);
    }
}

//*******************************************************************************
void Monitor::updateNumChannels(int nChansIn, int nChansOut)
{
    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }
}

//*******************************************************************************
void Monitor::volumeUpdated(float multiplier)
{
    // maps 0.0-1.0 to a -40 dB to 0 dB range
    // update if volumedsp.dsp and/or volumedsp.h
    // change their ranges
    mVolMultiplier = 40.0 * multiplier - 40.0;
    for (int i = 0; i < mNumChannels; i++) {
        APIUI* ui_ptr = static_cast<APIUI*>(monitorUIP[i]);
        int ndx       = ui_ptr->getParamIndex("Volume");
        ui_ptr->setParamValue(ndx, mVolMultiplier);
    }
}