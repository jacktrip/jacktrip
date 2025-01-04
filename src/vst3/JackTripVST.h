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

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

#define JackTripVSTVST3Category "Fx"
#define stringOriginalFilename  "JackTrip.vst3"
#define stringFileDescription   "JackTrip VST3"
#define stringCompanyName       "JackTrip Labs\0"
#define stringLegalCopyright    "Copyright(c) 2024 JackTrip Labs, Inc."
#define stringLegalTrademarks   "VST is a trademark of Steinberg Media Technologies GmbH"

namespace Steinberg {

//------------------------------------------------------------------------
enum JackTripVSTParams : Vst::ParamID
{
    kParamVolSendId = 100,
    kParamVolReceiveId = 101,
    kParamVolPassId = 102,
    kParamConnectedId = 200,
    kBypassId = 1000
};

//------------------------------------------------------------------------
static const Steinberg::FUID kJackTripVSTProcessorUID (0x176F9AF4, 0xA56041A1, 0x890DD021, 0x765ABCF0);
static const Steinberg::FUID kJackTripVSTControllerUID (0x075C3106, 0xBC524686, 0xB63544CC, 0xF88423FF);

//------------------------------------------------------------------------
} // namespace Steinberg
