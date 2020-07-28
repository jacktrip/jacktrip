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
 * \file main.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QLoggingCategory>

#include "JackAudioInterface.h"
#include "UdpDataProtocol.h"
#include "RingBuffer.h"
#include "JackTrip.h"
#include "Settings.h"
//#include "TestRingBuffer.h"
#include "LoopBack.h"
#include "PacketHeader.h"
//#include "JackTripThread.h"
#ifdef __RT_AUDIO__
#include "RtAudioInterface.h"
#endif
#include "jacktrip_tests.cpp"
#include "jacktrip_globals.h"
#include <signal.h>


void qtMessageHandler(QtMsgType /*type*/, const QMessageLogContext& /*context*/, const QString& msg)
{
    std::cerr << msg.toStdString() << std::endl;
}

#if defined (__LINUX__) || (__MAC_OSX__)
static int setupUnixSignalHandler(void (*handler)(int))
{
    //Setup our SIGINT handler.
    struct sigaction sigInt;
    sigInt.sa_handler = handler;
    sigemptyset(&sigInt.sa_mask);
    sigInt.sa_flags = 0;
    sigInt.sa_flags |= SA_RESTART;
    
    if (sigaction(SIGINT, &sigInt, 0)) {
        return 1;
    }
    return 0;
}
#else
bool isHubServer = false;

BOOL WINAPI windowsCtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
        case CTRL_C_EVENT:
            if (isHubServer) {
                UdpHubListener::sigIntHandler(0);
            } else {
                JackTrip::sigIntHandler(0);
            }
            return true;
        default:
            return false;
    }
}
#endif

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
    qInstallMessageHandler(qtMessageHandler);
    QScopedPointer<JackTrip> jackTrip;
    QScopedPointer<UdpHubListener> udpHub;

    bool testing = false;
    if ( argc > 1 ) {
        if ( !strcmp(argv[1], "test") ) {
            testing = true;
        }
    }

    if ( testing ) {
        std::cout << "=========TESTING=========" << std::endl;
        //main_tests(argc, argv); // test functions
        JackTrip jacktrip;
        //RtAudioInterface rtaudio(&jacktrip);
        //rtaudio.setup();
        //rtaudio.listAllInterfaces();
        //rtaudio.printDeviceInfo(0);

        //while (true) sleep(9999);
    }
    else {
        // catch all potential exeptions
        try
        {
            // Get Settings from user
            // ----------------------
            Settings settings;
            settings.parseInput(argc, argv);
            
            //Either start our hub server or our jacktrip process as appropriate.
            if (settings.isHubServer()) {
                udpHub.reset(settings.getConfiguredHubServer());
                if (gVerboseFlag) std::cout << "Settings:startJackTrip before udphub->start" << std::endl;
                QObject::connect(udpHub.data(), &UdpHubListener::signalStopped, &app, &QCoreApplication::quit, Qt::QueuedConnection);
                QObject::connect(udpHub.data(), &UdpHubListener::signalError, &app, &QCoreApplication::quit, Qt::QueuedConnection);
#if defined (__LINUX__) || (__MAC_OSX__)
                setupUnixSignalHandler(UdpHubListener::sigIntHandler);
#else
                isHubServer = true;
                SetConsoleCtrlHandler(windowsCtrlHandler, true);
#endif
                udpHub->start();
            } else {
                jackTrip.reset(settings.getConfiguredJackTrip());
                if (gVerboseFlag) std::cout << "Settings:startJackTrip before mJackTrip->startProcess" << std::endl;
                QObject::connect(jackTrip.data(), &JackTrip::signalProcessesStopped, &app, &QCoreApplication::quit, Qt::QueuedConnection);
                QObject::connect(jackTrip.data(), &JackTrip::signalError, &app, &QCoreApplication::quit, Qt::QueuedConnection);
#if defined (__LINUX__) || (__MAC_OSX__)
                setupUnixSignalHandler(JackTrip::sigIntHandler);
#else
                std::cout << SetConsoleCtrlHandler(windowsCtrlHandler, true) << std::endl;
#endif
#ifdef WAIRTOHUB // WAIR
                jackTrip->startProcess(0); // for WAIR compatibility, ID in jack client name
#else
                jackTrip->startProcess();
#endif // endwhere
            }
        
            if (gVerboseFlag) std::cout << "step 6" << std::endl;
            if (gVerboseFlag) std::cout << "jmain before app->exec()" << std::endl;
            
        }
        catch ( const std::exception & e )
        {
            std::cerr << "ERROR:" << std::endl;
            std::cerr << e.what() << std::endl;
            std::cerr << "Exiting JackTrip..." << std::endl;
            std::cerr << gPrintSeparator << std::endl;
            return -1;
        }
    }
    if (gVerboseFlag) std::cout << "step 6" << std::endl;
    if (gVerboseFlag) std::cout << "jacktrip_main before app.exec()" << std::endl;

    return app.exec();
}
