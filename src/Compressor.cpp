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
 * \file Compressor.cpp
 * \author Julius Smith, based on LoopBack.h
 * \date July 2008
 */

#include "Compressor.h"

#include "compressordsp.h"

//*******************************************************************************
Compressor::Compressor(int numchans,  // xtor
                       bool verboseIn, float ratioIn, float thresholdDBIn,
                       float attackMSIn, float releaseMSIn, float makeUpGainDBIn)
    : mNumChannels(numchans)
    , ratio(ratioIn)
    , thresholdDB(thresholdDBIn)
    , attackMS(attackMSIn)
    , releaseMS(releaseMSIn)
    , makeUpGainDB(makeUpGainDBIn)
{
    setVerbose(verboseIn);
    // presets.push_back(std::make_unique<CompressorPreset>(ratio,thresholdDB,attackMS,releaseMS,makeUpGainDB));
    for (int i = 0; i < mNumChannels; i++) {
        compressordsp* dsp_ptr = new compressordsp;
        APIUI* ui_ptr          = new APIUI;
        compressorP.push_back(dsp_ptr);
        compressorUIP.push_back(ui_ptr);  // #included in compressordsp.h
        dsp_ptr->buildUserInterface(ui_ptr);
    }
}

//*******************************************************************************
Compressor::~Compressor()
{
    for (int i = 0; i < mNumChannels; i++) {
        delete static_cast<compressordsp*>(compressorP[i]);
        delete static_cast<APIUI*>(compressorUIP[i]);
    }
    compressorP.clear();
    compressorUIP.clear();
}

//*******************************************************************************
void Compressor::setParamAllChannels(const char pName[], float p)
{
    for (int i = 0; i < mNumChannels; i++) {
        APIUI* ui_ptr = static_cast<APIUI*>(compressorUIP[i]);
        int ndx       = ui_ptr->getParamIndex(pName);
        if (ndx >= 0) {
            ui_ptr->setParamValue(ndx, p);
            if (verbose) {
                std::cout << "Compressor.h: parameter " << pName << " set to " << p
                          << " on audio channel " << i << "\n";
            }
        } else {
            std::cerr << "*** Compressor.h: Could not find parameter named " << pName
                      << "\n";
        }
    }
}

//*******************************************************************************
void Compressor::init(int samplingRate, int bufferSize)
{
    ProcessPlugin::init(samplingRate, bufferSize);
    fs = float(fSamplingFreq);
    for (int i = 0; i < mNumChannels; i++) {
        static_cast<compressordsp*>(compressorP[i])
            ->init(fs);  // compression filter parameters depend on sampling rate
    }
    setParamAllChannels("Ratio", ratio);
    setParamAllChannels("Threshold", thresholdDB);
    setParamAllChannels("Attack", attackMS);
    setParamAllChannels("Release", releaseMS);
    setParamAllChannels("MakeUpGain", makeUpGainDB);
    inited = true;
}

//*******************************************************************************
void Compressor::compute(int nframes, float** inputs, float** outputs)
{
    if (!inited) {
        std::cerr << "*** Compressor " << this << ": init never called! Doing it now.\n";
        init(0, 0);
    }
    for (int i = 0; i < mNumChannels; i++) {
        static_cast<compressordsp*>(compressorP[i])
            ->compute(nframes, &inputs[i], &outputs[i]);
    }
}
