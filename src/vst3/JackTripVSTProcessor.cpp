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

#include "../AudioSocket.h"
#include "JackTripVST.h"
#include "JackTripVSTDataBlock.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace std;
using namespace Steinberg;

// uncomment to generate log file, for debugging purposes
// #define JACKTRIP_VST_LOG

#ifdef JACKTRIP_VST_LOG
#if defined(_WIN32)
#define JACKTRIP_VST_LOG_PATH "c:/JackTripTemp"
#define JACKTRIP_VST_LOG_FILE "c:/JackTripTemp/vst.log"
#else
#define JACKTRIP_VST_LOG_PATH "/tmp/jacktrip"
#define JACKTRIP_VST_LOG_FILE "/tmp/jacktrip/vst.log"
#endif
#include <filesystem>
#include <fstream>
#include <iostream>

static ofstream kLogFile;

void qtMessageHandler([[maybe_unused]] QtMsgType type,
                      [[maybe_unused]] const QMessageLogContext& context,
                      const QString& msg)
{
    kLogFile << msg.toStdString() << endl;
}
#endif

// any multiplier less than this is considered to be silent
constexpr double kSilentMul = 0.0000001;

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

    getQtAppPtr();

    mInputBuffer  = new float*[AudioSocketNumChannels];
    mOutputBuffer = new float*[AudioSocketNumChannels];
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        mInputBuffer[i]  = new float[AudioSocketMaxSamplesPerBlock];
        mOutputBuffer[i] = new float[AudioSocketMaxSamplesPerBlock];
    }

#ifdef JACKTRIP_VST_LOG
    if (!filesystem::is_directory(JACKTRIP_VST_LOG_PATH)) {
        if (!filesystem::create_directory(JACKTRIP_VST_LOG_PATH)) {
            qDebug() << "Failed to create VST log directory: " << JACKTRIP_VST_LOG_PATH;
        }
    }
    kLogFile.open(JACKTRIP_VST_LOG_FILE, ios::app);
    if (kLogFile.is_open()) {
        kLogFile << "JackTrip VST initialized" << endl;
        kLogFile.flush();
        cout.rdbuf(kLogFile.rdbuf());
        cerr.rdbuf(kLogFile.rdbuf());
    } else {
        qDebug() << "Failed to open VST log file: " << JACKTRIP_VST_LOG_FILE;
    }
    qInstallMessageHandler(qtMessageHandler);
#endif

    qDebug() << "JackTrip VST initialized";

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

    qDebug() << "JackTrip VST terminated";

#ifdef JACKTRIP_VST_LOG
    kLogFile.close();
#endif

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
        // create a audio new socket
        if (mSocketPtr.isNull()) {
            // not yet initialized
            mSocketPtr.reset(new AudioSocket(true));
            // automatically retry to establish connection
            mSocketPtr->setRetryConnection(true);
            mSocketPtr->connect(mSampleRate, mBufferSize);
        }
        // activate data exchange API
        if (!mDataExchangePtr.isNull()) {
            mDataExchangePtr->onActivate(processSetup);
        }
    } else {
        // disconnect from remote when inactive
        mSocketPtr.reset();
        // deactivate data exchange API
        if (!mDataExchangePtr.isNull()) {
            mDataExchangePtr->onDeactivate();
        }
    }

    qDebug() << "JackTrip VST setActive(" << int(state) << ")";

    //--- called when the Plug-in is enable/disable (On/Off) -----
    return AudioEffect::setActive(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setProcessing(TBool state)
{
    qDebug() << "JackTrip VST setProcessing(" << int(state) << ")";
    return AudioEffect::setProcessing(state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::process(Vst::ProcessData& data)
{
    // sanity check; should never happen
    if (mSocketPtr.isNull())
        return kResultFalse;

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

#if 0
    if (mLogFile.is_open()) {
        mLogFile << "JackTrip VST process: inputs=" << data.numInputs
                 << ", outputs=" << data.numOutputs
                 << ", samples=" << data.numSamples
                 << endl;
    }
#endif

    // handle connection state change
    if (mConnected != mSocketPtr->isConnected()) {
        // try both methods because some hosts only support one or the other.
        // first try to use data output parameters, if available.
        bool updatedConnectedState = false;
        if (data.outputParameterChanges) {
            int32 index = 0;
            Steinberg::Vst::IParamValueQueue* paramQueue =
                data.outputParameterChanges->addParameterData(kParamConnectedId, index);
            if (paramQueue) {
                int8 connectedState = mSocketPtr->isConnected() ? 1 : 0;
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
                block->connectedState = mSocketPtr->isConnected();
                if (mDataExchangePtr->sendCurrentBlock()) {
                    updatedConnectedState = true;
                }
                // we need to acquire a new block before the current one will be sent
                acquireNewExchangeBlock();
            }
        }
        if (updatedConnectedState) {
            // we can update our state after successfully deliver the change
            mConnected = mSocketPtr->isConnected();
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

    // clear buffers
    for (int i = 0; i < AudioSocketNumChannels; i++) {
        memset(mInputBuffer[i], 0, data.numSamples * sizeof(float));
        memset(mOutputBuffer[i], 0, data.numSamples * sizeof(float));
    }

    // copy input to buffer
    if (mSendMul >= kSilentMul) {
        uint64 isSilentFlag = 1;
        int channelsIn      = min(data.inputs[0].numChannels, AudioSocketNumChannels);
        for (int i = 0; i < channelsIn; i++) {
            bool isSilent = isSilentFlag & data.inputs[0].silenceFlags;
            isSilentFlag <<= 1;
            if (isSilent)
                continue;
            Vst::Sample32* inBuffer = data.inputs[0].channelBuffers32[i];
            for (int j = 0; j < data.numSamples; j++) {
                mInputBuffer[i][j] = inBuffer[j] * mSendMul;
            }
        }
    }

    // send to audio socket
    mSocketPtr->compute(data.numSamples, mInputBuffer, mOutputBuffer);

    // copy buffer to output
    for (int i = 0; i < data.outputs[0].numChannels; i++) {
        bool silent = true;
        memset(data.outputs[0].channelBuffers32[i], 0,
               data.numSamples * sizeof(Vst::Sample32));
        if (mPassMul >= kSilentMul || mRecvMul >= kSilentMul) {
            Vst::Sample32* outBuffer = data.outputs[0].channelBuffers32[i];
            for (int j = 0; j < data.numSamples; j++) {
                if (i < AudioSocketNumChannels && mRecvMul >= kSilentMul) {
                    outBuffer[j] = mOutputBuffer[i][j] * mRecvMul;
                }
                if (i < data.inputs[0].numChannels && mPassMul >= kSilentMul) {
                    outBuffer[j] += data.inputs[0].channelBuffers32[i][j] * mPassMul;
                }
                if (silent && outBuffer[j] != 0) {
                    silent = false;
                }
            }
        }
        if (silent) {
            data.outputs[0].silenceFlags |= static_cast<Steinberg::uint64>(1) << i;
        }
    }

    return kResultOk;
}

//------------------------------------------------------------------------
float JackTripVSTProcessor::gainToVol(double gain)
{
    // handle min and max
    if (gain < kSilentMul)
        return 0;
    if (gain > 0.9999999)
        return 1.0;
    // simple logarithmic conversion
    return exp(log(1000) * gain) / 1000.0;
}

//------------------------------------------------------------------------
void JackTripVSTProcessor::updateVolumeMultipliers()
{
    // convert [0-1.0] gain (dB) values into [0-1.0] volume multiplers
    float outMul = gainToVol(mOutputGain);
    mSendMul     = gainToVol(mSendGain);
    mRecvMul     = mOutputMix * outMul;
    mPassMul     = (1.0f - mOutputMix) * outMul;

    qDebug() << "JackTrip VST send =" << mSendMul << "(" << mSendGain
             << "), out =" << outMul << "(" << mOutputGain << "), mix =" << mOutputMix
             << ", recv =" << mRecvMul << ", pass =" << mPassMul;
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

    qDebug() << "JackTrip VST setupProcessing: mSampleRate=" << mSampleRate
             << ", mbufferSize=" << mBufferSize;

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
    int8 connectedState = mConnected ? 1 : 0;
    int32 bypassState   = mBypass ? 1 : 0;

    IBStreamer streamer(state, kLittleEndian);
    streamer.writeFloat(sendGain);
    streamer.writeFloat(outputMix);
    streamer.writeFloat(outputGain);
    streamer.writeInt8(connectedState);
    streamer.writeInt32(bypassState);

    return kResultOk;
}
