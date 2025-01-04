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

#include "JackTripVSTController.h"
#include "JackTripVST.h"

#include "vstgui/plugin-bindings/vst3editor.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"


using namespace Steinberg;


namespace Steinberg {


//------------------------------------------------------------------------
// JackTripVSTController Implementation
//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::initialize (FUnknown* context)
{
    // Here the Plug-in will be instantiated

    //---do not forget to call parent ------
    tresult result = EditControllerEx1::initialize (context);
    if (result != kResultOk)
    {
        return result;
    }

    // Here you could register some parameters
    if (result == kResultTrue)
    {
        //---Create Parameters------------
        parameters.addParameter (STR16 ("Send Volume"), STR16 ("dB"), 0, 1.,
                                 Vst::ParameterInfo::kCanAutomate, JackTripVSTParams::kParamVolSendId, 0,
                                 STR16 ("Send"));

        parameters.addParameter (STR16 ("Receive Volume"), STR16 ("dB"), 0, 1.,
                                 Vst::ParameterInfo::kCanAutomate, JackTripVSTParams::kParamVolReceiveId, 0,
                                 STR16 ("Receive"));

        parameters.addParameter (STR16 ("Passthrough Volume"), STR16 ("dB"), 0, 1.,
                                 Vst::ParameterInfo::kCanAutomate, JackTripVSTParams::kParamVolPassId, 0,
                                 STR16 ("Passthrough"));

        parameters.addParameter (STR16 ("Connected"), STR16 ("On/Off"), 1, 1.,
                                 Vst::ParameterInfo::kIsReadOnly, JackTripVSTParams::kParamConnectedId, 0,
                                 STR16 ("Connected"));

        parameters.addParameter (STR16 ("Bypass"), nullptr, 1, 0,
                                 Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
                                 JackTripVSTParams::kBypassId);
    }

    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::terminate ()
{
    // Here the Plug-in will be de-instantiated, last possibility to remove some memory!

    //---do not forget to call parent ------
    return EditControllerEx1::terminate ();
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setComponentState (IBStream* state)
{
    // Here you get the state of the component (Processor part)
    if (!state)
        return kResultFalse;

    IBStreamer streamer (state, kLittleEndian);

    float sendVol = 1.f;
    if (streamer.readFloat (sendVol) == false)
        return kResultFalse;
    setParamNormalized (JackTripVSTParams::kParamVolSendId, sendVol);

    float receiveVol = 1.f;
    if (streamer.readFloat (receiveVol) == false)
        return kResultFalse;
    setParamNormalized (JackTripVSTParams::kParamVolReceiveId, receiveVol);

    float passVol = 1.f;
    if (streamer.readFloat (passVol) == false)
        return kResultFalse;
    setParamNormalized (JackTripVSTParams::kParamVolPassId, passVol);

    int8 connectedState = 0;
    if (streamer.readInt8 (connectedState) == false)
        return kResultFalse;
    setParamNormalized (JackTripVSTParams::kParamConnectedId, connectedState);

    int32 bypassState;
    if (streamer.readInt32 (bypassState) == false)
        return kResultFalse;
    setParamNormalized (kBypassId, bypassState ? 1 : 0);

    return kResultOk;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setState (IBStream* state)
{
    // Here you get the state of the controller

    return kResultTrue;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getState (IBStream* state)
{
    // Here you are asked to deliver the state of the controller (if needed)
    // Note: the real state of your plug-in is saved in the processor

    return kResultTrue;
}

//------------------------------------------------------------------------
IPlugView* PLUGIN_API JackTripVSTController::createView (FIDString name)
{
    // Here the Host wants to open your editor (if you have one)
    if (FIDStringsEqual (name, Vst::ViewType::kEditor))
    {
        // create your editor here and return a IPlugView ptr of it
        auto* view = new VSTGUI::VST3Editor (this, "view", "JackTripEditor.uidesc");
        return view;
    }
    return nullptr;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::setParamNormalized (Vst::ParamID tag, Vst::ParamValue value)
{
    // called by host to update your parameters
    tresult result = EditControllerEx1::setParamNormalized (tag, value);
    return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getParamStringByValue (Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string)
{
    // called by host to get a string for given normalized value of a specific parameter
    // (without having to set the value!)
    return EditControllerEx1::getParamStringByValue (tag, valueNormalized, string);
}

//------------------------------------------------------------------------
tresult PLUGIN_API JackTripVSTController::getParamValueByString (Vst::ParamID tag, Vst::TChar* string, Vst::ParamValue& valueNormalized)
{
    // called by host to get a normalized value from a string representation of a specific parameter
    // (without having to set the value!)
    return EditControllerEx1::getParamValueByString (tag, string, valueNormalized);
}

//------------------------------------------------------------------------
} // namespace Steinberg
