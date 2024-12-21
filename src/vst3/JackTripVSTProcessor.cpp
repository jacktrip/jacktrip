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
#include "JackTripVST.h"

#include "base/source/fstreamer.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"

using namespace Steinberg;

namespace Steinberg {
//------------------------------------------------------------------------
// JackTripVSTProcessor
//------------------------------------------------------------------------
JackTripVSTProcessor::JackTripVSTProcessor ()
{
	//--- set the wanted controller for our processor
	setControllerClass (kJackTripVSTControllerUID);
}

//------------------------------------------------------------------------
JackTripVSTProcessor::~JackTripVSTProcessor ()
{}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::initialize (FUnknown* context)
{
	// Here the Plug-in will be instantiated
	
	//---always initialize the parent-------
	tresult result = AudioEffect::initialize (context);
	// if everything Ok, continue
	if (result != kResultOk)
	{
		return result;
	}

	//--- create Audio IO ------
	addAudioInput (STR16 ("Stereo In"), Steinberg::Vst::SpeakerArr::kStereo);
	addAudioOutput (STR16 ("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);

/*
    if (mSocketClient.connect()) {
        // TODO: error handling, reconnecting, etc
        mSocketClient.sendHeader("audio");
    }
*/
	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::terminate ()
{
	//mSocketClient.close();

	//---do not forget to call parent ------
	return AudioEffect::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setActive (TBool state)
{
	//--- called when the Plug-in is enable/disable (On/Off) -----
	return AudioEffect::setActive (state);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::process (Vst::ProcessData& data)
{
    //--- Read inputs parameter changes-----------
    if (data.inputParameterChanges)
    {
        int32 numParamsChanged = data.inputParameterChanges->getParameterCount ();
        for (int32 index = 0; index < numParamsChanged; index++)
        {
            Vst::IParamValueQueue* paramQueue =
                data.inputParameterChanges->getParameterData (index);
            if (paramQueue)
            {
                Vst::ParamValue value;
                int32 sampleOffset;
                int32 numPoints = paramQueue->getPointCount ();
                switch (paramQueue->getParameterId ())
                {
                    case JackTripVSTParams::kParamVolId:
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mParam1 = value;
                        break;
                    case JackTripVSTParams::kParamOnId:
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mParam2 = value > 0 ? 1 : 0;
                        break;
                    case JackTripVSTParams::kBypassId:
                        if (paramQueue->getPoint (numPoints - 1, sampleOffset, value) ==
                            kResultTrue)
                            mBypass = (value > 0.5f);
                        break;
                }
            }
        }
    }

    //--- Process Audio---------------------
    //--- ----------------------------------
    if (data.numInputs == 0 || data.numOutputs == 0)
    {
        // nothing to do
        return kResultOk;
    }

    if (data.numSamples > 0)
    {
        // Process Algorithm
        // Ex: algo.process (data.inputs[0].channelBuffers32, data.outputs[0].channelBuffers32,
        // data.numSamples);
    }
    return kResultOk;

}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::setupProcessing (Vst::ProcessSetup& newSetup)
{
	//--- called before any processing ----
	return AudioEffect::setupProcessing (newSetup);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::canProcessSampleSize (int32 symbolicSampleSize)
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
tresult PLUGIN_API JackTripVSTProcessor::setState (IBStream* state)
{
	if (!state)
		return kResultFalse;

	// called when we load a preset or project, the model has to be reloaded

	IBStreamer streamer (state, kLittleEndian);

	float savedParam1 = 0.f;
	if (streamer.readFloat (savedParam1) == false)
		return kResultFalse;

	int32 savedParam2 = 0;
	if (streamer.readInt32 (savedParam2) == false)
		return kResultFalse;

	int32 savedBypass = 0;
	if (streamer.readInt32 (savedBypass) == false)
		return kResultFalse;

	mParam1 = savedParam1;
	mParam2 = savedParam2 > 0 ? 1 : 0;
	mBypass = savedBypass > 0;

	return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTProcessor::getState (IBStream* state)
{
	// here we need to save the model (preset or project)

	float toSaveParam1 = mParam1;
	int32 toSaveParam2 = mParam2;
	int32 toSaveBypass = mBypass ? 1 : 0;

	IBStreamer streamer (state, kLittleEndian);
	streamer.writeFloat (toSaveParam1);
	streamer.writeInt32 (toSaveParam2);
	streamer.writeInt32 (toSaveBypass);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace Steinberg
