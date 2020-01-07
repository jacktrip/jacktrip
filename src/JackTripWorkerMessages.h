//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
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
 * \file JackTripWorkerMessages.h
 * \author Juan-Pablo Caceres
 * \date October 2008
 */

#ifndef __JACKTRIPWORKERMESSAGES_H__
#define __JACKTRIPWORKERMESSAGES_H__

#include <QObject>
#include <QTimer>

#include <iostream>

class JackTripWorkerMessages : public QObject
{
    Q_OBJECT;

public:
    JackTripWorkerMessages() {};
    virtual ~JackTripWorkerMessages() {};

    void play()
    {
        std::cout << "********** PALYING ***********************************" << std::endl;
        QTimer *timer = new QTimer(this);
        QObject::connect(timer, SIGNAL(timeout()), this, SLOT(slotTest()), Qt::QueuedConnection);
        timer->start(300);
    }

public slots:
    void slotTest()
    {
        std::cout << "---JackTripWorkerMessages slotTest()---" << std::endl;
    }

signals:
    void signalTest();
    /// Signal to stop the event loop inside the JackTripWorker Thread
    void signalStopEventLoop();

};

#endif //__JACKTRIPWORKERMESSAGES_H__
