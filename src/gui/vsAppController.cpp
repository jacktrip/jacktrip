//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

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

/**
 * \file vsAppController.cpp
 * \author Dominick Hing
 * \date August 2023
 */

#include "vsAppController.h"

VsAppController::VsAppController(VsQuickView* view)
{
    m_view = view;
    m_view->engine()->rootContext()->setContextProperty(QStringLiteral("appctl"), this);

    // set default values
    setDeviceControlsCollapsed(false);
}

bool VsAppController::deviceControlsCollapsed()
{
    return m_deviceControlsCollapsed;
}

void VsAppController::setDeviceControlsCollapsed(bool deviceControlsCollapsed)
{
    m_deviceControlsCollapsed = deviceControlsCollapsed;
    emit deviceControlsCollapsedChanged(deviceControlsCollapsed);
}