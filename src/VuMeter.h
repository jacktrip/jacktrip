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
 * \file VuMeter.h
 * \author Dominick Hing
 * \date August 2022
 * \license MIT
 */

#ifndef __VUMETER_H__
#define __VUMETER_H__

#include <QObject>
#include <QTimer>
#include <QVector>
#include <iostream>
#include <vector>

#include "ProcessPlugin.h"
#include "vumeterdsp.h"

/** \brief The VuMeter class measures the live audio loudness level
 */
class VuMeter : public ProcessPlugin
{
    Q_OBJECT;

   public:
    /// \brief The class constructor sets the number of channels to measure
    VuMeter(int numchans, bool verboseFlag = false) : mNumChannels(numchans)
    {
        setVerbose(verboseFlag);
        for (int i = 0; i < mNumChannels; i++) {
            vumeterP.push_back(new vumeterdsp);
            // vumeterUIP.push_back(new APIUI);
            // vumeterP[i]->buildUserInterface(vumeterUIP[i]);
        }
    }

    /// \brief The class destructor
    virtual ~VuMeter() {
        for (int i = 0; i < mNumChannels; i++) {
            delete vumeterP[i];
        }
        vumeterP.clear();
    }

    void init(int samplingRate) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "VU Meter"; };

   private:
    float fs;
    int mNumChannels;
    std::vector<vumeterdsp*> vumeterP;
    std::vector<APIUI*> vumeterUIP;

    QTimer mTimer;
    QVector<float> mValues;

   private slots:
    void onTick();

   signals:
    void onComputedVolumeMeasurements(QVector<float> values);
};

#endif