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

#include "JackTripVST.h"
#include "JackTripVSTController.h"
#include "JackTripVSTProcessor.h"
#include "public.sdk/source/main/pluginfactory.h"

#define stringPluginName "JackTrip Audio Bridge"

using namespace Steinberg::Vst;
using namespace Steinberg;

//------------------------------------------------------------------------
//  VST Plug-in Entry
//------------------------------------------------------------------------
// Windows: do not forget to include a .def file in your project to export
// GetPluginFactory function!
//------------------------------------------------------------------------

BEGIN_FACTORY_DEF("JackTrip Labs", "www.jacktrip.com", "mailto:support@jacktrip.com")

//---First Plug-in included in this factory-------
// its kVstAudioEffectClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kJackTripVSTProcessorUID),
           PClassInfo::kManyInstances,  // cardinality
           kVstAudioEffectClass,        // the component category (do not changed this)
           stringPluginName,            // here the Plug-in name (to be changed)
           Vst::kDistributable,         // means that component and controller could be
                                        // distributed on different computers
           JackTripVSTVST3Category,     // Subcategory for this Plug-in (to be changed)
           FULL_VERSION_STR,            // Plug-in version (to be changed)
           kVstVersionString,  // the VST 3 SDK version (do not changed this, use always
                               // this define)
           JackTripVSTProcessor::createInstance)  // function pointer called when this
                                                  // component should be instantiated

// its kVstComponentControllerClass component
DEF_CLASS2(INLINE_UID_FROM_FUID(kJackTripVSTControllerUID),
           PClassInfo::kManyInstances,    // cardinality
           kVstComponentControllerClass,  // the Controller category (do not changed this)
           stringPluginName
           "Controller",       // controller name (could be the same than component name)
           0,                  // not used here
           "",                 // not used here
           FULL_VERSION_STR,   // Plug-in version (to be changed)
           kVstVersionString,  // the VST 3 SDK version (do not changed this, use always
                               // this define)
           JackTripVSTController::createInstance)  // function pointer called when this
                                                   // component should be instantiated

//----for others Plug-ins contained in this factory, put like for the first Plug-in
// different DEF_CLASS2---

END_FACTORY
