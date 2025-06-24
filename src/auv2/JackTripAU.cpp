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

#include "JackTripAU.h"

#include <AudioToolbox/AudioUnitUtilities.h>
#include <CoreServices/CoreServices.h>

#include <QCoreApplication>
#include <algorithm>
#include <cmath>

using namespace std;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____JackTripAU Processing

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____JackTripAU

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Standard DSP AudioUnit implementation
AUSDK_COMPONENT_ENTRY(ausdk::AUBaseProcessFactory, JackTripAU)

// Parameter names
static CFStringRef kSendGain_Name   = CFSTR("Send Gain");
static CFStringRef kOutputMix_Name  = CFSTR("Output Mix");
static CFStringRef kOutputGain_Name = CFSTR("Output Gain");
static CFStringRef kConnected_Name  = CFSTR("Connected");

// Parameter ranges
const float kMinGain           = 0.0;
const float kMaxGain           = 1.0;
const float kDefaultSendGain   = 1.0;
const float kDefaultOutputMix  = 0.0;
const float kDefaultOutputGain = 1.0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____Construction_Initialization

JackTripAU::JackTripAU(AudioUnit component) : AUEffectBase(component)
{
    // all the parameters must be set to their initial values here
    //
    // these calls have the effect both of defining the parameters for the first time
    // and assigning their initial values
    //
    SetParameter(kJackTripAUParam_SendGain, kDefaultSendGain);
    SetParameter(kJackTripAUParam_OutputMix, kDefaultOutputMix);
    SetParameter(kJackTripAUParam_OutputGain, kDefaultOutputGain);
    SetParameter(kJackTripAUParam_Connected, 0);

    updateVolumeMultipliers(true);
}

JackTripAU::~JackTripAU()
{
    // Cleanup is handled in Cleanup()
}

OSStatus JackTripAU::Initialize()
{
    OSStatus result = AUEffectBase::Initialize();

    if (result == noErr) {
        // Initialize the audio bridge processor
        mProcessor.initialize(GetSampleRate(), GetMaxFramesPerSlice());
    }

    return result;
}

void JackTripAU::Cleanup()
{
    // Uninitialize the audio bridge processor
    mProcessor.uninitialize();

    AUEffectBase::Cleanup();
}

OSStatus JackTripAU::ProcessBufferLists(
    [[maybe_unused]] AudioUnitRenderActionFlags& ioActionFlags,
    const AudioBufferList& inBuffer, AudioBufferList& outBuffer, UInt32 inFramesToProcess)
{
    // Update connection state from processor
    bool connected = GetParameter(kJackTripAUParam_Connected) > 0.5;
    if (connected != mProcessor.isEstablished()) {
        SetParameter(kJackTripAUParam_Connected, mProcessor.isEstablished() ? 1.0 : 0.0);
        // Notify parameter listeners of the change
        AudioUnitParameter changedParam = {GetComponentInstance(),
                                           kJackTripAUParam_Connected,
                                           kAudioUnitScope_Global, 0};
        AUParameterSet(nullptr, nullptr, &changedParam,
                       mProcessor.isEstablished() ? 1.0f : 0.0f, 0);
    }

    // Check if bypass is enabled
    if (ShouldBypassEffect()) {
        // Bypass mode - copy input to output
        for (UInt32 ch = 0; ch < outBuffer.mNumberBuffers; ch++) {
            if (ch < inBuffer.mNumberBuffers) {
                memcpy(outBuffer.mBuffers[ch].mData, inBuffer.mBuffers[ch].mData,
                       inFramesToProcess * sizeof(Float32));
            } else {
                memset(outBuffer.mBuffers[ch].mData, 0,
                       inFramesToProcess * sizeof(Float32));
            }
        }
        return noErr;
    }

    // Update parameters and calculate volume multipliers
    updateVolumeMultipliers(false);

    // Set up input buffers
    bool inputSilenceFlags[AudioSocketNumChannels];
    float* inputBuffers[AudioSocketNumChannels];
    for (UInt32 ch = 0; ch < AudioSocketNumChannels; ch++) {
        if (ch < inBuffer.mNumberBuffers) {
            inputBuffers[ch] = static_cast<float*>(inBuffer.mBuffers[ch].mData);
        } else {
            inputBuffers[ch] = nullptr;
        }
        inputSilenceFlags[ch] = false;
    }

    // Set up output buffers
    bool outputSilenceFlags[AudioSocketNumChannels];
    float* outputBuffers[AudioSocketNumChannels];
    for (UInt32 ch = 0; ch < AudioSocketNumChannels; ch++) {
        if (ch < outBuffer.mNumberBuffers) {
            outputBuffers[ch] = static_cast<float*>(outBuffer.mBuffers[ch].mData);
        } else {
            outputBuffers[ch] = nullptr;
        }
    }

    // Process through the audio bridge processor
    mProcessor.process(inputBuffers, outputBuffers, inputSilenceFlags, outputSilenceFlags,
                       inFramesToProcess);

    // Handle any remaining output channels by zeroing them
    for (UInt32 ch = AudioSocketNumChannels; ch < outBuffer.mNumberBuffers; ch++) {
        memset(outBuffer.mBuffers[ch].mData, 0, outBuffer.mBuffers[ch].mDataByteSize);
    }

    return noErr;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark ____Parameters

OSStatus JackTripAU::GetParameterInfo(AudioUnitScope inScope,
                                      AudioUnitParameterID inParameterID,
                                      AudioUnitParameterInfo& outParameterInfo)
{
    OSStatus result = noErr;

    outParameterInfo.flags =
        kAudioUnitParameterFlag_IsWritable + kAudioUnitParameterFlag_IsReadable;

    if (inScope == kAudioUnitScope_Global) {
        switch (inParameterID) {
        case kJackTripAUParam_SendGain:
            AUBase::FillInParameterName(outParameterInfo, kSendGain_Name, false);
            outParameterInfo.unit         = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue     = kMinGain;
            outParameterInfo.maxValue     = kMaxGain;
            outParameterInfo.defaultValue = kDefaultSendGain;
            outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            break;

        case kJackTripAUParam_OutputMix:
            AUBase::FillInParameterName(outParameterInfo, kOutputMix_Name, false);
            outParameterInfo.unit         = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue     = kMinGain;
            outParameterInfo.maxValue     = kMaxGain;
            outParameterInfo.defaultValue = kDefaultOutputMix;
            outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            break;

        case kJackTripAUParam_OutputGain:
            AUBase::FillInParameterName(outParameterInfo, kOutputGain_Name, false);
            outParameterInfo.unit         = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue     = kMinGain;
            outParameterInfo.maxValue     = kMaxGain;
            outParameterInfo.defaultValue = kDefaultOutputGain;
            outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
            break;

        case kJackTripAUParam_Connected:
            AUBase::FillInParameterName(outParameterInfo, kConnected_Name, false);
            outParameterInfo.unit         = kAudioUnitParameterUnit_Boolean;
            outParameterInfo.minValue     = 0;
            outParameterInfo.maxValue     = 1;
            outParameterInfo.defaultValue = 0;
            outParameterInfo.flags += kAudioUnitParameterFlag_IsReadable;
            outParameterInfo.flags -= kAudioUnitParameterFlag_IsWritable;
            break;

        default:
            result = kAudioUnitErr_InvalidParameter;
            break;
        }
    } else {
        result = kAudioUnitErr_InvalidParameter;
    }

    return result;
}

OSStatus JackTripAU::GetPropertyInfo(AudioUnitPropertyID inID, AudioUnitScope inScope,
                                     AudioUnitElement inElement, UInt32& outDataSize,
                                     bool& outWritable)
{
    if (inScope == kAudioUnitScope_Global) {
        switch (inID) {
        case kAudioUnitProperty_SupportedNumChannels:
            outDataSize =
                sizeof(AUChannelInfo) * 1;  // We support exactly one configuration
            outWritable = false;
            return noErr;
        case kAudioUnitProperty_CocoaUI:
            outDataSize = sizeof(AudioUnitCocoaViewInfo);
            outWritable = false;
            return noErr;
        }
    }

    return AUEffectBase::GetPropertyInfo(inID, inScope, inElement, outDataSize,
                                         outWritable);
}

OSStatus JackTripAU::GetProperty(AudioUnitPropertyID inID, AudioUnitScope inScope,
                                 AudioUnitElement inElement, void* outData)
{
    if (inScope == kAudioUnitScope_Global) {
        switch (inID) {
        case kAudioUnitProperty_SupportedNumChannels: {
            // We support exactly 2 input channels and 2 output channels
            AUChannelInfo* channelInfo = static_cast<AUChannelInfo*>(outData);
            channelInfo[0].inChannels  = 2;
            channelInfo[0].outChannels = 2;
            return noErr;
        }
        case kAudioUnitProperty_CocoaUI: {
            // Return information about our custom Cocoa UI
            AudioUnitCocoaViewInfo* viewInfo =
                static_cast<AudioUnitCocoaViewInfo*>(outData);
            return JackTrip_GetCocoaUI(viewInfo);
        }
        }
    }

    return AUEffectBase::GetProperty(inID, inScope, inElement, outData);
}

void JackTripAU::updateVolumeMultipliers(bool force)
{
    // Check parameter values
    float sendGain   = GetParameter(kJackTripAUParam_SendGain);
    float outputMix  = GetParameter(kJackTripAUParam_OutputMix);
    float outputGain = GetParameter(kJackTripAUParam_OutputGain);

    // Since this gets called on every process block, we only update if the parameters
    // have changed
    if (!force && sendGain == mSendGain && outputMix == mOutputMix
        && outputGain == mOutputGain) {
        return;
    }

    // Calculate volume multipliers and update processor
    float outMul  = AudioBridgeProcessor::gainToVol(outputGain);
    float sendMul = AudioBridgeProcessor::gainToVol(sendGain);
    float recvMul = outputMix * outMul;
    float passMul = (1.0f - outputMix) * outMul;

    // Update processor parameters
    mProcessor.setSendMul(sendMul);
    mProcessor.setRecvMul(recvMul);
    mProcessor.setPassMul(passMul);
}
