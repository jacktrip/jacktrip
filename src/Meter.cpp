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
 * \file Meter.cpp
 * \author Dominick Hing
 * \date August 2022
 * \license MIT
 */

#include "Meter.h"

#include <algorithm>
#include <iostream>

#include "jacktrip_types.h"
#include "meterdsp.h"

//*******************************************************************************
Meter::Meter(int numchans, bool verboseFlag) : mNumChannels(numchans)
{
    setVerbose(verboseFlag);
    for (int i = 0; i < mNumChannels; i++) {
        meterP.push_back(new meterdsp);
    }
}

//*******************************************************************************
Meter::~Meter()
{
    mTimer.stop();
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<meterdsp*>(meterP[i]);
    }
    meterP.clear();
    if (mValues) {
        delete mValues;
    }
    if (mOutValues) {
        delete mOutValues;
    }
    if (mBuffer) {
        delete mBuffer;
    }
}

//*******************************************************************************
void Meter::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);

    fs = float(fSamplingFreq);
    for (int i = 0; i < mNumChannels; i++) {
        static_cast<meterdsp*>(meterP[i])->init(fs);
    }

    /* Set meter values to the default floor */
    setupValues();

    /* Start timer */
    int timeout_ms = 100;
    connect(&mTimer, &QTimer::timeout, this, &Meter::onTick);
    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setInterval(timeout_ms);
    mTimer.setSingleShot(false);
    mTimer.start();

    inited = true;
}

//*******************************************************************************
void Meter::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Meter " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }

    // Will measure inputs by default unless mMeasureOutputBuffer = true,
    // in which case the plugin will measure from the outputs. This is useful when
    // measuring with a monitor, since AudioInterface.cpp expects monitoring plugins
    // to behave differently than input and output chain plugins
    float** measuringBuffer = inputs;
    if (mIsMonitoringMeter) {
        measuringBuffer = outputs;
    }

    if (mBufSize < nframes) {
        if (mBuffer) {
            delete mBuffer;
        }
        mBufSize = nframes;
        mBuffer  = new float[mBufSize];
    }

    for (int i = 0; i < mNumChannels; i++) {
        /* Run the signal through Faust  */
        static_cast<meterdsp*>(meterP[i])->compute(nframes, &measuringBuffer[i],
                                                   &mBuffer);

        /* Use the existing value of mValues[i] as
           the threshold - this will be reset to the default floor of -80dB
           on each timeout */
        float maxSample = *(std::max_element)(mBuffer, mBuffer + nframes);

        /* Update mValues */
        mValues[i] = std::max<float>(mValues[i], maxSample);
    }

    /* Set processed audio flag */
    hasProcessedAudio = true;
}

//*******************************************************************************
void Meter::updateNumChannels(int nChansIn, int nChansOut)
{
    // this should only be called before init!
    if (inited) {
        return;
    }

    if (outgoingPluginToNetwork) {
        mNumChannels = nChansIn;
    } else {
        mNumChannels = nChansOut;
    }

    setupValues();
}

void Meter::setupValues()
{
    if (mValues) {
        float* oldValues = mValues;
        // Delete our old array after 5 seconds.
        QTimer::singleShot(5000, this, [=]() {
            delete oldValues;
        });
    }
    if (mOutValues) {
        float* oldOut = mOutValues;
        QTimer::singleShot(5000, this, [=]() {
            delete oldOut;
        });
    }
    mValues    = new float[mNumChannels];
    mOutValues = new float[mNumChannels];
    for (int i = 0; i < mNumChannels; i++) {
        mValues[i]    = threshold;
        mOutValues[i] = threshold;
    }
}

//*******************************************************************************
void Meter::onTick()
{
    /* Set meter values to the default floor */
    for (int i = 0; i < mNumChannels; i++) {
        mOutValues[i] = mValues[i];
        mValues[i]    = threshold;
    }

    if (hasProcessedAudio) {
        /* Send the measurements to whatever other component requests it */
        emit onComputedVolumeMeasurements(mOutValues, mNumChannels);
    }
}
