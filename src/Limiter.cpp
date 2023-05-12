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
 * \author Julius Smith, based on LoopBack.h
 * \date May-Nov 2020
 * \license MIT
 */

#include "Limiter.h"

#include "jacktrip_types.h"
#include "limiterdsp.h"

//*******************************************************************************
Limiter::Limiter(int numchans, int numclients, bool verboseFlag)
    : mNumChannels(numchans)
    , mNumClients(numclients)
    , warningAmp(0.0)
    , warnCount(0)
    , peakMagnitude(0.0)
    , nextWarning(1)
{
    setVerbose(verboseFlag);
    for (int i = 0; i < mNumChannels; i++) {
        limiterdsp* dsp_ptr = new limiterdsp;
        APIUI* ui_ptr       = new APIUI;
        limiterP.push_back(dsp_ptr);
        limiterUIP.push_back(ui_ptr);  // #included in limiterdsp.h
        dsp_ptr->buildUserInterface(ui_ptr);
#ifdef SINE_TEST
        limitertest* test_ptr = new limitertest;
        ui_ptr                = new APIUI;
        limiterTestP.push_back(test_ptr);
        limiterTestUIP.push_back(ui_ptr);  // #included in limitertest.h
        test_ptr->buildUserInterface(ui_ptr);
#endif
    }
    //    std::cout << "Limiter: constructed for "
    // << mNumChannels << " channels and "
    // << mNumClients << " assumed clients\n";
}

//*******************************************************************************
Limiter::~Limiter()
{
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<limiterdsp*>(limiterP[i]);
        delete static_cast<APIUI*>(limiterUIP[i]);
    }
    limiterP.clear();
    limiterUIP.clear();
}

//*******************************************************************************
void Limiter::init(int samplingRate)
{
    ProcessPlugin::init(samplingRate);
    if (samplingRate != fSamplingFreq) {
        std::cerr << "Sampling rate not set by superclass!\n";
        std::exit(1);
    }
    fs = float(fSamplingFreq);
    for (int i = 0; i < mNumChannels; i++) {
        static_cast<limiterdsp*>(limiterP[i])
            ->init(fs);  // compression filter parameters depend on sampling rate
        APIUI* ui_ptr = static_cast<APIUI*>(limiterUIP[i]);
        int ndx       = ui_ptr->getParamIndex("NumClientsAssumed");
        ui_ptr->setParamValue(ndx, mNumClients);
#ifdef SINE_TEST
        static_cast<limitertest*>(limiterTestP[i])
            ->init(fs);  // oscillator parameters depend on sampling rate
        ui_ptr = static_cast<APIUI*>(limiterTestUIP[i]);
        ndx    = ui_ptr->getParamIndex("Amp");
        ui_ptr->setParamValue(ndx, 0.2);
        ndx = ui_ptr->getParamIndex("Freq");
        float sineFreq =
            110.0 * pow(1.5, double(i))
            * (mNumClients > 1 ? 1.25 : 1.0);  // Maj 7 chord for stereo in & out
        ui_ptr->setParamValue(ndx, sineFreq);
#endif
    }
    inited = true;
}

//*******************************************************************************
void Limiter::compute(int nframes, float** inputs, float** outputs)
{
    if (not inited) {
        std::cerr << "*** Limiter " << this << ": init never called! Doing it now.\n";
        if (fSamplingFreq <= 0) {
            fSamplingFreq = 48000;
            std::cout << "Limiter " << this
                      << ": *** HAD TO GUESS the sampling rate (chose 48000 Hz) ***\n";
        }
        init(fSamplingFreq);
    }
#ifdef SINE_TEST
    float sineTestOut[nframes];
    float* faustSigs[1]{sineTestOut};
#endif
    for (int i = 0; i < mNumChannels; i++) {
        if (warningAmp > 0.0) {
            checkAmplitudes(nframes,
                            inputs[i]);  // we presently do one check across all channels
        }
        static_cast<limiterdsp*>(limiterP[i])->compute(nframes, &inputs[i], &outputs[i]);
#ifdef SINE_TEST
        static_cast<limitertest*>(limiterTestP[i])
            ->compute(nframes, faustSigs, faustSigs);
        for (int n = 0; n < nframes; n++) {
            outputs[i][n] = outputs[i][n] + sineTestOut[n];
        }
#endif
    }
}
