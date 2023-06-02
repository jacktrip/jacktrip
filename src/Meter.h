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
#include <vector>

#include "ProcessPlugin.h"

/** \brief The Meter class measures the live audio loudness level
 */
class Meter : public ProcessPlugin
{
    Q_OBJECT;

   public:
    /// \brief The class constructor sets the number of channels to measure
    Meter(int numchans, bool verboseFlag = false);

    /// \brief The class destructor
    virtual ~Meter();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "VU Meter"; };

    void updateNumChannels(int nChansIn, int nChansOut) override;

    void setIsMonitoringMeter(bool isMonitoringMeter)
    {
        mIsMonitoringMeter = isMonitoringMeter;
    };
    bool getIsMonitoringMeter() { return mIsMonitoringMeter; };

   private:
    void setupValues();

    float fs;
    bool mIsMonitoringMeter = false;

    int mNumChannels;
    float threshold = -80.0;
    std::vector<void*> meterP;
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
