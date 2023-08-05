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
 * \file vsWorker.h
 * \author Mike Dickey
 * \date June 2023
 */

#ifndef VSWORKER_H
#define VSWORKER_H

#include <QObject>
#include <QString>

class VirtualStudio;

class VsWorker : public QObject
{
    Q_OBJECT
  
  public:
    VsWorker(VirtualStudio *ptr);
    virtual ~VsWorker() {}

  public:
    Q_INVOKABLE void startAudio() { emit signalStartAudio(); }
    Q_INVOKABLE void stopAudio() { emit signalStopAudio(); }
    Q_INVOKABLE void refreshDevices() { emit signalRefreshDevices(); }
    Q_INVOKABLE void connectToStudio(int idx) { emit signalConnectToStudio(idx); }
    Q_INVOKABLE void completeConnection() { emit signalCompleteConnection(); }
    Q_INVOKABLE void connectionFinished() { emit signalConnectionFinished(); }
    Q_INVOKABLE void sendHeartbeat() { emit signalSendHeartbeat(); }
    Q_INVOKABLE void disconnect() { emit signalDisconnect(); }
    Q_INVOKABLE void processError(const QString& msg) { emit signalProcessError(msg); }
    Q_INVOKABLE void exit() { emit signalExit(); }

  signals:
    void signalStartAudio();
    void signalStopAudio();
    void signalRefreshDevices();
    void signalConnectToStudio(int idx);
    void signalCompleteConnection();
    void signalConnectionFinished();
    void signalSendHeartbeat();
    void signalDisconnect();
    void signalProcessError(const QString& msg);
    void signalExit();

  public slots:
    void _startAudio();
    void _stopAudio();
    void _refreshDevices();
    void _connectToStudio(int idx);
    void _completeConnection();
    void _connectionFinished();
    void _sendHeartbeat();
    void _disconnect();
    void _processError(const QString& msg);
    void _exit();

  private:
    VirtualStudio *mVsPtr;
};

#endif  // VSWORKER_H
