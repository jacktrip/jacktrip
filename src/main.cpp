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
#include <QCommandLineParser>

#ifndef NO_UPDATER
#include "dblsqd/feed.h"
#include "dblsqd/update_dialog.h"
#endif

#ifndef NO_VS
#include <QDebug>
#include <QFile>
#include <QLocalServer>
#include <QLocalSocket>
#include <QQmlEngine>
#include <QQuickView>
#include <QSettings>
#include <QTextStream>

#include "JTApplication.h"
#include "gui/virtualstudio.h"
#include "gui/vsQmlClipboard.h"
#include "gui/vsUrlHandler.h"
#endif

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
#ifndef NO_VS
static QTextStream* ts;
static QFile outFile;
#endif  // NO_VS
#endif  // NO_GUI

QCoreApplication* createApplication(int& argc, char* argv[])
{
    // Check for some specific, GUI related command line options.
    bool forceGui = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--gui") == 0) {
            forceGui = true;
        } else if (strcmp(argv[i], "--test-gui") == 0) {
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
#if defined(Q_OS_MACOS) && !defined(NO_VS)
        // Turn on high DPI support.
        JTApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        // Fix for display scaling like 125% or 150% on Windows
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif  // QT_VERSION
        return new JTApplication(argc, argv);
#else
        // Turn on high DPI support.
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        // Fix for display scaling like 125% or 150% on Windows
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif  // QT_VERSION
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
#ifndef NO_VS
    // Writes to file in order to debug bundles and executables
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    *ts << msg << Qt::endl;
#else
    *ts << msg << endl;
#endif  // QT_VERSION > 5.14.0
#endif  // NO_VS
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

#ifndef NO_VS
    QString deeplink = QLatin1String("");
    QSharedPointer<VirtualStudio> vs;
#ifdef _WIN32
    QSharedPointer<QLocalServer> instanceServer;
    QSharedPointer<QLocalSocket> instanceCheckSocket;
#endif
#endif

#if defined(Q_OS_MACOS) && !defined(NO_VS)
    if (qobject_cast<JTApplication*>(app.data())) {
#else
    if (qobject_cast<QApplication*>(app.data())) {
#endif
        // Start the GUI if there are no command line options.
#ifdef _WIN32
        // Remove the console that appears if we're on windows and not running from a
        // command line.
        if (!isRunFromCmd()) {
            FreeConsole();
        }
#endif  // _WIN32
        app->setOrganizationName(QStringLiteral("jacktrip"));
        app->setOrganizationDomain(QStringLiteral("jacktrip.org"));
        app->setApplicationName(QStringLiteral("JackTrip"));
        app->setApplicationVersion(gVersion);

        QCommandLineParser parser;
        QCommandLineOption verboseOption(QStringList() << QStringLiteral("V")
                                                       << QStringLiteral("verbose"));
        parser.addOption(verboseOption);
        parser.parse(app->arguments());
        if (parser.isSet(verboseOption)) {
            gVerboseFlag = true;
        }

#ifndef NO_VS
        // Register clipboard Qml type
        qmlRegisterType<VsQmlClipboard>("VS", 1, 0, "Clipboard");

        // Parse command line for deep link
        QCommandLineOption deeplinkOption(QStringList() << QStringLiteral("deeplink"));
        deeplinkOption.setValueName(QStringLiteral("deeplink"));
        parser.addOption(deeplinkOption);
        parser.parse(app->arguments());
        if (parser.isSet(deeplinkOption)) {
            deeplink = parser.value(deeplinkOption);
        }

        // Check if we need to show our first run window.
        QSettings settings;
        int uiMode = settings.value(QStringLiteral("UiMode"), QJackTrip::UNSET).toInt();
#ifndef __unix__
        QString updateChannel = settings.value(QStringLiteral("UpdateChannel"), "stable")
                                    .toString()
                                    .toLower();
#endif
#ifdef _WIN32
        // Set url scheme in registry
        QString path = QDir::toNativeSeparators(qApp->applicationFilePath());

        QSettings set("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
        set.beginGroup("jacktrip");
        set.setValue("Default", "URL:JackTrip Protocol");
        set.setValue("DefaultIcon/Default", path);
        set.setValue("URL Protocol", "");
        set.setValue("shell/open/command/Default",
                     QString("\"%1\"").arg(path) + " --gui --deeplink \"%1\"");
        set.endGroup();

        // Create socket
        instanceCheckSocket =
            QSharedPointer<QLocalSocket>::create(new QLocalSocket(app.data()));
        // End process if instance exists
        QObject::connect(
            instanceCheckSocket.data(), &QLocalSocket::connected, app.data(),
            [&]() {
                // pass deeplink to existing instance before quitting
                if (!deeplink.isEmpty()) {
                    QByteArray baDeeplink = deeplink.toLocal8Bit();
                    qint64 writeBytes     = instanceCheckSocket->write(baDeeplink);
                    instanceCheckSocket->flush();
                    instanceCheckSocket->disconnectFromServer();  // remove next

                    if (writeBytes < 0) {
                        qDebug() << "sending deeplink failed";
                    }
                }
                emit QCoreApplication::quit();
            },
            Qt::QueuedConnection);
        // Create instanceServer to prevent new instances from being created
        void (QLocalSocket::*errorFunc)(QLocalSocket::LocalSocketError);
#ifdef Q_OS_LINUX
        errorFunc = &QLocalSocket::error;
#else
        errorFunc = &QLocalSocket::errorOccurred;
#endif
        QObject::connect(
            instanceCheckSocket.data(), errorFunc, app.data(),
            [&](QLocalSocket::LocalSocketError socketError) {
                switch (socketError) {
                case QLocalSocket::ServerNotFoundError:
                case QLocalSocket::SocketTimeoutError:
                case QLocalSocket::ConnectionRefusedError:
                    instanceServer = QSharedPointer<QLocalServer>::create(
                        new QLocalServer(app.data()));
                    instanceServer->setSocketOptions(QLocalServer::WorldAccessOption);
                    instanceServer->listen("jacktripExists");
                    QObject::connect(
                        instanceServer.data(), &QLocalServer::newConnection, app.data(),
                        [&]() {
                            // This is the first instance. Bring it to the
                            // top.
                            vs->raiseToTop();
                            while (instanceServer->hasPendingConnections()) {
                                // Receive URL from 2nd instance
                                QLocalSocket* connectedSocket =
                                    instanceServer->nextPendingConnection();

                                if (!connectedSocket->waitForConnected()) {
                                    qDebug() << "Never received connection";
                                    return;
                                }

                                if (!connectedSocket->waitForReadyRead()) {
                                    qDebug() << "Never ready to read";
                                    return;
                                }

                                if (connectedSocket->bytesAvailable()
                                    < (int)sizeof(quint16)) {
                                    qDebug() << "no bytes available";
                                    break;
                                }

                                QByteArray in(connectedSocket->readAll());
                                QString urlString(in);
                                QUrl url(urlString);

                                // Join studio using received URL
                                if (url.scheme() == "jacktrip" && url.host() == "join") {
                                    vs->setStudioToJoin(url);
                                }
                            }
                        },
                        Qt::QueuedConnection);
                    break;
                case QLocalSocket::PeerClosedError:
                    break;
                default:
                    qDebug() << instanceCheckSocket->errorString();
                }
            });
        // Check for existing instance
        instanceCheckSocket->connectToServer("jacktripExists");

#endif  // _WIN32
        window.reset(new QJackTrip(argc, !deeplink.isEmpty()));
#else
        window.reset(new QJackTrip(argc));
#endif  // NO_VS
        QObject::connect(window.data(), &QJackTrip::signalExit, app.data(),
                         &QCoreApplication::quit, Qt::QueuedConnection);
#ifndef NO_VS
        vs.reset(new VirtualStudio(uiMode == QJackTrip::UNSET));
        QObject::connect(vs.data(), &VirtualStudio::signalExit, app.data(),
                         &QCoreApplication::quit, Qt::QueuedConnection);
        vs->setStandardWindow(window);
        window->setVs(vs);

        VsUrlHandler* m_urlHandler = new VsUrlHandler();
        QDesktopServices::setUrlHandler(QStringLiteral("jacktrip"), m_urlHandler,
                                        "handleUrl");
        QObject::connect(m_urlHandler, &VsUrlHandler::joinUrlClicked, vs.data(),
                         [&](const QUrl& url) {
                             if (url.scheme() == QLatin1String("jacktrip")
                                 && url.host() == QLatin1String("join")) {
                                 vs->setStudioToJoin(url);
                             }
                         });
        // Open with any command line-passed url
        QDesktopServices::openUrl(QUrl(deeplink));

        if (uiMode == QJackTrip::UNSET) {
            vs->show();
        } else if (uiMode == QJackTrip::VIRTUAL_STUDIO) {
            vs->show();
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

#ifndef NO_UPDATER
        // Setup auto-update feed
        dblsqd::Feed* feed = 0;
        QString baseUrl =
            "https://raw.githubusercontent.com/jacktrip/jacktrip/dev/releases";
#ifdef Q_OS_WIN
        feed = new dblsqd::Feed();
        feed->setUrl(
            QUrl(QString("%1/%2/%3-manifests.json").arg(baseUrl, updateChannel, "win")));
#endif
#ifdef Q_OS_MACOS
        feed = new dblsqd::Feed();
        feed->setUrl(
            QUrl(QString("%1/%2/%3-manifests.json").arg(baseUrl, updateChannel, "mac")));
#endif
        if (feed) {
            dblsqd::UpdateDialog* updateDialog = new dblsqd::UpdateDialog(feed);
            updateDialog->setIcon(":/qjacktrip/icon.png");
        }
#endif  // NO_UPDATER
    } else {
#endif  // NO_GUI
        // Otherwise use the non-GUI version, and parse our command line.
        QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
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
