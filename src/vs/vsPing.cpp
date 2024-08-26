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

/**
 * \file vsPinger.cpp
 * \author Dominick Hing
 * \date July 2022
 */

#include "vsPing.h"

#include <iostream>

using std::cout;
using std::endl;

// NOTE: It's better not to use
// using namespace std;
// because some functions (like exit()) get confused with QT functions

//*******************************************************************************
VsPing::VsPing(uint32_t pingNum, uint32_t timeout_msec) : mPingNumber(pingNum)
{
    connect(&mTimer, &QTimer::timeout, this, &VsPing::onTimeout);

    mTimer.setTimerType(Qt::PreciseTimer);
    mTimer.setSingleShot(true);
    mTimer.setInterval(timeout_msec);
    mTimer.start();
}

void VsPing::send()
{
    QDateTime now = QDateTime::currentDateTime();
    mSent         = now;
}

void VsPing::receive()
{
    QDateTime now = QDateTime::currentDateTime();
    if (!mTimedOut) {
        mTimer.stop();
        mReceivedReply = true;
        mReceived      = now;
    }
}

void VsPing::onTimeout()
{
    if (!mReceivedReply) {
        mTimedOut = true;
        emit timeout(mPingNumber);
    }
}
