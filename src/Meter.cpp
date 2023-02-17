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

#include <QVector>

#include "jacktrip_types.h"

//*******************************************************************************
void Meter::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }

    fs = float(fSamplingFreq);
    for (int i = 0; i < mNumChannels; i++) {
        meterP[i]->init(fs);
    }

    /* Set meter values to the default floor */
    mValues.resize(mNumChannels);
    QVector<float>::iterator it;
    for (it = mValues.begin(); it != mValues.end(); ++it) {
        *it = threshold;
    }

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
void Meter::compute(int nframes, float** inputs, float** /*_*/)
{
    // Note that the second parameter is unused. This is because all of the ProcessPlugins
    // require the same function signature for the compute() function and is normally used
    // for the faust plugin output. However, this plugin is not supposed to modify the
    // signal itself like the other plugins (e.g. Limiter) do, so we don't want to write
    // to this buffer. We just need to report the VU meter output

    if (not inited) {
        std::cerr << "*** Meter " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Meter " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }

    QVector<float> meterBuf(nframes);
    float* meterBufPtr = meterBuf.data();
    float** output     = &meterBufPtr;
    for (int i = 0; i < mNumChannels; i++) {
        /* Run the signal through Faust  */
        meterP[i]->compute(nframes, &inputs[i], output);

        /* Use the existing value of mValues[i] as
           the threshold - this will be reset to the default floor of -80dB
           on each timeout */
        float max = mValues[i];
        QVector<float>::iterator it;
        for (it = meterBuf.begin(); it != meterBuf.end(); ++it) {
            if (*it > max) {
                max = *it;
            }
        }

        /* Update mValues */
        mValues[i] = max;
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

    mValues.resize(mNumChannels);
    QVector<float>::iterator it;
    for (it = mValues.begin(); it != mValues.end(); ++it) {
        *it = threshold;
    }
}

//*******************************************************************************
void Meter::onTick()
{
    if (hasProcessedAudio) {
        /* Send the measurements to whatever other component requests it */
        QVector<float> valuesCopy(mValues.size());
        for (int i = 0; i < mValues.size(); ++i) {
            valuesCopy[i] = mValues.at(i);
        }
        emit onComputedVolumeMeasurements(valuesCopy);

        /* Set meter values to the default floor */
        QVector<float>::iterator it;
        for (it = mValues.begin(); it != mValues.end(); ++it) {
            *it = threshold;
        }
    }
}