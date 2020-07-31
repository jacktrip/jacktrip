//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2020 Aaron Wyatt.
  
  This file is part of QJackTrip.

  QJackTrip is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  QJackTrip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with QJackTrip.  If not, see <https://www.gnu.org/licenses/>.
*/
//*****************************************************************

#include "qjacktrip.h"
#include <QApplication>
#include <QScopedPointer>
#include <iostream>
#include <signal.h>
#include "jacktrip_globals.h"
#include "Settings.h"
#include "UdpHubListener.h"
#include <QLoggingCategory>

QCoreApplication *createApplication(int &argc, char *argv[])
{
    if (argc == 1) {
        return new QApplication(argc, argv);
    } else {
        return new QCoreApplication(argc, argv);
    }
}

void qtMessageHandler(__attribute__((unused)) QtMsgType type, __attribute__((unused)) const QMessageLogContext &context, const QString &msg)
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

int main(int argc, char *argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QScopedPointer<QJackTrip> w;
    QScopedPointer<JackTrip> jackTrip;
    QScopedPointer<UdpHubListener> udpHub;
    
    if (qobject_cast<QApplication *>(app.data())) {
        //Start the GUI if there are no command line options.
#ifdef __WIN_32__
        //Remove the console that appears if we're in windows.
        FreeConsole();
#endif
        w.reset(new QJackTrip);
        QObject::connect(w.data(), &QJackTrip::signalExit, app.data(), &QCoreApplication::quit, Qt::QueuedConnection);
        w->show();
    } else {
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
                QObject::connect(jackTrip.data(), &JackTrip::signalProcessesStopped, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
                QObject::connect(jackTrip.data(), &JackTrip::signalError, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
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
        } catch (const std::exception &e) {
            std::cerr << "ERROR:" << std::endl;
            std::cerr << e.what() << std::endl;
            std::cerr << "Exiting JackTrip..." << std::endl;
            std::cerr << gPrintSeparator << std::endl;
            return -1;
        }
    }
    
    return app->exec();
}
