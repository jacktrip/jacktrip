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
 * \file Limiter.h
 * \author Julius Smith, based on LoopBack.h
 * \date May-Nov 2020
 * \license MIT
 */

/** \brief Applies limiter_lad_mono from the faustlibraries distribution, compressors.lib
 *
 */
#ifndef __LIMITER_H__
#define __LIMITER_H__

//#define SINE_TEST

#ifdef SINE_TEST
#include "limitertest.h"
#endif

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

#include "ProcessPlugin.h"

/** \brief The Limiter class confines the output dynamic range to a
 *  "dynamic range lane" determined by the assumed number of clients.
 */
class Limiter : public ProcessPlugin
{
   public:
    /// \brief The class constructor sets the number of channels to limit
    Limiter(int numchans, int numclients, bool verboseFlag = false);

    /// \brief The class destructor
    virtual ~Limiter();

    void init(int samplingRate) override;
    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "Limiter"; }

    void setWarningAmplitude(double wa)
    {  // setting to 0 turns off warnings
        warningAmp = std::max(0.0, std::min(1.0, wa));
    }

   private:
    void checkAmplitudes(int nframes, float* buf)
    {
        const int maxWarningInterval{10000};  // this could become an option
        assert(warningAmp > 0.0);
        assert(mNumClients > 0);
        for (int i = 0; i < nframes; i++) {
            double tmp_sample = double(buf[i]);
            double limiterAmp =
                fabs(tmp_sample)
                / sqrt(double(mNumClients));  // KEEP IN SYNC with gain in
                                              // ../faust-src/limiterdsp.dsp
            if (limiterAmp >= warningAmp) {
                warnCount++;
                peakMagnitude = std::max(peakMagnitude, limiterAmp);
                if (warnCount == nextWarning) {
                    double peakMagnitudeDB = 20.0 * std::log10(peakMagnitude);
                    double warningAmpDB    = 20.0 * std::log10(warningAmp);
                    if (warnCount == 1) {
                        if (warningAmp == 1.0) {
                            std::cerr << "*** Limiter.cpp: Audio HARD-CLIPPED!\n";
                            fprintf(stderr,
                                    "\tReduce your audio input level(s) by %0.1f dB to "
                                    "avoid this.\n",
                                    peakMagnitudeDB);
                        } else {
                            fprintf(stderr,
                                    "*** Limiter.cpp: Amplitude levels must stay below "
                                    "%0.1f dBFS to avoid compression.\n",
                                    warningAmpDB);
                            fprintf(
                                stderr,
                                "\tReduce input level(s) by %0.1f dB to achieve this.\n",
                                peakMagnitudeDB - warningAmpDB);
                        }
                    } else {
                        fprintf(stderr,
                                "\tReduce audio input level(s) by %0.1f dB to avoid "
                                "limiter compression distortion.\n",
                                peakMagnitudeDB - warningAmpDB);
                    }
                    peakMagnitude = 0.0;  // reset for next group measurement
                    if (nextWarning < maxWarningInterval) {  // don't let it stop
                                                             // reporting for too long
                        nextWarning *= 10;
                    } else {
                        warnCount = 0;
                    }
                }  // warnCount==nextWarning
            }      // above warningAmp
        }          // loop over frames
    }              // checkAmplitudes()

   private:
    float fs;
    int mNumChannels;
    int mNumClients;
    std::vector<void*> limiterP;
    std::vector<void*> limiterUIP;
#ifdef SINE_TEST
    std::vector<void*> limiterTestP;
    std::vector<void*> limiterTestUIP;
#endif
    double warningAmp;
    uint32_t warnCount;
    double peakMagnitude;
    uint32_t nextWarning;
};

#endif
