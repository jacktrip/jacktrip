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
 * \date May 2020
 */


/** \brief Applies limiter_basic_mono from the faustlibraries distribution, compressors.lib
 *
 */
#ifndef __LIMITER_H__
#define __LIMITER_H__

#include "ProcessPlugin.h"
#include "limiterdsp.h"
#include <vector>

/** \brief The Limiter class confines the output dynamic range to a
 *  "dynamic range lane" determined by the assumed number of clients.
 */
class Limiter : public ProcessPlugin
{
public:
  /// \brief The class constructor sets the number of channels to limit
  Limiter(int numchans, int numclients) { // xtor
    mNumChannels = numchans;
    mNumClients = numclients;

    for ( int i = 0; i < mNumClients; i++ ) {
      limiterP.push_back(new limiterdsp);
      limiterUIP.push_back(new APIUI); // #included in limiterdsp.h
      limiterP[i]->buildUserInterface(limiterUIP[i]);
    }

    std::cout << "Limiter: constructed for "
              << mNumChannels << " channels and "
              << mNumClients << " assumed clients\n";
  }

  /// \brief The class destructor
  virtual ~Limiter() {
    for ( int i = 0; i < mNumClients; i++ ) {
      delete limiterP[i];
      delete limiterUIP[i];
    }
    limiterP.clear();
    limiterUIP.clear();
  }

  void init(int samplingRate) override {
    ProcessPlugin::init(samplingRate);
#ifdef DEBUG
    if (samplingRate != fSamplingFreq) {
      std::cerr << "Sampling rate not set by superclass!\n";
      std::exit(1); }
#endif
    fs = float(fSamplingFreq);
    for ( int i = 0; i < mNumClients; i++ ) {
      limiterP[i]->init(fs); // compression filter parameters depend on sampling rate
      int ndx = limiterUIP[i]->getParamIndex("NumClientsAssumed");
      limiterUIP[i]->setParamValue(ndx, mNumClients);
    }
  }
  int getNumInputs() override { return(mNumChannels); }
  int getNumOutputs() override { return(mNumChannels); }
  void compute(int nframes, float** inputs, float** outputs) override;

private:
  float fs;
  int mNumChannels;
  int mNumClients;
  std::vector<limiterdsp*> limiterP;
  std::vector<APIUI*> limiterUIP;
};

#endif
