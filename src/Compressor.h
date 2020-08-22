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
 * \file Compressor.h
 * \author Julius Smith, starting from Limiter.h
 * \date August 2020
 */


/** \brief Applies compressor_mono from the faustlibraries distribution, compressors.lib
 *
 */
#ifndef __COMPRESSOR_H__
#define __COMPRESSOR_H__

#include "ProcessPlugin.h"
#include "compressordsp.h"
#include "CompressorPresets.h"
#include <vector>

/* Settings from http://www.anythingpeaceful.org/sonar/settings/comp.html

   Name     Thresh(dB) Att(ms) Rel(ms) Ratio:1 Gain(dB)    Comments
   Vocal 1     -20     31      342     2.5     2       Compressor for Solo Vocal
   Vocal 2     -8      26      331     2.5     1.5     Variation of Solo 1
   Full Comp 1 -8      60      2500    2.5     0       For Overall Volume Level
   Full Comp 2 -18     94      447     3.5     2.5     Variation of Total Comp 1: Harder ratio
   Full Comp 3 -16     11      180     6       6       Nearly a limiter effect
   Kick Comp   -24     9       58      3       5.5     Compressor for Acoustic Bass Drum
   Snare Comp  -17     8       12      2.5     3.5     Compressor for Acoustic Snare Drum
   Guitar      -10     5       238     2.5     1.5     Compressor for Acoustic Guitar
   Brass Sec   -18     18      226     1.7     4       Brass Sounds for Strong Attacks
   Bass 1      -12     15      470     2       4.5     Finger Picked Bass Guitar
   Bass 2      -12     6       133     1.7     4       Slap Electric Bass
   E Guitar    -8      7       261     3.5     2.5     Electric Guitar Compressor
   Piano 1     -9      17      238     2.5     1       Brightens Piano
   Piano 2     -18     7       174     3.5     6       Variation of Piano 1
   Kick        -14     2       35      2       3.5     For sampled Bass Drum
   Snare       -18     8       354     4       8       For sampled Snare Drum
   Strings 1   -11     33      749     2       1.5     For String instruments
   Strings 2   -12     93      2500    1.5     1.5     For Violas and Cellos
   Strings 3   -17     76      186     1.5     2.5     Cellos or DoubleBass
   Syn Bass    -10     9       250     3.5     3       Adjust level of Synth Bass
   Syn Pad     -13     58      238     2       2       Prevents diffusion of sound in synth pad
   Limiting    -1      0.1     325     20      0       Slow release limiter
   Chorusing   -9      39      225     1.7     2.5     For vocal Chorusing

   From https://www.dummies.com/art-center/music/recording-music/dynamic-music-compression-settings-for-horns-piano-and-percussion/

   Horns       –8      100     300     2.5-3   2       Brasses not normally compressed [jos gain estimate based on above table]
   Piano       -10     100-105 115     1.5-2   2       Normally not compressed ["]
   Kick        -6      40-50   200-300 4-6     3       Looks more like a limiter to me [jos] ["]
   Snare       -4      5-10    125-175 4-6     3       Crucial for a tight, punchy sound
   Bongos      -6      10-25   100-300 3-6     3       "Hand Drums" - protect against excess "slap"
   Perc.       -10     10-20   50      3-6     3       Transient overdrive protection in mix

*/

/** \brief A Compressor reduces the output dynamic range when the
 *         signal level exceeds the threshold.
 */
class Compressor : public ProcessPlugin
{
public:
  /// \brief The class constructor sets the number of channels to limit
  Compressor(int numchans, // xtor
	     bool verboseIn = false,
             float ratioIn = 2.0f, // 2:1 compression above threshold
             float thresholdDBIn = -24.0f,
             float attackMSIn = 15.0f,
             float releaseMSIn = 40.0f,
             float makeUpGainDBIn = 2.0f)
    : mNumChannels(numchans)
    , ratio(ratioIn)
    , thresholdDB(thresholdDBIn)
    , attackMS(attackMSIn)
    , releaseMS(releaseMSIn)
    , makeUpGainDB(makeUpGainDBIn)
  {
    setVerbose(verboseIn);
    // presets.push_back(std::make_unique<CompressorPreset>(ratio,thresholdDB,attackMS,releaseMS,makeUpGainDB));
    for ( int i = 0; i < mNumChannels; i++ ) {
      compressorP.push_back(new compressordsp);
      compressorUIP.push_back(new APIUI); // #included in compressordsp.h
      compressorP[i]->buildUserInterface(compressorUIP[i]);
    }
  }

  /// \brief The class destructor
  virtual ~Compressor() {
    for ( int i = 0; i < mNumChannels; i++ ) {
      delete compressorP[i];
      delete compressorUIP[i];
    }
    compressorP.clear();
    compressorUIP.clear();
  }

  //  void setParamAllChannels(std::string& pName, float p) {
  void setParamAllChannels(const char pName[], float p) {
    for ( int i = 0; i < mNumChannels; i++ ) {
      int ndx = compressorUIP[i]->getParamIndex(pName);
      if (ndx >= 0) {
        compressorUIP[i]->setParamValue(ndx, p);
        if (verbose) {
          std::cout << "Compressor.h: parameter " << pName << " set to " << p << "\n";
        }
      } else {
        std::cerr << "*** Compressor.h: Could not find parameter named " << pName << "\n";
      }
    }
  }

  void init(int samplingRate) override {
    ProcessPlugin::init(samplingRate);
    // std::cout << "Compressor: init(" << samplingRate << ")\n";
    if (samplingRate != fSamplingFreq) {
      std::cerr << "Sampling rate not set by superclass!\n";
      std::exit(1); }
    fs = float(fSamplingFreq);
    for ( int i = 0; i < mNumChannels; i++ ) {
      compressorP[i]->init(fs); // compression filter parameters depend on sampling rate
      // See Limiter.h for how to set compression parameters (same pattern)
    }
    setParamAllChannels("Ratio", ratio);
    setParamAllChannels("Threshold", thresholdDB);
    setParamAllChannels("Attack", attackMS);
    setParamAllChannels("Release", releaseMS);
    setParamAllChannels("MakeUpGain", makeUpGainDB);
    inited = true;
  }

  int getNumInputs() override { return(mNumChannels); }
  int getNumOutputs() override { return(mNumChannels); }
  void compute(int nframes, float** inputs, float** outputs) override;

private:
  float fs;
  int mNumChannels;
  std::vector<compressordsp*> compressorP;
  std::vector<APIUI*> compressorUIP;
  float ratio; // 2:1 compression above threshold
  float thresholdDB;
  float attackMS;
  float releaseMS;
  float makeUpGainDB;
};

#endif
