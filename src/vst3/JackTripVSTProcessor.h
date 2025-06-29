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

#pragma once

#include <QCoreApplication>
#include <QScopedPointer>
#include <QThread>

#include "../AudioBridgeProcessor.h"
#include "public.sdk/source/vst/utility/dataexchange.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

//------------------------------------------------------------------------
//  JackTripVSTProcessor
//------------------------------------------------------------------------
class JackTripVSTProcessor : public Steinberg::Vst::AudioEffect
{
   public:
    JackTripVSTProcessor();
    ~JackTripVSTProcessor() SMTG_OVERRIDE;

    // Create function
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IAudioProcessor*)new JackTripVSTProcessor;
    }

    //--- ---------------------------------------------------------------------
    // AudioEffect overrides:
    //--- ---------------------------------------------------------------------
    /** Called at first after constructor */
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;

    /** Called at the end before destructor */
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

    /** Called to connect data exchange API */
    Steinberg::tresult PLUGIN_API
    connect(Steinberg::Vst::IConnectionPoint* other) override;

    /** Called to disconnect data exchange API */
    Steinberg::tresult PLUGIN_API
    disconnect(Steinberg::Vst::IConnectionPoint* other) override;

    /** Called to set bus arrangements */
    Steinberg::tresult PLUGIN_API setBusArrangements(
        Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns,
        Steinberg::Vst::SpeakerArrangement* outputs,
        Steinberg::int32 numOuts) SMTG_OVERRIDE;

    /** Switch the Plug-in on/off */
    Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) SMTG_OVERRIDE;

    /** Called by audio thread immediately before processing starts, and after it ends */
    Steinberg::tresult PLUGIN_API setProcessing(Steinberg::TBool state) SMTG_OVERRIDE;

    /** Will be called before any process call */
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& newSetup)
        SMTG_OVERRIDE;

    /** Asks if a given sample size is supported see SymbolicSampleSizes. */
    Steinberg::tresult PLUGIN_API
    canProcessSampleSize(Steinberg::int32 symbolicSampleSize) SMTG_OVERRIDE;

    /** Here we go...the process call */
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data)
        SMTG_OVERRIDE;

    /** For persistence */
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;

    //------------------------------------------------------------------------
   protected:
    void updateVolumeMultipliers();
    void acquireNewExchangeBlock();

    Steinberg::Vst::ParamValue mSendGain   = 1.f;
    Steinberg::Vst::ParamValue mOutputMix  = 0;
    Steinberg::Vst::ParamValue mOutputGain = 1.f;
    bool mConnected                        = false;
    bool mBypass                           = false;

   private:
    AudioBridgeProcessor mProcessor;
    QScopedPointer<Steinberg::Vst::DataExchangeHandler> mDataExchangePtr;
    Steinberg::Vst::DataExchangeBlock mCurrentExchangeBlock;
    Steinberg::Vst::SampleRate mSampleRate = 0;
    int mBufferSize                        = 0;
};
