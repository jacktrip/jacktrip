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

// Based on the Hello World VST 3 example from Steinberg
// https://github.com/steinbergmedia/vst3_example_plugin_hello_world

#include "JackTripVSTProcessor.h"

#include "JackTripVST.h"
#include "JackTripVSTDataBlock.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace std;
using namespace Steinberg;

//------------------------------------------------------------------------
// JackTripVSTProcessor
//------------------------------------------------------------------------
JackTripVSTProcessor::JackTripVSTProcessor()
{
    //--- set the wanted controller for our processor
    setControllerClass(kJackTripVSTControllerUID);
    mCurrentExchangeBlock =
        Vst::DataExchangeBlock{nullptr, 0, Vst::InvalidDataExchangeBlockID};
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
    addAudioInput(STR16("Stereo In"), Vst::SpeakerArr::kStereo);
    addAudioOutput(STR16("Stereo Out"), Vst::SpeakerArr::kStereo);

    // qDebug() << "JackTrip VST initialized";

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::terminate()
{
    mProcessor.uninitialize();

    // qDebug() << "JackTrip VST terminated";

    //---do not forget to call parent ------
    return AudioEffect::terminate();
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::connect(Vst::IConnectionPoint* other)
{
    auto result = Vst::AudioEffect::connect(other);
    if (result == kResultTrue) {
        auto configCallback = [](Vst::DataExchangeHandler::Config& config,
                                 [[maybe_unused]] const Vst::ProcessSetup& setup) {
            config.blockSize     = sizeof(DataBlock);
            config.numBlocks     = 2;  // max number of pending blocks allowed
            config.alignment     = 32;
            config.userContextID = 0;
            return true;
        };
        mDataExchangePtr.reset(new Vst::DataExchangeHandler(this, configCallback));
        mDataExchangePtr->onConnect(other, getHostContext());
    }
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::disconnect(Vst::IConnectionPoint* other)
{
    if (!mDataExchangePtr.isNull()) {
        mDataExchangePtr->onDisconnect(other);
        mDataExchangePtr.reset();
    }
    return AudioEffect::disconnect(other);
}

//------------------------------------------------------------------------
tresult PLUGIN_API
JackTripVSTProcessor::setBusArrangements(Vst::SpeakerArrangement* inputs, int32 numIns,
                                         Vst::SpeakerArrangement* outputs, int32 numOuts)
{
    // based on again example from sdk (support 1->1 or 2->2)
    if (numIns == 1 && numOuts == 1) {
        // the host wants Mono => Mono (or 1 channel -> 1 channel)
        if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 1
            && Vst::SpeakerArr::getChannelCount(outputs[0]) == 1) {
            auto* bus = FCast<Steinberg::Vst::AudioBus>(audioInputs.at(0));
            if (bus) {
                // check if we are Mono => Mono, if not we need to recreate the busses
                if (bus->getArrangement() != inputs[0]) {
                    getAudioInput(0)->setArrangement(inputs[0]);
                    getAudioInput(0)->setName(STR16("Mono In"));
                    getAudioOutput(0)->setArrangement(outputs[0]);
                    getAudioOutput(0)->setName(STR16("Mono Out"));
                }
                return kResultOk;
            }
        } else {
            // the host wants something else than Mono => Mono,
            // in this case we are always Stereo => Stereo
            auto* bus = FCast<Steinberg::Vst::AudioBus>(audioInputs.at(0));
            if (bus) {
                tresult result = kResultFalse;
                // the host wants 2->2 (could be LsRs -> LsRs)
                if (Vst::SpeakerArr::getChannelCount(inputs[0]) == 2
                    && Vst::SpeakerArr::getChannelCount(outputs[0]) == 2) {
                    getAudioInput(0)->setArrangement(inputs[0]);
                    getAudioInput(0)->setName(STR16("Stereo In"));
                    getAudioOutput(0)->setArrangement(outputs[0]);
                    getAudioOutput(0)->setName(STR16("Stereo Out"));
                    result = kResultTrue;
                } else if (bus->getArrangement() != Vst::SpeakerArr::kStereo) {
                    // the host want something different than 1->1 or 2->2 : in this case
                    // we want stereo
                    getAudioInput(0)->setArrangement(Vst::SpeakerArr::kStereo);
                    getAudioInput(0)->setName(STR16("Stereo In"));
                    getAudioOutput(0)->setArrangement(Vst::SpeakerArr::kStereo);
                    getAudioOutput(0)->setName(STR16("Stereo Out"));
                    result = kResultFalse;
                }
                return result;
            }
        }
    }
    return kResultFalse;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setActive(TBool state)
{
    if (state) {
        // sanity check to ensure these were initialized by setupProcessing()
        if (mSampleRate == 0 || mBufferSize == 0) {
            return kResultFalse;
        }
        // initialize the audio bridge processor
        mProcessor.initialize(mSampleRate, mBufferSize);
        // activate data exchange API
        if (!mDataExchangePtr.isNull()) {
            mDataExchangePtr->onActivate(processSetup);
        }
    } else {
        // uninitialize the audio bridge processor
        mProcessor.uninitialize();
        // deactivate data exchange API
        if (!mDataExchangePtr.isNull()) {
            mDataExchangePtr->onDeactivate();
        }
    }

    // qDebug() << "JackTrip VST setActive(" << int(state) << ")";

    //--- called when the Plug-in is enable/disable (On/Off) -----
    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setProcessing(TBool state)
{
    // qDebug() << "JackTrip VST setProcessing(" << int(state) << ")";
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
                case JackTripVSTParams::kParamGainSendId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mSendGain = value;
                    break;
                case JackTripVSTParams::kParamMixOutputId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mOutputMix = value;
                    break;
                case JackTripVSTParams::kParamGainOutputId:
                    if (paramQueue->getPoint(numPoints - 1, sampleOffset, value)
                        == kResultTrue)
                        mOutputGain = value;
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
        if (numParamsChanged > 0)
            updateVolumeMultipliers();
    }

    // handle connection state change
    if (mConnected != mProcessor.isConnected()) {
        // try both methods because some hosts only support one or the other.
        // first try to use data output parameters, if available.
        bool updatedConnectedState = false;
        if (data.outputParameterChanges) {
            int32 index = 0;
            Steinberg::Vst::IParamValueQueue* paramQueue =
                data.outputParameterChanges->addParameterData(kParamConnectedId, index);
            if (paramQueue) {
                int8 connectedState = mProcessor.isConnected() ? 1 : 0;
                int32 index2        = 0;
                if (paramQueue->addPoint(0, connectedState, index2) == kResultOk) {
                    updatedConnectedState = true;
                }
            }
        }
        // if unsuccessful, try to use data exchange API
        if (!updatedConnectedState && !mDataExchangePtr.isNull()) {
            if (mCurrentExchangeBlock.blockID == Vst::InvalidDataExchangeBlockID) {
                acquireNewExchangeBlock();
            }
            if (auto block = toDataBlock(mCurrentExchangeBlock)) {
                block->connectedState = mProcessor.isConnected();
                if (mDataExchangePtr->sendCurrentBlock()) {
                    updatedConnectedState = true;
                }
                // we need to acquire a new block before the current one will be sent
                acquireNewExchangeBlock();
            }
        }
        if (updatedConnectedState) {
            // we can update our state after successfully deliver the change
            mConnected = mProcessor.isConnected();
        }
    }

    //--- Process Audio---------------------
    //--- ----------------------------------
    if (data.numSamples <= 0 || data.numInputs == 0 || data.numOutputs == 0) {
        // nothing to do
        return kResultOk;
    }

    if (data.numSamples > AudioSocketMaxSamplesPerBlock) {
        // just a sanity check; shouldn't happen
        data.numSamples = AudioSocketMaxSamplesPerBlock;
    }

    if (mBypass) {
        // copy input to output
        for (int i = 0; i < data.inputs[0].numChannels && i < data.outputs[0].numChannels;
             i++) {
            memcpy(data.outputs[0].channelBuffers32[i],
                   data.inputs[0].channelBuffers32[i],
                   data.numSamples * sizeof(Vst::Sample32));
        }
        data.outputs[0].silenceFlags = data.inputs[0].silenceFlags;
        return kResultOk;
    }

    // Set up input buffers
    bool inputSilenceFlags[AudioSocketNumChannels];
    float* inputBuffers[AudioSocketNumChannels];
    for (int ch = 0; ch < AudioSocketNumChannels; ch++) {
        if (ch < data.inputs[0].numChannels) {
            inputBuffers[ch] = static_cast<float*>(data.inputs[0].channelBuffers32[ch]);
            uint64 isSilentFlag   = static_cast<uint64>(1) << ch;
            inputSilenceFlags[ch] = (isSilentFlag & data.inputs[0].silenceFlags) != 0;
        } else {
            inputBuffers[ch]      = nullptr;
            inputSilenceFlags[ch] = true;
        }
    }

    // Set up output buffers
    bool outputSilenceFlags[AudioSocketNumChannels];
    float* outputBuffers[AudioSocketNumChannels];
    for (int ch = 0; ch < AudioSocketNumChannels; ch++) {
        if (ch < data.outputs[0].numChannels) {
            outputBuffers[ch] = static_cast<float*>(data.outputs[0].channelBuffers32[ch]);
        } else {
            outputBuffers[ch] = nullptr;
        }
    }

    // Process through the audio bridge processor
    mProcessor.process(inputBuffers, outputBuffers, inputSilenceFlags, outputSilenceFlags,
                       data.numSamples);

    // Update silence flags
    // Handle any remaining output channels by zeroing them
    for (int ch = 0; ch < data.outputs[0].numChannels; ch++) {
        bool silent = true;
        if (ch < AudioSocketNumChannels) {
            silent = outputSilenceFlags[ch];
        } else {
            memset(data.outputs[0].channelBuffers32[ch], 0,
                   data.numSamples * sizeof(Vst::Sample32));
            silent = true;
        }
        if (silent) {
            data.outputs[0].silenceFlags |= static_cast<Steinberg::uint64>(1) << ch;
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
void JackTripVSTProcessor::updateVolumeMultipliers()
{
    // convert [0-1.0] gain (dB) values into [0-1.0] volume multiplers
    float outMul  = AudioBridgeProcessor::gainToVol(mOutputGain);
    float sendMul = AudioBridgeProcessor::gainToVol(mSendGain);
    float recvMul = mOutputMix * outMul;
    float passMul = (1.0f - mOutputMix) * outMul;

    mProcessor.setSendMul(sendMul);
    mProcessor.setRecvMul(recvMul);
    mProcessor.setPassMul(passMul);

    // qDebug() << "JackTrip VST send =" << sendMul << "(" << mSendGain
    //          << "), out =" << outMul << "(" << mOutputGain << "), mix =" << mOutputMix
    //          << ", recv =" << recvMul << ", pass =" << passMul;
}

//------------------------------------------------------------------------
void JackTripVSTProcessor::acquireNewExchangeBlock()
{
    mCurrentExchangeBlock = mDataExchangePtr->getCurrentOrNewBlock();
    if (auto block = toDataBlock(mCurrentExchangeBlock)) {
        block->connectedState = false;  // default
    }
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setupProcessing(Vst::ProcessSetup& newSetup)
{
    mSampleRate = newSetup.sampleRate;
    mBufferSize = static_cast<int>(newSetup.maxSamplesPerBlock);

    // qDebug() << "JackTrip VST setupProcessing: mSampleRate=" << mSampleRate
    //          << ", mbufferSize=" << mBufferSize;

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

    float sendGain = 1.f;
    if (streamer.readFloat(sendGain) == false)
        return kResultFalse;

    float outputMix = 1.f;
    if (streamer.readFloat(outputMix) == false)
        return kResultFalse;

    float outputGain = 1.f;
    if (streamer.readFloat(outputGain) == false)
        return kResultFalse;

    int8 connectedState = 0;
    if (streamer.readInt8(connectedState) == false)
        return kResultFalse;

    int32 bypassState = 0;
    if (streamer.readInt32(bypassState) == false)
        return kResultFalse;

    mSendGain   = sendGain;
    mOutputMix  = outputMix;
    mOutputGain = outputGain;
    mConnected  = connectedState > 0;
    mBypass     = bypassState > 0;

    updateVolumeMultipliers();

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::getState(IBStream* state)
{
    // here we need to save the model (preset or project)

    float sendGain      = mSendGain;
    float outputMix     = mOutputMix;
    float outputGain    = mOutputGain;
    int8 connectedState = mProcessor.isConnected() ? 1 : 0;
    int32 bypassState   = mBypass ? 1 : 0;

    IBStreamer streamer(state, kLittleEndian);
    streamer.writeFloat(sendGain);
    streamer.writeFloat(outputMix);
    streamer.writeFloat(outputGain);
    streamer.writeInt8(connectedState);
    streamer.writeInt32(bypassState);

    return kResultOk;
}
