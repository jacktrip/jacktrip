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
 * \file Meter.h
 * \author Dominick Hing
 * \date August 2022
 * \license MIT
 */

#ifndef __METER_H__
#define __METER_H__

#include <QObject>
#include <QTimer>
#include <iostream>
#include <vector>

#include "ProcessPlugin.h"
#include "meterdsp.h"

/** \brief The Meter class measures the live audio loudness level
 */
class Meter : public ProcessPlugin
{
    Q_OBJECT;

   public:
    /// \brief The class constructor sets the number of channels to measure
    Meter(int numchans, bool verboseFlag = false) : mNumChannels(numchans)
    {
        setVerbose(verboseFlag);
        for (int i = 0; i < mNumChannels; i++) {
            meterP.push_back(new meterdsp);
            // meterUIP.push_back(new APIUI);
            // meterP[i]->buildUserInterface(meterUIP[i]);
        }
    }

    /// \brief The class destructor
    virtual ~Meter()
    {
        for (int i = 0; i < mNumChannels; i++) {
            delete meterP[i];
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

    void init(int samplingRate) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "VU Meter"; };

    void updateNumChannels(int nChansIn, int nChansOut) override;

   private:
    void setupValues();

    float fs;
    int mNumChannels;
    float threshold = -80.0;
    std::vector<meterdsp*> meterP;
    bool hasProcessedAudio = false;

    QTimer mTimer;
    float* mValues    = nullptr;
    float* mOutValues = nullptr;
    float* mBuffer    = nullptr;
    int mBufSize      = 0;

   private slots:
    void onTick();

   signals:
    void onComputedVolumeMeasurements(float* values, int n);
};

#endif
