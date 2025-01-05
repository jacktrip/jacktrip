//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2024 JackTrip Labs, Inc.

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

// Based on the Hello World VST 3 example from Steinberg
// https://github.com/steinbergmedia/vst3_example_plugin_hello_world

#include "JackTripVSTProcessor.h"

#include "../AudioSocket.h"
#include "JackTripVST.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace Steinberg
{

static QCoreApplication* sQtAppPtr = nullptr;

static QCoreApplication* getQtAppPtr()
{
    if (sQtAppPtr == nullptr) {
        sQtAppPtr = QCoreApplication::instance();
        if (sQtAppPtr == nullptr) {
            int argc  = 0;
            sQtAppPtr = new QCoreApplication(argc, nullptr);
            sQtAppPtr->setAttribute(Qt::AA_NativeWindows);
        }
    }
    return sQtAppPtr;
}

//------------------------------------------------------------------------
// JackTripVSTProcessor
//------------------------------------------------------------------------
JackTripVSTProcessor::JackTripVSTProcessor()
{
    //--- set the wanted controller for our processor
    setControllerClass(kJackTripVSTControllerUID);
}

//------------------------------------------------------------------------
JackTripVSTProcessor::~JackTripVSTProcessor() {}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::initialize(FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---always initialize the parent-------
    tresult result = AudioEffect::initialize(context);
    // if everything Ok, continue
    if (result != kResultOk) {
        return result;
    }

    //--- create Audio IO ------
    addAudioInput(STR16("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

    getQtAppPtr();

    mInputBuffer  = new float*[AudioSocketNumChannels];
    mOutputBuffer = new float*[AudioSocketNumChannels];
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        mInputBuffer[i]  = new float[AudioSocketMaxSamplesPerBlock];
        mOutputBuffer[i] = new float[AudioSocketMaxSamplesPerBlock];
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::terminate()
{
    mSocketPtr.reset();

    for (int i = 0; i < AudioSocketNumChannels; i++) {
        delete[] mInputBuffer[i];
        delete[] mOutputBuffer[i];
    }
    delete[] mInputBuffer;
    delete[] mOutputBuffer;

    //---do not forget to call parent ------
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setActive(TBool state)
{
    if (state) {
        // sanity check to ensure these were initialized by setupProcessing()
        if (mSampleRate == 0 || mBufferSize == 0) {
            return kResultFalse;
        }
        // create a audio new socket
        if (mSocketPtr.isNull()) {
            // not yet initialized
            mSocketPtr.reset(new AudioSocket(true));
            // automatically retry to establish connection
            mSocketPtr->setRetryConnection(true);
            mSocketPtr->connect(mSampleRate, mBufferSize);
        }
    } else {
        // disconnect from remote when inactive
        mSocketPtr.reset();
    }
    //--- called when the Plug-in is enable/disable (On/Off) -----
    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setProcessing(TBool state)
{
    return AudioEffect::setProcessing(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::process(Vst::ProcessData& data)
{
    //--- Read inputs parameter changes-----------
    if (data.inputParameterChanges) {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount();
        for (int32 index = 0; index < numParamsChanged; index++) {
            Vst::IParamValueQueue* paramQueue =
                data.inputParameterChanges->getParameterData(index);
            if (paramQueue) {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount();
                switch (paramQueue->getParameterId()) {
                case JackTripVSTParams::kParamVolSendId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mSendVol = value;
                    break;
                case JackTripVSTParams::kParamVolReceiveId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mReceiveVol = value;
                    break;
                case JackTripVSTParams::kParamVolPassId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mPassVol = value;
                    break;
                case JackTripVSTParams::kParamConnectedId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mConnected = value > 0;
                    break;
                case JackTripVSTParams::kBypassId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mBypass = value > 0;
                    break;
                }
            }
        }
    }

    // handle connection state change
    if (mConnected != mSocketPtr->isConnected() && data.outputParameterChanges) {
        int32 index = 0;
        Steinberg::Vst::IParamValueQueue* paramQueue =
            data.outputParameterChanges->addParameterData(kParamConnectedId, index);
        if (paramQueue) {
            mConnected          = mSocketPtr->isConnected();
            int8 connectedState = mConnected ? 1 : 0;
            int32 index2        = 0;
            paramQueue->addPoint(0, connectedState, index2);
        }
    }

    //--- Process Audio---------------------
    //--- ----------------------------------
    if (data.numInputs == 0 || data.numOutputs == 0) {
        // nothing to do
        return kResultOk;
    }

    if (data.numSamples <= 0) {
        // nothing to do
        return kResultOk;
    }

    if (mBypass) {
        // copy input to output
        for (int i = 0; i < data.inputs[0].numChannels && i < data.outputs[0].numChannels;
             i++) {
            memcpy(data.outputs[0].channelBuffers32[i],
                   data.inputs[0].channelBuffers32[i],
                   data.numSamples * sizeof(Steinberg::Vst::Sample32));
        }
        data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
        return kResultOk;
    }

    // Process Algorithm
    // Ex: algo.process (data.inputs[0].channelBuffers32,
    // data.outputs[0].channelBuffers32, data.numSamples);

    // clear buffers
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        memset(mInputBuffer[i], 0, AudioSocketMaxSamplesPerBlock * sizeof(float));
        memset(mOutputBuffer[i], 0, AudioSocketMaxSamplesPerBlock * sizeof(float));
    }

    // copy input to buffer
    if (mSendVol >= 0.0000001) {
        Steinberg::uint64 inSilentFlag = 1;
        int channelsIn = std::min(data.inputs[0].numChannels, AudioSocketNumChannels);
        for (int i = 0; i < channelsIn; i++) {
            bool isSilent = inSilentFlag & data.inputs[0].silenceFlags;
            inSilentFlag <<= 1;
            if (isSilent)
                continue;
            Steinberg::Vst::Sample32* inBuffer = data.inputs[0].channelBuffers32[i];
            for (int j = 0; j < data.numSamples; j++) {
                mInputBuffer[i][j] = inBuffer[j] * mSendVol;
            }
        }
    }

    // send to audio socket
    mSocketPtr->compute(data.numSamples, mInputBuffer, mOutputBuffer);

    // copy buffer to output
    for (int i = 0; i < data.outputs[0].numChannels; i++) {
        bool silent = true;
        memset(data.outputs[0].channelBuffers32[i], 0,
               data.numSamples * sizeof(Steinberg::Vst::Sample32));
        if (mPassVol >= 0.0000001
            || mReceiveVol >= 0.0000001) {  // silent output shortcut
            Steinberg::Vst::Sample32* outBuffer = data.outputs[0].channelBuffers32[i];
            for (int j = 0; j < data.numSamples; j++) {
                if (i < AudioSocketNumChannels && mReceiveVol >= 0.0000001) {
                    outBuffer[j] = mOutputBuffer[i][j] * mReceiveVol;
                }
                // TODO: is addition sufficient for mixing audio?
                if (i < data.inputs[0].numChannels && mPassVol >= 0.0000001) {
                    outBuffer[j] += data.inputs[0].channelBuffers32[i][j] * mPassVol;
                }
                if (silent && outBuffer[j] != 0) {
                    silent = false;
                }
            }
        }
        if (silent) {
            data.outputs[0].silenceFlags |= 1 << i;
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
{
    mSampleRate = newSetup.sampleRate;
    mBufferSize = newSetup.maxSamplesPerBlock;

    //--- called before any processing ----
    return AudioEffect::setupProcessing(newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::canProcessSampleSize(int32 symbolicSampleSize)
{
    // by default kSample32 is supported
    if (symbolicSampleSize == Vst::kSample32)
        return kResultTrue;

    // disable the following comment if your processing support kSample64
    /* if (symbolicSampleSize == Vst::kSample64)
        return kResultTrue; */

    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setState(IBStream* state)
{
    if (!state)
        return kResultFalse;

    // called when we load a preset or project, the model has to be reloaded

    IBStreamer streamer(state, kLittleEndian);

    float sendVol = 1.f;
    if (streamer.readFloat(sendVol) == false)
        return kResultFalse;

    float receiveVol = 1.f;
    if (streamer.readFloat(receiveVol) == false)
        return kResultFalse;

    float passVol = 1.f;
    if (streamer.readFloat(passVol) == false)
        return kResultFalse;

    int8 connectedState = 0;
    if (streamer.readInt8(connectedState) == false)
        return kResultFalse;

    int32 bypassState = 0;
    if (streamer.readInt32(bypassState) == false)
        return kResultFalse;

    mSendVol    = sendVol;
    mReceiveVol = receiveVol;
    mPassVol    = passVol;
    mConnected  = connectedState > 0;
    mBypass     = bypassState > 0;

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::getState(IBStream* state)
{
    // here we need to save the model (preset or project)

    float sendVol       = mSendVol;
    float receiveVol    = mReceiveVol;
    float passVol       = mPassVol;
    int8 connectedState = mConnected ? 1 : 0;
    int32 bypassState   = mBypass ? 1 : 0;

    IBStreamer streamer(state, kLittleEndian);
    streamer.writeFloat(sendVol);
    streamer.writeFloat(receiveVol);
    streamer.writeFloat(passVol);
    streamer.writeInt8(connectedState);
    streamer.writeInt32(bypassState);

    return kResultOk;
}

//------------------------------------------------------------------------
}  // namespace Steinberg
