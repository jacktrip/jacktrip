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
 * \author Aaron Wyatt, based on jacktrip_main by Juan-Pablo Caceres
 * \date July 2020
 */

#ifndef __NO_GUI__
#include "gui/qjacktrip.h"
#include <QApplication>
#else
#include <QCoreApplication>
#endif
#include <QScopedPointer>
#include <iostream>
#include <signal.h>
#include "jacktrip_globals.h"
#include "Settings.h"
#include "UdpHubListener.h"
#include <QLoggingCategory>

QCoreApplication *createApplication(int &argc, char *argv[])
{
#ifndef __NO_GUI__
    //Find the name that the app was called with.
    //TODO: Consider replacing the C implementation below with one that uses QString or std::string.
    char* argv0 = strdup(argv[0]);
    char* command;
    char* token;
    
    token = strtok(argv0, "/");
    command = token;
    while (token != NULL) {
        token = strtok (NULL, "/");
        if (token != NULL) {
            command = token;
        }
    }
    
    //If we have command line arguments or have been called as jacktrip, run on the command line.
    if (argc == 1 && (strcmp(command, "jacktrip") != 0)) {
        free(argv0);
        return new QApplication(argc, argv);
    } else {
        free(argv0);
        return new QCoreApplication(argc, argv);
    }
#else
    return new QCoreApplication(argc, argv);
#endif
}

void qtMessageHandler([[maybe_unused]] QtMsgType type, [[maybe_unused]] const QMessageLogContext &context, const QString &msg)
{
    std::cerr << msg.toStdString() << std::endl;
}

#if defined (__LINUX__) || defined (__MAC_OSX__)
static int setupUnixSignalHandler(void (*handler)(int))
{
    //Setup our SIGINT handler.
    struct sigaction sigInt;
    sigInt.sa_handler = handler;
    sigemptyset(&sigInt.sa_mask);
    sigInt.sa_flags = 0;
    sigInt.sa_flags |= SA_RESTART;
    
    int result = 0;
    if (sigaction(SIGINT, &sigInt, 0)) {
        std::cout << "Unable to register SIGINT handler" << std::endl;
        result |= 1;
    }
    if (sigaction(SIGTERM, &sigInt, 0)) {
        std::cout << "Unable to register SIGTERM handler" << std::endl;
        result |= 2;
    }
    return result;
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

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QScopedPointer<JackTrip> jackTrip;
    QScopedPointer<UdpHubListener> udpHub;
#ifndef __NO_GUI__    
    QScopedPointer<QJackTrip> window;
    if (qobject_cast<QApplication *>(app.data())) {
        //Start the GUI if there are no command line options.
#ifdef __WIN_32__
        //Remove the console that appears if we're in windows.
        FreeConsole();
#endif // __WIN_32__
        app->setApplicationName("QJackTrip");
        window.reset(new QJackTrip);
        QObject::connect(window.data(), &QJackTrip::signalExit, app.data(), &QCoreApplication::quit, Qt::QueuedConnection);
        window->show();

    } else {
#endif // __NO_GUI__
        //Otherwise use the non-GUI version, and parse our command line.
        QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
        qInstallMessageHandler(qtMessageHandler);
        try {
            Settings settings;
            settings.parseInput(argc, argv);
            
            //Either start our hub server or our jacktrip process as appropriate.
            if (settings.isHubServer()) {
                udpHub.reset(settings.getConfiguredHubServer());
                if (gVerboseFlag) std::cout << "Settings:startJackTrip before udphub->start" << std::endl;
                QObject::connect(udpHub.data(), &UdpHubListener::signalStopped, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
                QObject::connect(udpHub.data(), &UdpHubListener::signalError, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
#if defined (__LINUX__) || defined (__MAC_OSX__)
                setupUnixSignalHandler(UdpHubListener::sigIntHandler);
#else
                isHubServer = true;
                SetConsoleCtrlHandler(windowsCtrlHandler, true);
#endif
                udpHub->start();
            } else {
                jackTrip.reset(settings.getConfiguredJackTrip());
                if (gVerboseFlag) std::cout << "Settings:startJackTrip before mJackTrip->startProcess" << std::endl;
                QObject::connect(jackTrip.data(), &JackTrip::signalProcessesStopped, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
                QObject::connect(jackTrip.data(), &JackTrip::signalError, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
#if defined (__LINUX__) || defined (__MAC_OSX__)
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
        } catch (const std::exception &e) {
            std::cerr << "ERROR:" << std::endl;
            std::cerr << e.what() << std::endl;
            std::cerr << "Exiting JackTrip..." << std::endl;
            std::cerr << gPrintSeparator << std::endl;
            return -1;
        }
#ifndef __NO_GUI__
    }
#endif // __NO_GUI__
    
    return app->exec();
}
