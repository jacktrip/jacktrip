//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024-2025 JackTrip Labs, Inc.

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

// based on Filter sample code from https://github.com/apple/AudioUnit-Examples

#include <AudioToolbox/AudioUnitProperties.h>
#include <AudioUnitSDK/AUEffectBase.h>

#include <memory>

#include "../AudioBridgeProcessor.h"

// Forward declaration for Cocoa UI support
// AudioUnitCocoaViewInfo is defined in AudioUnitProperties.h
extern "C" OSStatus JackTrip_GetCocoaUI(AudioUnitCocoaViewInfo* viewInfo);

#ifndef __JackTripAU_h__
#define __JackTripAU_h__

enum JackTripAUParams {
    kJackTripAUParam_SendGain   = 0,
    kJackTripAUParam_OutputMix  = 1,
    kJackTripAUParam_OutputGain = 2,
    kJackTripAUParam_Connected  = 3,
};

class JackTripAU : public ausdk::AUEffectBase
{
   public:
    JackTripAU(AudioUnit component);
    ~JackTripAU();

    OSStatus Initialize() override;
    void Cleanup() override;

    OSStatus ProcessBufferLists(AudioUnitRenderActionFlags& ioActionFlags,
                                const AudioBufferList& inBuffer,
                                AudioBufferList& outBuffer,
                                UInt32 inFramesToProcess) override;

    OSStatus GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID,
                              AudioUnitParameterInfo& outParameterInfo) override;

    OSStatus GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
                             AudioUnitElement inElement, UInt32& outDataSize,
                             bool& outWritable) override;

    OSStatus GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
                         AudioUnitElement inElement, void* outData) override;

    Float64 GetLatency() override { return 0.0; }

   private:
    // Updates volume multipliers in processor
    void updateVolumeMultipliers(bool force);

    // Audio bridge processor
    AudioBridgeProcessor mProcessor;

    // Cache values of current parameters
    float mSendGain   = 1.f;
    float mOutputMix  = 0;
    float mOutputGain = 1.f;
};

#endif