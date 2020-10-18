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
 * \file AudioTester.cpp
 * \author Julius Smith
 * \date August 2020
 */

#include "AudioTester.h"
#include <assert.h>

// Called 1st in Audiointerface.cpp
void AudioTester::lookForReturnPulse(QVarLengthArray<sample_t*>& out_buffer,
                                     unsigned int n_frames) {
  if (not enabled) {
    std::cerr << "*** AudioTester.h: lookForReturnPulse: NOT ENABLED\n";
    return;
  }
  if (impulsePending) { // look for return impulse in channel sendChannel:
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
            float sampleCountWhenImpulseSent = - 32768.0f * out_buffer[sendChannel][n+1];
            elapsedSamples = sampleCountSinceImpulse + n - int64_t(sampleCountWhenImpulseSent);
            sampleCountSinceImpulse = 1; // reset sample counter between impulses
            roundTripCount += 1.0;
          }
          // int64_t curTimeUS = timeMicroSec(); // time since launch in us
          // int64_t impulseDelayUS = curTimeUS - ImpulseTimeUS;
          // float impulseDelaySec = float(impulseDelayUS) * 1.0e-6;
          // float impulseDelayBuffers = impulseDelaySec / (float(n_frames)/float(sampleRate));
          // int64_t impulseDelayMS = (int64_t)round(double(impulseDelayUS)/1000.0);
          if (elapsedSamples > 0) { // found impulse and reset, time to print buffer results:
            double elapsedSamplesMS = 1000.0 * double(elapsedSamples)/double(sampleRate); // ms
            if (roundTripCount > 1.0) {
              double prevSum = roundTripMean * (roundTripCount-1.0); // undo previous normalization
              roundTripMean = (prevSum + elapsedSamplesMS) / roundTripCount; // add latest and renormalize
              double prevSumSq = roundTripMeanSquare * (roundTripCount-1.0); // undo previous normalization

              roundTripMeanSquare = (prevSumSq + elapsedSamplesMS*elapsedSamplesMS) / roundTripCount;
            } else { // just getting started:
              roundTripMean = elapsedSamplesMS;
              roundTripMeanSquare = elapsedSamplesMS * elapsedSamplesMS;
            }
            if (roundTripCount == 1.0) {
              printf("JackTrip Test Mode (option -x printIntervalInSeconds=%0.3f)\n",printIntervalSec);
              printf("\tA test impulse-train is output on channel %d (from 0) with repeatedly ramping amplitude\n",
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
            double stdDev = sqrt(std::max(0.0, (roundTripMeanSquare - (roundTripMean*roundTripMean))));
            if (timeSinceLastPrintUS >= printIntervalSec * 1.0e6) {
              if (printIntervalSec == 0.0) { printf("%0.1f (", elapsedSamplesMS); }
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
    sampleCountSinceImpulse += n_frames; // gets reset to 1 when impulse is found, counts freely until then
  } // ImpulsePending
}

// Called 2nd in Audiointerface.cpp
void AudioTester::writeImpulse(QVarLengthArray<sample_t*>& mInBufCopy,
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
      impulseTimeSamples = sampleCountSinceImpulse; // timer in samples for current impulse loopback test
      // Also send impulse time:
      if (n_frames>1) { // always true?
        mInBufCopy[sendChannel][1] = -float(impulseTimeSamples)/32768.0f; // survives if there is no digital processing at the server
      } else {
        std::cerr << "\n*** AudioTester.h: Timestamp cannot fit into a lenth " << n_frames << " buffer ***\n";
      }
    } else {
      mInBufCopy[sendChannel][0] = 0.0f; // send zeros until a new impulse is needed
      if (n_frames>1) {
        mInBufCopy[sendChannel][1] = 0.0f;
      }
    }
  } else {
    bufferSkip--;
  }
}
