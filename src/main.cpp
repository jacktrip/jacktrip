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

#ifndef NO_GUI
#include <QApplication>
#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QQuickStyle>
#endif

#ifndef NO_UPDATER
#include "dblsqd/feed.h"
#include "dblsqd/update_dialog.h"
#endif

#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QQmlEngine>
#include <QQuickView>
#include <QSGRendererInterface>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
// TODO: Add support for QtWebView
//#include <QtWebView>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

#include "JTApplication.h"
#include "gui/virtualstudio.h"
#include "gui/vsDeeplink.h"
#include "gui/vsQmlClipboard.h"
#endif  // NO_VS && QT_VERSION

#include "gui/qjacktrip.h"
#else
#include <QCoreApplication>
#endif
#include <QLoggingCategory>
#include <QScopedPointer>
#include <csignal>
#include <iostream>

#include "Settings.h"
#include "UdpHubListener.h"
#include "jacktrip_globals.h"

#ifdef _WIN32
#include <psapi.h>
#include <tlhelp32.h>
#include <windows.h>
#endif

#ifndef NO_GUI
#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
static QTextStream* ts;
static QFile outFile;
#endif  // NO_VS && QT_VERSION
#endif  // NO_GUI

QCoreApplication* createApplication(int& argc, char* argv[])
{
    // Check for some specific, GUI related command line options.
    bool forceGui = false;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--gui", 5) == 0 || strncmp(argv[i], "--deeplink", 10) == 0
            || strncmp(argv[i], "--classic-gui", 13) == 0
            || strncmp(argv[i], "jacktrip://", 11) == 0) {
            forceGui = true;
        } else if (strncmp(argv[i], "--test-gui", 10) == 0) {
            // Command line option to test if the binary has been built with GUI support.
            // Exits immediately. Exits with an error if GUI support has not been built
            // in.
#ifdef NO_GUI
            std::cout << "This version of JackTrip has been built without GUI support."
                      << std::endl;
            std::cout << "(To run JackTrip normally, please omit the --test-gui option.)"
                      << std::endl;
            std::exit(1);
#else
            std::cout << "This version of JackTrip has been built with GUI support."
                      << std::endl;
            std::cout << "(To run JackTrip normally, please omit the --test-gui option.)"
                      << std::endl;
            std::exit(0);
#endif
        }
    }

    // If we have command line arguments and aren't forcing the GUI run on the command
    // line.
    if (argc == 1 || forceGui) {
#ifdef NO_GUI
        if (forceGui) {
            std::cout << "This version of jacktrip has not been built with GUI support."
                      << std::endl;
            std::exit(1);
        } else {
            return new QCoreApplication(argc, argv);
        }
#else
#if defined(__unix__)
        // Check if X or Wayland environment variables are set.
        if (std::getenv("WAYLAND_DISPLAY") == nullptr
            && std::getenv("DISPLAY") == nullptr) {
            std::cout << "ERROR: Display not found. Make sure X or Wayland is running or "
                         "try running jacktrip in command line mode."
                      << std::endl;
            std::cout << "(To display a list of command line options run \"jacktrip -h\")"
                      << std::endl;
            std::exit(1);
        }
#endif
#if defined(Q_OS_MACOS) && !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Turn on high DPI support.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        JTApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        // Fix for display scaling like 125% or 150% on Windows
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif  // QT_VERSION

        // Initialize webengine
        QtWebEngineQuick::initialize();
        // TODO: Add support for QtWebView
        // qputenv("QT_WEBVIEW_PLUGIN", "native");
        // QtWebView::initialize();

        return new JTApplication(argc, argv);
#else
        // Turn on high DPI support.
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
        // Fix for display scaling like 125% or 150% on Windows
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
#if defined(NO_VS) && defined(_WIN32)
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::RoundPreferFloor);
#else
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif  // NO_VS
#endif  // QT_VERSION

#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Enables resource sharing between the OpenGL contexts
        QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
        // QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

        // QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
        QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

        // Initialize webengine
        QtWebEngineQuick::initialize();
        // TODO: Add support for QtWebView
        // qputenv("QT_WEBVIEW_PLUGIN", "native");
        // QtWebView::initialize();
#endif

        return new QApplication(argc, argv);
#endif  // Q_OS_MACOS
#endif  // NO_GUI
    } else {
        return new QCoreApplication(argc, argv);
    }
}

void qtMessageHandler([[maybe_unused]] QtMsgType type,
                      [[maybe_unused]] const QMessageLogContext& context,
                      const QString& msg)
{
    std::cerr << msg.toStdString() << std::endl;
#ifndef NO_GUI
#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Writes to file in order to debug bundles and executables
    *ts << msg << Qt::endl;
#endif  // NO_VS && QT_VERSION
#endif  // NO_GUI
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

bool isRunFromCmd()
{
    // Get our parent process pid
    HANDLE h = NULL;
    PROCESSENTRY32 pe;
    ZeroMemory(&pe, sizeof(PROCESSENTRY32));
    DWORD pid = GetCurrentProcessId();
    DWORD ppid = 0;
    pe.dwSize = sizeof(PROCESSENTRY32);
    h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (Process32First(h, &pe)) {
        do {
            // Loop through the list of processes until we find ours.
            if (pe.th32ProcessID == pid) {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while (Process32Next(h, &pe));
    }
    CloseHandle(h);

    // Get the name of our parent process;
    char pname[MAX_PATH] = {0};
    DWORD size = MAX_PATH;
    h = NULL;
    h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ppid);
    if (h) {
        if (QueryFullProcessImageNameA(h, 0, pname, &size)) {
            CloseHandle(h);

            // Check if our parent process is a command line.
            if (size >= 14 && strncmp(pname + size - 14, "powershell.exe", 14) == 0) {
                return true;
            }
            if (size >= 7 && strncmp(pname + size - 7, "cmd.exe", 7) == 0) {
                return true;
            }
            if (size >= 6 && strncmp(pname + size - 6, "wt.exe", 6) == 0) {
                return true;
            }
            // a few extras for msys/cygwin/etc
            if (size >= 8 && strncmp(pname + size - 8, "bash.exe", 8) == 0) {
                return true;
            }
            if (size >= 6 && strncmp(pname + size - 6, "sh.exe", 6) == 0) {
                return true;
            }
            if (size >= 7 && strncmp(pname + size - 7, "zsh.exe", 7) == 0) {
                return true;
            }
        } else {
            CloseHandle(h);
        }
    }

    return false;
}
#endif

int main(int argc, char* argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QScopedPointer<JackTrip> jackTrip;
    QScopedPointer<UdpHubListener> udpHub;
#ifndef NO_GUI
    QSharedPointer<QJackTrip> window;
#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QQuickStyle::setStyle("Basic");
#endif  // QT_VERSION

#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QSharedPointer<VirtualStudio> vsPtr;
    QScopedPointer<VsDeeplink> vsDeeplinkPtr;
#endif  // NO_VS && QT_VERSION

#if defined(Q_OS_MACOS) && !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (qobject_cast<JTApplication*>(app.data())) {
#else
    if (qobject_cast<QApplication*>(app.data())) {
#endif
        // Start the GUI if there are no command line options.
#ifdef _WIN32
        // Remove the console that appears if we're on windows and not running from a
        // command line.
        if (!isRunFromCmd()) {
            std::cout << "This extra window is caused by a bug in Microsoft Windows. "
                      << "It can safely be ignored or closed." << std::endl
                      << std::endl
                      << "To fix this bug, please upgrade to the latest version of "
                      << "Windows Terminal available in the Microsoft App Store:"
                      << std::endl
                      << "https://aka.ms/terminal" << std::endl;

            FreeConsole();
        }
#endif  // _WIN32
        app->setOrganizationName(QStringLiteral("jacktrip"));
        app->setOrganizationDomain(QStringLiteral("jacktrip.org"));
        app->setApplicationName(QStringLiteral("JackTrip"));
        app->setApplicationVersion(gVersion);

        QSharedPointer<Settings> cliSettings;
        cliSettings.reset(new Settings(true));
        cliSettings->parseInput(argc, argv);

#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        // Register clipboard Qml type
        qmlRegisterType<VsQmlClipboard>("VS", 1, 0, "Clipboard");

        // prepare handler for deeplinks jacktrip://join/<StudioID>
        vsDeeplinkPtr.reset(new VsDeeplink(cliSettings->getDeeplink()));
        if (!vsDeeplinkPtr->getDeeplink().isEmpty()) {
            bool readyForExit = vsDeeplinkPtr->waitForReady();
            if (readyForExit)
                return 0;
        }

        // Check which mode we are running in
        QSettings settings;
        int uiMode = QJackTrip::UNSET;
        if (!vsDeeplinkPtr->getDeeplink().isEmpty()) {
            uiMode = QJackTrip::VIRTUAL_STUDIO;
        } else if (cliSettings->guiForceClassicMode()) {
            uiMode = QJackTrip::STANDARD;
            // force settings change; otherwise, virtual studio
            // window will still be displayed
            settings.setValue(QStringLiteral("UiMode"), uiMode);
        } else {
            uiMode = settings.value(QStringLiteral("UiMode"), QJackTrip::UNSET).toInt();
        }

        window.reset(new QJackTrip(cliSettings, !vsDeeplinkPtr->getDeeplink().isEmpty()));
#else
        window.reset(new QJackTrip(cliSettings));
#endif  // NO_VS
        QObject::connect(window.data(), &QJackTrip::signalExit, app.data(),
                         &QCoreApplication::quit, Qt::QueuedConnection);

#if !defined(NO_VS) && QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        vsPtr.reset(new VirtualStudio(uiMode == QJackTrip::UNSET));
        QObject::connect(vsPtr.data(), &VirtualStudio::signalExit, app.data(),
                         &QCoreApplication::quit, Qt::QueuedConnection);
        vsPtr->setStandardWindow(window);
        vsPtr->setCLISettings(cliSettings);
        window->setVs(vsPtr);
        QObject::connect(vsDeeplinkPtr.get(), &VsDeeplink::signalDeeplink, vsPtr.get(),
                         &VirtualStudio::handleDeeplinkRequest, Qt::QueuedConnection);
        vsDeeplinkPtr->readyForSignals();

        if (uiMode == QJackTrip::UNSET) {
            vsPtr->show();
        } else if (uiMode == QJackTrip::VIRTUAL_STUDIO) {
            vsPtr->show();
        } else {
            window->show();
        }

        // Log to file
        QString logPath(
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QDir logDir;
        if (!logDir.exists(logPath)) {
            logDir.mkpath(logPath);
        }
        QString fileLoc(logPath.append("/log.txt"));
        qDebug() << "Log file location:" << fileLoc;
        outFile.setFileName(fileLoc);
        if (!outFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            qDebug() << "Log file open failed:" << outFile.errorString();
        }
        ts = new QTextStream(&outFile);
        qInstallMessageHandler(qtMessageHandler);
#else
        window->show();
#endif  // NO_VS

#if !defined(NO_UPDATER) && !defined(__unix__)
#ifndef PSI
#if defined(NO_VS) || QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

        // This wasn't set up earlier in NO_VS builds. Create it here.
        QSettings settings;
#endif
        QString updateChannel = settings.value(QStringLiteral("UpdateChannel"), "stable")
                                    .toString()
                                    .toLower();
        QString baseUrl = QStringLiteral("https://files.jacktrip.org/app-releases/%1")
                              .arg(updateChannel);
#else
        QString baseUrl = QStringLiteral("https://nuages.psi-borg.org/jacktrip");
#endif  // PSI
        // Setup auto-update feed
        dblsqd::Feed* feed = new dblsqd::Feed();
#ifdef Q_OS_WIN
        feed->setUrl(QUrl(QString("%1/%2-manifests.json").arg(baseUrl, "win")));
#endif
#ifdef Q_OS_MACOS
        feed->setUrl(QUrl(QString("%1/%2-manifests.json").arg(baseUrl, "mac")));
#endif
        if (feed) {
            dblsqd::UpdateDialog* updateDialog = new dblsqd::UpdateDialog(feed);
            updateDialog->setIcon(":/qjacktrip/icon.png");
        }
#endif  // NO_UPDATER
    } else {
#endif  // NO_GUI
        // Otherwise use the non-GUI version, and parse our command line.
#ifndef PSI
        QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
#endif
        try {
            Settings settings;
            settings.parseInput(argc, argv);

            // Either start our hub server or our jacktrip process as appropriate.
            if (settings.isHubServer()) {
                udpHub.reset(settings.getConfiguredHubServer());
                if (gVerboseFlag)
                    std::cout << "Settings:startJackTrip before udphub->start"
                              << std::endl;
                QObject::connect(udpHub.data(), &UdpHubListener::signalStopped,
                                 app.data(), &QCoreApplication::quit,
                                 Qt::QueuedConnection);
                QObject::connect(udpHub.data(), &UdpHubListener::signalError, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
#ifndef _WIN32
                setupUnixSignalHandler(UdpHubListener::sigIntHandler);
#else
            isHubServer = true;
            SetConsoleCtrlHandler(windowsCtrlHandler, true);
#endif
                udpHub->start();
            } else {
                jackTrip.reset(settings.getConfiguredJackTrip());
                if (gVerboseFlag)
                    std::cout << "Settings:startJackTrip before mJackTrip->startProcess"
                              << std::endl;
                QObject::connect(jackTrip.data(), &JackTrip::signalProcessesStopped,
                                 app.data(), &QCoreApplication::quit,
                                 Qt::QueuedConnection);
                QObject::connect(jackTrip.data(), &JackTrip::signalError, app.data(),
                                 &QCoreApplication::quit, Qt::QueuedConnection);
#ifndef _WIN32
                setupUnixSignalHandler(JackTrip::sigIntHandler);
#else
            std::cout << SetConsoleCtrlHandler(windowsCtrlHandler, true) << std::endl;
#endif
#ifdef WAIRTOHUB  // WAIR
                jackTrip->startProcess(
                    0);  // for WAIR compatibility, ID in jack client name
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
#ifndef NO_GUI
    }
#endif  // NO_GUI

    return app->exec();
}
