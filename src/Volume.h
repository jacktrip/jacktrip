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
 * \file Volume.h
 * \author Matt Horton
 * \date September 2022
 * \license MIT
 */

#ifndef __VOLUME_H__
#define __VOLUME_H__

#include <QObject>
#include <vector>

#include "ProcessPlugin.h"

/** \brief The Volume plugin adjusts the level of the signal via multiplication
 */
class Volume : public ProcessPlugin
{
    Q_OBJECT;

   public:
    /// \brief The class constructor sets the number of channels to measure
    Volume(int numchans, bool verboseFlag = false);

    /// \brief The class destructor
    virtual ~Volume();

    void init(int samplingRate, int bufferSize) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "Volume"; };

    void updateNumChannels(int nChansIn, int nChansOut) override;

   public slots:
    void volumeUpdated(float multiplier);
    void muteUpdated(bool muted);

   private:
    std::vector<void*> volumeP;
    std::vector<void*> volumeUIP;
    float fs;
    int mNumChannels;
    float mVolMultiplier = 0.0;
    bool isMuted         = false;
};

#endif