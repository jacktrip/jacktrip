//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2020 Aaron Wyatt.

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

#include "NoNap.h"
#include <Foundation/Foundation.h>

NoNap::NoNap() :
    m_preventNap(false)
{}

void NoNap::disableNap()
{
    if (m_preventNap) {
        return;
    }
    m_preventNap = true;
    m_activity = [[NSProcessInfo processInfo] beginActivityWithOptions:NSActivityLatencyCritical | NSActivityUserInitiated reason:@"Disable App Nap"];
    [m_activity retain];
}

void NoNap::enableNap()
{
    if (!m_preventNap) {
        return;
    }
    m_preventNap = false;
    [[NSProcessInfo processInfo] endActivity:m_activity];
    [m_activity release];
}

NoNap::~NoNap()
{
    if (m_preventNap) {
        enableNap();
    }
}
