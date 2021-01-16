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
 * \file AudioTester.h
 * \author Julius Smith
 * \license MIT
 * \date Aug-Oct 2020
 */

#pragma once

#include "jacktrip_types.h" // sample_t

#include <iostream>
//#include <ctime>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <string>
#include <map>

#include <QVarLengthArray>

class AudioTester
{
  bool enabled { false };
  float printIntervalSec { 1.0f };
  int sendChannel { 0 };

  bool impulsePending { false };
  int64_t lastPrintTimeUS { 0 };
  int64_t impulseTimeUS { 0 };
  int64_t impulseTimeSamples { 0 };
  uint64_t sampleCountSinceImpulse { 1 }; // 0 not used
  double roundTripMean { 0.0 };
  double roundTripMeanSquare { 0.0 };
  double roundTripCount { 0.0 };
  const int bufferSkipStart { 100 };
  int bufferSkip { bufferSkipStart };
  const float impulseAmplitude { 0.1f };
  const int numAmpCells { 10 };
  const float ampCellHeight { impulseAmplitude/numAmpCells };

  const double latencyHistogramCellWidth { 5.0 }; // latency range in ms covered one cell
  const double latencyHistogramCellMin { 0.0 };
  const double latencyHistogramCellMax { 19.0 };  // in cells, so 5x this is max latency in ms
  const int latencyHistogramPrintCountMax { 72 }; // normalize when asterisks exceed this number

  int pendingCell { 0 }; // 0 is not used
  float sampleRate { 48000.0f };

public:
  AudioTester() {}
  ~AudioTester() = default;

  void lookForReturnPulse(QVarLengthArray<sample_t*>& out_buffer,
                                       unsigned int n_frames);

  void writeImpulse(QVarLengthArray<sample_t*>& mInBufCopy,
                    unsigned int n_frames);

  bool getEnabled() { return enabled; }
  void setEnabled(bool e) { enabled = e; }
  void setPrintIntervalSec(float s) { printIntervalSec = s; }
  void setSendChannel(int c) { sendChannel = c; }
  int getSendChannel() { return sendChannel; }
  int getPendingCell() { return pendingCell; }
  void setPendingCell(int pc) { pendingCell = pc; }
  void setSampleRate(float fs) { sampleRate = fs; }
  int getBufferSkip() { return bufferSkip; } // used for debugging breakpoints
  void printHelp(char* command, char helpCase);

private:

  float getImpulseAmp() {
    pendingCell += 1; // only called when no impulse is pending
    if (pendingCell >= numAmpCells) {
      pendingCell = 1; // wrap-around, not using zero
    }
    float imp = float(pendingCell) * (impulseAmplitude/float(numAmpCells));
    return imp;
  }

  int getImpulseCellNum(float amp) {
    float ch = ampCellHeight;
    float cell = amp / ch;
    int iCell = int(std::floor(0.5f + cell));
    if (iCell > numAmpCells - 1) {
      std::cerr << "*** AudioTester.h: getImpulseCellNum("<<amp<<"): Return pulse amplitude is beyond maximum expected\n";
      iCell = numAmpCells-1;
    } else if (iCell < 0) {
      std::cerr << "*** AudioTester.h: getImpulseCellNum("<<amp<<"): Return pulse amplitude is below minimum expected\n";
      iCell = 0;
    }
    return iCell;
  }

  uint64_t timeMicroSec() {
#if 1
    using namespace std::chrono;
    // return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
#else
    clock_t tics_since_launch = std::clock();
    double timeUS = double(tics_since_launch)/double(CLOCKS_PER_SEC);
    return (uint64_t)timeUS;
#endif
  }

  std::map<int, int> latencyHistogram;

  std::map<int, int> getLatencyHistogram() {
    return latencyHistogram;
  }

  void extendLatencyHistogram(double latencyMS) {
    int latencyCell = static_cast<int>(floor(std::max(latencyHistogramCellMin,
                                                      std::min(latencyHistogramCellMax,
                                                               std::floor(latencyMS / latencyHistogramCellWidth)))));
    latencyHistogram[latencyCell] += 1;
  }

  int latencyHistogramCountMax() {
    int lhMax = 0;
    int histStart = latencyHistogramFirstNonzeroCellIndex();
    int histLast = latencyHistogramLastNonzeroCellIndex();
    for (int i = histStart; i <= histLast; ++i) {
      int lhi = latencyHistogram[i];
      if (lhi > lhMax) {
        lhMax = lhi;
      }
    }
    return lhMax;
  }

  int latencyHistogramFirstNonzeroCellIndex() {
    for (int i=latencyHistogramCellMin; i <= latencyHistogramCellMax; i++) {
      if (latencyHistogram[i]>0) {
        return i;
      }
    }
    std::cerr << "*** AudioTester: LATENCY HISTOGRAM IS EMPTY!\n";
    return -1;
  }

  int latencyHistogramLastNonzeroCellIndex() {
    for (int i=latencyHistogramCellMax; i>=latencyHistogramCellMin; i--) {
      if (latencyHistogram[i]>0) {
        return i;
      }
    }
    std::cerr << "*** AudioTester: LATENCY HISTOGRAM IS EMPTY!\n";
    return -1;
  }

  std::string getLatencyHistogramString() {
    int histStart = latencyHistogramFirstNonzeroCellIndex();
    int histLast = latencyHistogramLastNonzeroCellIndex();
    std::string marker = "*";
    double histScale = 1.0;
    int lhcm = latencyHistogramCountMax();
    int lhpcm = latencyHistogramPrintCountMax;
    bool normalizing = lhpcm < lhcm;
    if (normalizing) {
      marker = "#";
      histScale = double(lhpcm) / double(lhcm);
    }
    std::string rows = "";
    for (int i = histStart; i <= histLast; ++i) {
      int hi = latencyHistogram[i];
      int hin = int(std::round(histScale * double(hi)));
      std::string istrm1 = std::to_string(int(latencyHistogramCellWidth * double(i)));
      std::string istr = std::to_string(int(latencyHistogramCellWidth * double(i+1)));
      // std::string histr = boost::format("%02d",hi);
      std::string histr = std::to_string(hi);
      while (histr.length()<3) {
        histr = " " + histr;
      }
      std::string row = "["+istrm1+"-"+istr+"ms]="+histr+":";
      for (int j=0; j<hin; j++) {
        row += marker;
      }
      rows += row + "\n";
    }
    if (histLast == latencyHistogramCellMax) {
      rows += " and above\n";
    }
    return rows;
  }

};
