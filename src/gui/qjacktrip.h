//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

//*****************************************************************

#ifndef QJACKTRIP_H
#define QJACKTRIP_H

#include <QByteArray>
#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QScopedPointer>
#include <QString>
#include <QTemporaryFile>

#include "../JackTrip.h"
#include "../UdpHubListener.h"
#include "messageDialog.h"

#ifdef __MAC_OSX__
#include "NoNap.h"
#endif

#ifdef __RT_AUDIO__
#include <QComboBox>
#endif

namespace Ui
{
class QJackTrip;
}

class QJackTrip : public QMainWindow
{
    Q_OBJECT

   public:
    explicit QJackTrip(QWidget* parent = nullptr);
    ~QJackTrip() override;

    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

    void setArgc(int argc);

   signals:
    void signalExit();

   private slots:
    void processFinished();
    void processError(const QString& errorMessage);
    void receivedConnectionFromPeer();
    void udpWaitingTooLong();
    void queueLengthChanged(int queueLength);
    void chooseRunType(int index);
    void addressChanged(const QString& address);
    void authFilesChanged();
    void credentialsChanged();
    void browseForFile();
    void receivedIP(QNetworkReply* reply);
    void resetOptions();
    void start();
    void stop();
    void exit();

   private:
    enum runTypeT { P2P_CLIENT, P2P_SERVER, HUB_CLIENT, HUB_SERVER };

    int findTab(const QString& tabName);
    void enableUi(bool enabled);
    void advancedOptionsForHubServer(bool isHubServer);
    void migrateSettings();
    void loadSettings();
    void saveSettings();

#ifdef __RT_AUDIO__
    void populateDeviceMenu(QComboBox* menu, bool isInput);
#endif

    void setupStatsWindow();
    void appendPlugins(JackTrip* jackTrip, int numSendChannels, int numRecvChannels);

    QString commandLineFromCurrentOptions();
    void showCommandLineMessageBox();

    QScopedPointer<Ui::QJackTrip> m_ui;
    QScopedPointer<UdpHubListener> m_udpHub;
    QScopedPointer<JackTrip> m_jackTrip;
    QScopedPointer<QNetworkAccessManager> m_netManager;
    QScopedPointer<MessageDialog> m_messageDialog;
    QSharedPointer<QTemporaryFile> m_ioStatsOutput;
    bool m_jackTripRunning;
    bool m_isExiting;

    QMutex m_requestMutex;
    QString m_IPv6Address;
    bool m_hasIPv4Reply;
    QString m_lastPath;

    QLabel m_autoQueueIndicator;
    int m_argc;
    bool m_hideWarning;

#ifdef __MAC_OSX__
    NoNap m_noNap;
#endif
};

#endif  // QJACKTRIP_H
