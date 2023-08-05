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
 * \file vsWorker.cpp
 * \author Mike Dickey
 * \date June 2023
 */

#include "vsWorker.h"
#include "virtualstudio.h"

VsWorker::VsWorker(VirtualStudio *ptr)
    : mVsPtr(ptr)
{
    connect(this, &VsWorker::signalStartAudio, this, &VsWorker::_startAudio,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalStopAudio, this, &VsWorker::_stopAudio,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalRefreshDevices, this, &VsWorker::_refreshDevices,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalConnectToStudio, this, &VsWorker::_connectToStudio,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalCompleteConnection, this, &VsWorker::_completeConnection,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalConnectionFinished, this, &VsWorker::_connectionFinished,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalSendHeartbeat, this, &VsWorker::_sendHeartbeat,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalDisconnect, this, &VsWorker::_disconnect,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalProcessError, this, &VsWorker::_processError,
            Qt::QueuedConnection);
    connect(this, &VsWorker::signalExit, this, &VsWorker::_exit,
            Qt::QueuedConnection);
}

void VsWorker::_startAudio()
{
    mVsPtr->_startAudio();
}

void VsWorker::_stopAudio()
{
    mVsPtr->_stopAudio();
}

void VsWorker::_refreshDevices()
{
    mVsPtr->_refreshDevices();
}

void VsWorker::_connectToStudio(int studioIndex)
{
    mVsPtr->_connectToStudio(studioIndex);
}

void VsWorker::_completeConnection()
{
    mVsPtr->_completeConnection();
}

void VsWorker::_connectionFinished()
{
    mVsPtr->_connectionFinished();
}

void VsWorker::_sendHeartbeat()
{
    mVsPtr->_sendHeartbeat();
}

void VsWorker::_disconnect()
{
    mVsPtr->_disconnect();
}

void VsWorker::_processError(const QString& msg)
{
    mVsPtr->_processError(msg);
}

void VsWorker::_exit()
{
    mVsPtr->_exit();
}
