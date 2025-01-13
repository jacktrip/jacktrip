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

#include "public.sdk/source/vst/utility/dataexchange.h"
#include "public.sdk/source/vst/vsteditcontroller.h"

//------------------------------------------------------------------------
//  JackTripVSTController
//------------------------------------------------------------------------
class JackTripVSTController
    : public Steinberg::Vst::EditControllerEx1
    , public Steinberg::Vst::IDataExchangeReceiver
{
   public:
    //------------------------------------------------------------------------
    JackTripVSTController()                = default;
    ~JackTripVSTController() SMTG_OVERRIDE = default;

    // Create function
    static Steinberg::FUnknown* createInstance(void* /*context*/)
    {
        return (Steinberg::Vst::IEditController*)new JackTripVSTController;
    }

    // IPluginBase
    Steinberg::tresult PLUGIN_API initialize(Steinberg::FUnknown* context) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API terminate() SMTG_OVERRIDE;

    // EditController
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state)
        SMTG_OVERRIDE;
    Steinberg::IPlugView* PLUGIN_API createView(Steinberg::FIDString name) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) SMTG_OVERRIDE;
    Steinberg::int32 PLUGIN_API getParameterCount() SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API setParamNormalized(
        Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API getParamStringByValue(
        Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized,
        Steinberg::Vst::String128 string) SMTG_OVERRIDE;
    Steinberg::tresult PLUGIN_API
    getParamValueByString(Steinberg::Vst::ParamID tag, Steinberg::Vst::TChar* string,
                          Steinberg::Vst::ParamValue& valueNormalized) SMTG_OVERRIDE;

    // IDataExchangeReceiver
    Steinberg::tresult PLUGIN_API notify(Steinberg::Vst::IMessage* message) override;
    void PLUGIN_API queueOpened(Steinberg::Vst::DataExchangeUserContextID userContextID,
                                Steinberg::uint32 blockSize,
                                Steinberg::TBool& dispatchOnBackgroundThread) override;
    void PLUGIN_API
    queueClosed(Steinberg::Vst::DataExchangeUserContextID userContextID) override;
    void PLUGIN_API onDataExchangeBlocksReceived(
        Steinberg::Vst::DataExchangeUserContextID userContextID,
        Steinberg::uint32 numBlocks, Steinberg::Vst::DataExchangeBlock* blocks,
        Steinberg::TBool onBackgroundThread) override;

    //---Interface---------
    DEFINE_INTERFACES
    DEF_INTERFACE(Steinberg::Vst::IDataExchangeReceiver)
    END_DEFINE_INTERFACES(EditController)
    DELEGATE_REFCOUNT(EditController)

   private:
    Steinberg::Vst::DataExchangeReceiverHandler mDataExchangeHandler{this};
};
