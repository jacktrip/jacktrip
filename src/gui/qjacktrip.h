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

#ifdef __APPLE__
#include "NoNap.h"
#endif

#ifdef RT_AUDIO
#include <QComboBox>
#endif

namespace Ui
{
class QJackTrip;
}  // namespace Ui

#ifndef NO_VS
class VirtualStudio;
#endif

class QJackTrip : public QMainWindow
{
    Q_OBJECT

   public:
    explicit QJackTrip(int argc = 0, bool suppressCommandlineWarning = false,
                       QWidget* parent = nullptr);
    ~QJackTrip() override;

    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

#ifndef NO_VS
    enum uiModeT { UNSET, VIRTUAL_STUDIO, STANDARD };
    void setVs(QSharedPointer<VirtualStudio> vs);
#endif

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
#ifndef NO_VS
    void virtualStudioMode();
#endif

   private:
    enum runTypeT { P2P_CLIENT, P2P_SERVER, HUB_CLIENT, HUB_SERVER };
    enum patchTypeT { SERVERTOCLIENT, CLIENTECHO, CLIENTFOFI, FULLMIX, NOAUTO };

    int findTab(const QString& tabName);
    void enableUi(bool enabled);
    void advancedOptionsForHubServer(bool isHubServer);
    void migrateSettings();
    void loadSettings();
    void saveSettings();

#ifdef RT_AUDIO
    void populateDeviceMenu(QComboBox* menu, bool isInput);
#endif

    void appendPlugins(JackTrip* jackTrip, int numSendChannels, int numRecvChannels);

    QString commandLineFromCurrentOptions();
    void showCommandLineMessageBox();

    QScopedPointer<Ui::QJackTrip> m_ui;
    QScopedPointer<UdpHubListener> m_udpHub;
    QScopedPointer<JackTrip> m_jackTrip;
    QScopedPointer<QNetworkAccessManager> m_netManager;
    QScopedPointer<MessageDialog> m_statsDialog;
    QScopedPointer<MessageDialog> m_debugDialog;
    std::ostream m_realCout;
    std::ostream m_realCerr;
    bool m_jackTripRunning;
    bool m_isExiting;
    bool m_exitSent;

    QMutex m_requestMutex;
    QString m_IPv6Address;
    bool m_hasIPv4Reply;
    QString m_lastPath;

    QLabel m_autoQueueIndicator;
    int m_argc;
    bool m_hideWarning;
    bool m_firstShow = true;

#ifndef NO_VS
    QSharedPointer<VirtualStudio> m_vs;
#endif
#ifdef __APPLE__
    NoNap m_noNap;
#endif
};

#endif  // QJACKTRIP_H
