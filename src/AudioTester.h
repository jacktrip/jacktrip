#pragma once

#include "jacktrip_types.h" // sample_t

#include <iostream>
//#include <ctime>
#include <chrono>
#include <cstdint>
#include <cmath>

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
  uint64_t sampleCount { 1 }; // 0 not used
  double roundTripMean { 0.0 };
  double roundTripMeanSquare { 0.0 };
  double roundTripCount { 0.0 };
  const int bufferSkipStart { 100 };
  int bufferSkip { bufferSkipStart };
  static constexpr float impulseAmplitude { 0.1f };
  static constexpr int numAmpCells { 10 };
  static constexpr float ampCellHeight { impulseAmplitude/numAmpCells };
    
  int pendingCell { 0 }; // 0 is not used
  float sampleRate { 48000.0f };

public:
  AudioTester() {}
  ~AudioTester() = default;

  void lookForReturnPulse(QVarLengthArray<sample_t*>& out_buffer,
				       unsigned int n_frames);

  void writeImpulse(QVarLengthArray<sample_t*>& mInBufCopy,
		    QVarLengthArray<sample_t*>& in_buffer,
		    unsigned int n_frames);

  bool getEnabled() { return enabled; }
  void setEnabled(bool e) { enabled = e; }
  void setPrintIntervalSec(float s) { printIntervalSec = s; }
  void setSendChannel(int c) { sendChannel = c; }
  int getPendingCell() { return pendingCell; }
  void setPendingCell(int pc) { pendingCell = pc; }
  void setSampleRate(float fs) { sampleRate = fs; }
  int getBufferSkip() { return bufferSkip; } // used for debugging breakpoints

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

};
