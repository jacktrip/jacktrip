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
 * \file Limiter.cpp
 * \author Dominick Hing
 * \date August 2022
 * \license MIT
 */

#include "VuMeter.h"

#include <QVector>

#include "jacktrip_types.h"

//*******************************************************************************
void VuMeter::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }

    fs = float(fSamplingFreq);
    for (int i = 0; i < mNumChannels; i++) {
        vumeterP[i]->init(fs);
        // int ndx = vumeterUIP[i]->getParamIndex("NumClientsAssumed");
        // vumeterUIP[i]->setParamValue(ndx, mNumClients);
    }

    mValues.resize(mNumChannels);
    int timeout_ms = 10;

    connect(&mTimer, &QTimer::timeout, this, &VuMeter::onTick);
    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setInterval(timeout_ms);
    mTimer.setSingleShot(false);
    mTimer.start();

    inited = true;
}

//*******************************************************************************
void VuMeter::compute(int nframes, float** inputs, float** _)
{
    // Note that the second parameter is unused. This is because all of the ProcessPlugins
    // require the same function signature for the compute() function and is normally used
    // for the faust plugin output. However, this plugin is not supposed to modify the
    // signal itself like the other plugins (e.g. Limiter) do, so we don't want to write
    // to this buffer. We just need to report the VU meter output

    if (not inited) {
        std::cerr << "*** VuMeter " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Limiter " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }

    /* Creates and allocates a buffer of floats to use for VU meter outputs.
       This is the memory space where the results will be written to. */
    int numValues = nframes * mNumChannels;
    QVector<float> vumeter_buffer(numValues);

    /* Convenience variable to store the location of each channel's memory space */
    QVector<float*> vumeter_channel_ptrs(mNumChannels);

    // #ifdef SINE_TEST
    //     float sineTestOut[nframes];
    //     float* faustSigs[1]{sineTestOut};
    // #endif

    for (int i = 0; i < mNumChannels; i++) {
        // if (warningAmp > 0.0) {
        //     checkAmplitudes(nframes,
        //                     inputs[i]);  // we presently do one check across all
        //                     channels
        // }

        float* chanBufPtr = vumeter_buffer.data() + i * nframes;
        vumeterP[i]->compute(nframes, &inputs[i], &chanBufPtr);
        vumeter_channel_ptrs.push_back(chanBufPtr);

        mValues[i] =
            *(chanBufPtr + nframes / 2);  // use the first value as the VU meter value

        // #ifdef SINE_TEST
        //         limiterTestP[i]->compute(nframes, faustSigs, faustSigs);
        //         for (int n = 0; n < nframes; n++) {
        //             outputs[i][n] = outputs[i][n] + sineTestOut[n];
        //         }
        // #endif
    }
}

//*******************************************************************************
void VuMeter::onTick()
{
    emit onComputedVolumeMeasurements(mValues);
}