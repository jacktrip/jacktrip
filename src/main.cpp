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
 * \file main.cpp
 * \author Aaron Wyatt, based on jacktrip_main by Juan-Pablo Caceres
 * \date July 2020
 */

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QScopedPointer>
#include <csignal>
#include <iostream>

#include "Settings.h"
#include "UdpHubListener.h"
#include "jacktrip_globals.h"

#ifndef NO_GUI
#include "UserInterface.h"
#endif

#ifdef _WIN32
#include <psapi.h>
#include <tlhelp32.h>
#include <windows.h>
#endif

QCoreApplication* createApplication(int& argc, char* argv[])
{
#ifdef __APPLE__
    // Check for the DYLD_INSERT_LIBRARIES environment variable.
    // Refuse to run if it is set, to avoid code injection attacks.
    // Just an extra precaution since QtWebEngine requires the entitlement
    // com.apple.security.cs.allow-dyld-environment-variable
    // See https://doc.qt.io/qt-6/qtwebengine-deploying.html
    if (getenv("DYLD_INSERT_LIBRARIES") != nullptr) {
        std::cout << "Detected environment variable: DYLD_INSERT_LIBRARIES." << std::endl;
        std::cout << "To run JackTrip, please omit the this environment variable."
                  << std::endl;
        std::exit(1);
    }
#endif

    // Check for some specific, GUI related command line options.
    bool forceGui = false;
    bool testGui  = false;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--gui", 5) == 0 || strncmp(argv[i], "--deeplink", 10) == 0
            || strncmp(argv[i], "--classic-gui", 13) == 0
            || strncmp(argv[i], "jacktrip://", 11) == 0) {
            forceGui = true;
        } else if (strncmp(argv[i], "--test-gui", 10) == 0) {
            // Command line option to test if the binary has been built with GUI support.
            // Exits immediately. Exits with an error if GUI support has not been built
            // in.
            testGui = true;
        }
    }

#ifdef NO_GUI
    if (testGui) {
        std::cout << "This version of JackTrip has been built without GUI support."
                  << std::endl;
        std::cout << "(To run JackTrip normally, please omit the --test-gui option.)"
                  << std::endl;
        std::exit(1);
    }
    if (forceGui) {
        std::cout << "This version of jacktrip has not been built with GUI support."
                  << std::endl;
        std::exit(1);
    }
#else
    if (testGui) {
        std::cout << "This version of JackTrip has been built with GUI support."
                  << std::endl;
        std::cout << "(To run JackTrip normally, please omit the --test-gui option.)"
                  << std::endl;
        std::exit(0);
    }
    if (forceGui || argc == 1) {
        return UserInterface::createApplication(argc, argv);
    }
#endif

    return new QCoreApplication(argc, argv);
}

void outputError(const QString& msg)
{
    std::cerr << "Error: " << msg.toStdString() << std::endl;
}

#ifndef _WIN32
static int setupUnixSignalHandler(void (*handler)(int))
{
    // Setup our SIGINT handler.
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

int main(int argc, char* argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QScopedPointer<JackTrip> jackTrip;
    QScopedPointer<UdpHubListener> udpHub;
    QSharedPointer<Settings> cliSettings;

#ifndef NO_GUI
    QScopedPointer<UserInterface> interface;
    QApplication* guiApp = dynamic_cast<QApplication*>(app.data());
    if (guiApp != nullptr) {
        // Start the GUI
        cliSettings.reset(new Settings(true));
        cliSettings->parseInput(argc, argv);
        interface.reset(new UserInterface(cliSettings));
        interface->start(guiApp);
        return app->exec();
    }
#endif  // NO_GUI

    // Otherwise use the non-GUI version, and parse our command line.
    try {
        cliSettings.reset(new Settings(false));
        cliSettings->parseInput(argc, argv);

#ifndef PSI
        if (gVerboseFlag) {
            QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
        }
#endif

        // Either start our hub server or our jacktrip process as appropriate.
        if (cliSettings->isHubServer()) {
            udpHub.reset(cliSettings->getConfiguredHubServer());
            if (gVerboseFlag)
                std::cout << "Settings:startJackTrip before udphub->start" << std::endl;
            QObject::connect(udpHub.data(), &UdpHubListener::signalStopped, app.data(),
                             &QCoreApplication::quit, Qt::QueuedConnection);
            QObject::connect(udpHub.data(), &UdpHubListener::signalError, outputError);
            QObject::connect(udpHub.data(), &UdpHubListener::signalError, app.data(),
                             &QCoreApplication::quit, Qt::QueuedConnection);
#ifndef _WIN32
            setupUnixSignalHandler(UdpHubListener::sigIntHandler);
#else
            isHubServer = true;
            SetConsoleCtrlHandler(windowsCtrlHandler, true);
#endif  // _WIN32
            udpHub->start();
        } else {
            jackTrip.reset(cliSettings->getConfiguredJackTrip());
            if (gVerboseFlag)
                std::cout << "Settings:startJackTrip before mJackTrip->startProcess"
                          << std::endl;
            QObject::connect(jackTrip.data(), &JackTrip::signalProcessesStopped,
                             app.data(), &QCoreApplication::quit, Qt::QueuedConnection);
            QObject::connect(jackTrip.data(), &JackTrip::signalError, outputError);
            QObject::connect(jackTrip.data(), &JackTrip::signalError, app.data(),
                             &QCoreApplication::quit, Qt::QueuedConnection);

#ifndef _WIN32
            setupUnixSignalHandler(JackTrip::sigIntHandler);
#else
            std::cout << SetConsoleCtrlHandler(windowsCtrlHandler, true) << std::endl;
#endif  // _WIN32

#ifdef WAIRTOHUB                        // WAIR
            jackTrip->startProcess(0);  // for WAIR compatibility, ID in jack client name
#else
            jackTrip->startProcess();
#endif  // endwhere
        }

        if (gVerboseFlag)
            std::cout << "step 6" << std::endl;
        if (gVerboseFlag)
            std::cout << "jmain before app->exec()" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR:" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "Exiting JackTrip..." << std::endl;
        std::cerr << gPrintSeparator << std::endl;
        return -1;
    }

    return app->exec();
}
