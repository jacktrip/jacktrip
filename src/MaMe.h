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
 * \file MaMe.h
 * \author Julius Smith, starting from Limiter.h
 * \date August 2020
 */

/** \brief Applies compressor_mono from the faustlibraries distribution, compressors.lib
 *
 */
#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include <iostream>
#include <vector>

#include "MaMePresets.h"
#include "ProcessPlugin.h"
#include "master_me_jacktripdsp.h"

/** \brief A Compressor reduces the output dynamic range when the
 *         signal level exceeds the threshold.
 */
class MaMe : public ProcessPlugin
{
   public:
    /// \brief The class constructor sets the number of audio channels and default
    /// parameters.
    MaMe(int numchans,  // xtor
               bool verboseIn = false, float ratioIn = 2.0f, float thresholdDBIn = -24.0f,
               float attackMSIn = 15.0f, float releaseMSIn = 40.0f,
               float makeUpGainDBIn = 2.0f)
        : mNumChannels(numchans)
        , ratio(ratioIn)
        , thresholdDB(thresholdDBIn)
        , attackMS(attackMSIn)
        , releaseMS(releaseMSIn)
        , makeUpGainDB(makeUpGainDBIn)
    {
        setVerbose(verboseIn);
        // presets.push_back(std::make_unique<MaMePreset>(ratio,thresholdDB,attackMS,releaseMS,makeUpGainDB));
        for (int i = 0; i < mNumChannels; i++) {
            mameP.push_back(new master_me_jacktripdsp);
            mameUIP.push_back(new APIUI);  // #included in mamedsp.h
            mameP[i]->buildUserInterface(mameUIP[i]);
        }
    }

    MaMe(int numchans,  // xtor
               bool verboseIn = false, MaMePreset preset = MaMePresets::voice)
        : MaMe(numchans, verboseIn, preset.ratio, preset.thresholdDB,
                     preset.attackMS, preset.releaseMS, preset.makeUpGainDB)
    {
    }
    /// \brief The class destructor
    virtual ~MaMe()
    {
        for (int i = 0; i < mNumChannels; i++) {
            delete mameP[i];
            delete mameUIP[i];
        }
        mameP.clear();
        mameUIP.clear();
    }

    //  void setParamAllChannels(std::string& pName, float p) {
    void setParamAllChannels(const char pName[], float p)
    {
        for (int i = 0; i < mNumChannels; i++) {
            int ndx = mameUIP[i]->getParamIndex(pName);
            if (ndx >= 0) {
                mameUIP[i]->setParamValue(ndx, p);
                if (verbose) {
                    std::cout << "MaMe.h: parameter " << pName << " set to " << p
                              << " on audio channel " << i << "\n";
                }
            } else {
                std::cerr << "*** MaMe.h: Could not find parameter named " << pName
                          << "\n";
            }
        }
    }

    void init(int samplingRate) override
    {
        ProcessPlugin::init(samplingRate);
        if (samplingRate != fSamplingFreq) {
            std::cerr << "Sampling rate not set by superclass!\n";
            std::exit(1);
        }
        fs = float(fSamplingFreq);
        for (int i = 0; i < mNumChannels; i++) {
            mameP[i]->init(
                fs);  // compression filter parameters depend on sampling rate
        }
        setParamAllChannels("Ratio", ratio);
        setParamAllChannels("Threshold", thresholdDB);
        setParamAllChannels("Attack", attackMS);
        setParamAllChannels("Release", releaseMS);
        setParamAllChannels("MakeUpGain", makeUpGainDB);
        inited = true;
    }

    int getNumInputs() override { return (mNumChannels); }
    int getNumOutputs() override { return (mNumChannels); }
    void compute(int nframes, float** inputs, float** outputs) override;
    const char* getName() const override { return "MaMe"; }

   private:
    float fs;
    int mNumChannels;
    std::vector<master_me_jacktripdsp*> mameP;
    std::vector<APIUI*> mameUIP;
    float ratio;
    float thresholdDB;
    float attackMS;
    float releaseMS;
    float makeUpGainDB;
};

#endif
