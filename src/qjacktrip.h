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

#ifndef QJACKTRIP_H
#define QJACKTRIP_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMutex>
#include <QByteArray>
#include <QString>
#include <QTemporaryFile>
#include <QLabel>
#include "UdpHubListener.h"
#include "JackTrip.h"
#include "messageDialog.h"

#ifdef __MAC_OSX__
#include "NoNap.h"
#endif

namespace Ui {
class QJackTrip;
}

class QJackTrip : public QMainWindow
{
    Q_OBJECT

public:
    explicit QJackTrip(QWidget *parent = nullptr);
    ~QJackTrip() override;
    
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

signals:
    void signalExit();
    void signalHide();

private slots:
    void processFinished();
    void processError(const QString &errorMessage);
    void receivedConnectionFromPeer();
    void udpWaitingTooLong();
    void queueLengthChanged(int queueLength);
    void chooseRunType(const QString &type);
    void addressChanged(const QString &address);
    void authFilesChanged();
    void credentialsChanged();
    void browseForFile();
    void receivedIP(QNetworkReply *reply);
    void resetOptions();
    void exit();

public slots:
    void start();
    void stop();
    
private:
    void enableUi(bool enabled);
    void advancedOptionsForHubServer(bool isHubServer);
    void loadSettings();
    void saveSettings();
    
    void setupStatsWindow();
    void appendPlugins(JackTrip *jackTrip, int numChannels);
    
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
    
#ifdef __MAC_OSX__
    NoNap m_noNap;
#endif
};

#endif // QJACKTRIP_H
