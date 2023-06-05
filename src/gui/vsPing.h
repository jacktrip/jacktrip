//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsPing.h
 * \author Dominick Hing
 * \date July 2022
 */

#ifndef VSPING_H
#define VSPING_H

#include <QAbstractSocket>
#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <QtWebSockets>
#include <stdexcept>

/** \brief A helper class for VsPinger
 *
 */
class VsPing : public QObject
{
    Q_OBJECT;

   public:
    explicit VsPing(uint32_t pingNum, uint32_t timeout_msec);
    virtual ~VsPing() { mTimer.stop(); }
    uint32_t pingNumber() { return mPingNumber; }

    QDateTime sentTimestamp() { return mSent; }
    QDateTime receivedTimestamp() { return mReceived; }
    bool receivedReply() { return mReceivedReply; }
    bool timedOut() { return mTimedOut; }

    void send();
    void receive();

   private:
    uint32_t mPingNumber;
    QDateTime mSent;
    QDateTime mReceived;

    QTimer mTimer;
    bool mTimedOut      = false;
    bool mReceivedReply = false;

   public slots:
    void onTimeout();

   signals:
    void timeout(uint32_t pingNum);
};

#endif  // VSPING_H