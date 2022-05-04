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
 * \file virtualstudio.h
 * \author Aaron Wyatt
 * \date March 2022
 */

#ifndef VIRTUALSTUDIO_H
#define VIRTUALSTUDIO_H

#include <QList>
#include <QMutex>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTimer>
#include <QtNetworkAuth>

#include "../JackTrip.h"
#include "vsQuickView.h"
#include "vsServerInfo.h"

#ifdef __APPLE__
#include "NoNap.h"
#endif

class QJackTrip;

class VirtualStudio : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showFirstRun READ showFirstRun NOTIFY showFirstRunChanged)
    Q_PROPERTY(bool hasRefreshToken READ hasRefreshToken NOTIFY hasRefreshTokenChanged)
    Q_PROPERTY(QString versionString READ versionString CONSTANT)
    Q_PROPERTY(QString logoSection READ logoSection NOTIFY logoSectionChanged)
    Q_PROPERTY(bool selectableBackend READ selectableBackend CONSTANT)
    Q_PROPERTY(QString audioBackend READ audioBackend WRITE setAudioBackend NOTIFY
                   audioBackendChanged)
    Q_PROPERTY(
        int inputDevice READ inputDevice WRITE setInputDevice NOTIFY inputDeviceChanged)
    Q_PROPERTY(int outputDevice READ outputDevice WRITE setOutputDevice NOTIFY
                   outputDeviceChanged)
    Q_PROPERTY(
        int bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged)
    Q_PROPERTY(int currentStudio READ currentStudio NOTIFY currentStudioChanged)
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(float fontScale READ fontScale CONSTANT)
    Q_PROPERTY(float uiScale READ uiScale WRITE setUiScale NOTIFY uiScaleChanged)

    Q_PROPERTY(bool psiBuild READ psiBuild CONSTANT)

   public:
    explicit VirtualStudio(bool firstRun = false, QObject* parent = nullptr);
    ~VirtualStudio() override;

    void setStandardWindow(QSharedPointer<QJackTrip> window);
    void show();

    bool showFirstRun();
    bool hasRefreshToken();
    QString versionString();
    QString logoSection();
    bool selectableBackend();
    QString audioBackend();
    void setAudioBackend(const QString& backend);
    int inputDevice();
    void setInputDevice(int device);
    int outputDevice();
    void setOutputDevice(int device);
    int bufferSize();
    void setBufferSize(int index);
    int currentStudio();
    QString connectionState();
    float fontScale();
    float uiScale();
    void setUiScale(float scale);
    bool psiBuild();

   public slots:
    void toStandard();
    void toVirtualStudio();
    void login();
    void logout();
    void refreshStudios(int index);
    void refreshDevices();
    void revertSettings();
    void applySettings();
    void connectToStudio(int studioIndex);
    void completeConnection();
    void disconnect();
    void manageStudio(int studioIndex);
    void showAbout();
    void exit();

   signals:
    void authSucceeded();
    void authFailed();
    void connected();
    void disconnected();
    void refreshFinished(int index);
    void showFirstRunChanged();
    void hasRefreshTokenChanged();
    void logoSectionChanged();
    void audioBackendChanged();
    void inputDeviceChanged();
    void outputDeviceChanged();
    void bufferSizeChanged();
    void currentStudioChanged();
    void connectionStateChanged();
    void uiScaleChanged();
    void newScale();
    void signalExit();
    void periodicRefresh();

   private slots:
    void slotAuthSucceded();
    void slotAuthFailed();
    void processFinished();
    void processError(const QString& errorMessage);
    void receivedConnectionFromPeer();
    void checkForHostname();
    void endRetryPeriod();

   private:
    void setupAuthenticator();
    void getServerList(bool firstLoad = false, int index = -1);
    void getUserId();
    void getSubscriptions();
#ifdef RT_AUDIO
    void getDeviceList(QStringList* list, bool isInput);
#endif
    void stopStudio();

    bool m_showFirstRun = false;
    bool m_checkSsl     = true;
    QString m_refreshToken;
    QString m_userId;
    VsQuickView m_view;
    QSharedPointer<QJackTrip> m_standardWindow;
    QScopedPointer<QOAuth2AuthorizationCodeFlow> m_authenticator;

    QList<QObject*> m_servers;
    QStringList m_subscribedServers;
    QString m_logoSection     = QStringLiteral("Your Studios");
    bool m_selectableBackend  = true;
    bool m_useRtAudio         = false;
    int m_currentStudio       = 0;
    QString m_connectionState = QStringLiteral("Connecting...");
    QScopedPointer<JackTrip> m_jackTrip;
    QTimer m_startTimer;
    QTimer m_retryPeriodTimer;
    bool m_startedStudio = false;
    bool m_retryPeriod;
    bool m_jackTripRunning = false;

    QTimer m_refreshTimer;
    QMutex m_refreshMutex;
    bool m_allowRefresh = true;

    bool m_onConnectedScreen = false;
    bool m_isExiting         = false;
    float m_fontScale        = 1;
    float m_uiScale;
    float m_previousUiScale;

#ifdef RT_AUDIO
    QStringList m_inputDeviceList;
    QStringList m_outputDeviceList;
    QString m_inputDevice;
    QString m_outputDevice;
    quint16 m_bufferSize;
    QString m_previousInput;
    QString m_previousOutput;
    quint16 m_previousBuffer;
    bool m_previousUseRtAudio = false;
#endif
    QStringList m_bufferOptions = {"16", "32", "64", "128", "256", "512", "1024"};

#ifdef __APPLE__
    NoNap m_noNap;
#endif
};

#endif  // VIRTUALSTUDIO_H
