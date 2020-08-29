#pragma once

#include <iostream>
//#include <ctime>
#include <chrono>
#include <cstdint>

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

  bool getEnabled() { return enabled; }
  void setEnabled(bool e) { enabled = e; }
  void setPrintIntervalSec(float s) { printIntervalSec = s; }
  void setSendChannel(int c) { sendChannel = c; }
  int getPendingCell() { return pendingCell; }
  void setPendingCell(int pc) { pendingCell = pc; }
  void setSampleRate(float fs) { sampleRate = fs; }
  int getBufferSkip() { return bufferSkip; } // used for debugging breakpoints

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
    int iCell = round(cell);
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

  void lookForReturnPulse(QVarLengthArray<sample_t*>& out_buffer,
                          unsigned int n_frames) {
    if (not enabled) {
      std::cerr << "*** AudioTester.h: lookForReturnPulse: NOT ENABLED\n";
      return;
    }
    if (impulsePending) { // look for return impulse in channel 0:
      assert(sendChannel<out_buffer.size());
      for (uint n=0; n<n_frames; n++) {
        float amp = out_buffer[sendChannel][n];
        if (amp > 0.5 * ampCellHeight) { // got something
          int cellNum =  getImpulseCellNum(out_buffer[sendChannel][n]);
          if (cellNum != pendingCell) { // not our impulse!
            std::cerr <<
              "*** AudioTester.h: computeProcessFromNetwork: Received pulse amplitude "
                      << amp << " (cell " << cellNum << ") while looking for cell "
                      << pendingCell << " - ABORTING CURRENT PULSE\n";
            impulsePending = false;
          } else { // found our impulse:
            int64_t elapsedSamples = -1;
            if (n >= n_frames-1) {
              // Impulse timestamp didn't make it so we skip this one.
            } else {
              float currentTestSampleCount = - 32768.0f * out_buffer[sendChannel][n+1];
              elapsedSamples = sampleCount + int64_t(currentTestSampleCount);
              sampleCount = 1; // reset sample counter between impulses
              roundTripCount += 1.0;
            }
            // int64_t curTimeUS = timeMicroSec(); // time since launch in us
            // int64_t impulseDelayUS = curTimeUS - ImpulseTimeUS;
            // float impulseDelaySec = float(impulseDelayUS) * 1.0e-6;
            // float impulseDelayBuffers = impulseDelaySec / (float(n_frames)/float(sampleRate));
            // int64_t impulseDelayMS = (int64_t)round(double(impulseDelayUS)/1000.0);
            if (elapsedSamples > 0) {
              int64_t elapsedSamplesMS = (int64_t)round(1000.0 * double(elapsedSamples)/double(sampleRate)); // ms
              if (roundTripCount > 1.0) {
                double prevSum = roundTripMean * (roundTripCount-1.0); // undo previous normalization
                roundTripMean = (prevSum + elapsedSamplesMS) / roundTripCount; // add latest and renormalize
                double prevSumSq = roundTripMeanSquare * (roundTripCount-1.0); // undo previous normalization

                roundTripMeanSquare = (prevSumSq + elapsedSamplesMS*elapsedSamplesMS) / roundTripCount;
              } else {
                roundTripMean = elapsedSamplesMS;
                roundTripMeanSquare = elapsedSamplesMS * elapsedSamplesMS;
              }
              if (roundTripCount == 1.0) {
                printf("JackTrip Test Mode (option -x printIntervalInSeconds=%0.3f)\n",printIntervalSec);
                printf("\tYou will hear a test impulse-train in channel %d (from 0) with ramping amplitude\n",
                       sendChannel);
                if (printIntervalSec == 0.0) {
                  printf("\tPrinting each audio buffer round-trip latency in ms followed by cumulative (mean and [standard deviation])");
                } else {
                  printf("\tPrinting cumulative mean and [standard deviation] of audio round-trip latency in ms");
                  printf(" every %0.3f seconds", printIntervalSec);
                }
                printf(" after skipping first %d buffers:\n", bufferSkipStart);
                // not printing this presently: printf("( * means buffer skipped due missing timestamp or lost impulse)\n");
                lastPrintTimeUS = timeMicroSec();
              }
              //printf("%d (%d) ", elapsedSamplesMS, impulseDelayMS); // measured time is "buffer time" not sample time
              int64_t curTimeUS = timeMicroSec(); // time since launch in us
              double timeSinceLastPrintUS = double(curTimeUS - lastPrintTimeUS);
              float stdDev = sqrt(roundTripMeanSquare - (roundTripMean*roundTripMean));
              if (timeSinceLastPrintUS >= printIntervalSec * 1.0e6) {
                if (printIntervalSec == 0.0) { printf("%lld (", elapsedSamplesMS); }
                printf("%0.1f [%0.1f]", roundTripMean, stdDev);
                if (printIntervalSec == 0.0) { printf(") "); } else { printf(" "); }
                lastPrintTimeUS = curTimeUS;
              }
              std::cout << std::flush;
            } else {
              // not printing this presently: printf("* "); // we got the impulse but lost its timestamp in samples
            }
            impulsePending = false;
          } // found our impulse
          // remain pending until timeout, hoping to find our return pulse
        } // got something
      } // loop over samples
    } // ImpulsePending
  }  

  void writeImpulse(QVarLengthArray<sample_t*>& mInBufCopy,
		    QVarLengthArray<sample_t*>& in_buffer,
                    unsigned int n_frames) {
    if (not enabled) {
      std::cerr << "*** AudioTester.h: writeImpulse: NOT ENABLED\n";
      return;
    }
    if (bufferSkip <= 0) { // send test signals (-x option)
      bool sendImpulse;
      if (impulsePending) {
        sendImpulse = false; // unless:
        const uint64_t timeOut = 500e3; // time out after waiting 500 ms
        if (timeMicroSec() > (impulseTimeUS + timeOut)) {
          sendImpulse = true;
          std::cout << "\n*** TEST MODE (-x): TIMED OUT waiting for return impulse *** sending a new one\n";
        }
      } else { // time for the next repeating impulse:
        sendImpulse = true;
      }
      if (sendImpulse) {
        assert(sendChannel < in_buffer.size());
	assert(sendChannel < mInBufCopy.size());
        mInBufCopy[sendChannel][0] = getImpulseAmp();
        impulsePending = true;
        impulseTimeUS = timeMicroSec();
        impulseTimeSamples = sampleCount; // timer in samples for current impulse loopback test
        // also send impulse time:
      } else {
        mInBufCopy[sendChannel][0] = 0.0f; // send zeros until a new impulse is needed
      }
      // In either case, sent current sample-count:
      if (n_frames>1) { // always true?
        mInBufCopy[sendChannel][1] = -float(sampleCount)/32768.0f; // survives if there is no digital processing at the server
      } else {
        std::cerr << "\n*** AudioTester.h: Timestamp cannot fit into a lenth " << n_frames << " buffer ***\n";
      }
      sampleCount += n_frames; // reset to 1 when sent impulse is found
    } else {
      bufferSkip--;
    }
  }
};
