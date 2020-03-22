/**
 * \file ThreadPoolTest.h
 * \author Juan-Pablo Caceres
 * \date October 2008
 */

#ifndef __THREADPOOLTEST_H__
#define __THREADPOOLTEST_H__

#include <QThreadPool>
#include <QEventLoop>
#include <QThread>
#include <QObject>

#include <iostream>

#include "NetKS.h"
#include "JackTripWorkerMessages.h"


class ThreadPoolTest : public QObject, public QRunnable
        //class ThreadPoolTest : public QThread
{
    Q_OBJECT;

public:
    ThreadPoolTest()
    {
        setAutoDelete(false);
    }

    void run()
    {
        JackTripWorkerMessages jtm;
        QThread testThread;
        //jtm.moveToThread(&testThread);

        //QObject::connect(&jtm, SIGNAL(signalTest()), &jtm, SLOT(slotTest()), Qt::QueuedConnection);
        testThread.start();
        jtm.play();
        //testThread.wait();

        //std::cout << "--------------- BEFORE ---------------" << std::endl;
        //NetKS netks;
        //netks.play();
        //std::cout << "--------------- AFTER ---------------" << std::endl;

        QEventLoop loop;
        //QObject::connect(this, SIGNAL(stopELoop()), &loop, SLOT(quit()), Qt::QueuedConnection);
        loop.exec();
        //std::cout << "--------------- EXITING QRUNNABLE---------------" << std::endl;
        /*
    while (true) {
      std::cout << "Hello world from thread" << std::endl;
      sleep(1);
    }
    */
    }

    void stop()
    {
        std::cout << "--------------- ELOOP STOP---------------" << std::endl;
        emit stopELoop();
    }

signals:
    void stopELoop();

private slots:
    void fromServer()
    {
        std::cout << "--------------- SIGNAL RECEIVED ---------------" << std::endl;
    }

};

#endif //__THREADPOOLTEST_H__
