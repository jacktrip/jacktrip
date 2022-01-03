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

#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QSettings>
#include <QVector>
#include <cstdlib>
#include <ctime>

#include "about.h"
#ifdef NO_JTVS
#include "ui_qjacktrip_novs.h"
#else
#include "ui_qjacktrip.h"
#endif
#ifdef USE_WEAK_JACK
#include "weak_libjack.h"
#endif

#ifdef RT_AUDIO
#include "RtAudio.h"
#endif

#include "../Compressor.h"
#include "../CompressorPresets.h"
#include "../Limiter.h"
#include "../Reverb.h"

QJackTrip::QJackTrip(QWidget* parent)
    : QMainWindow(parent)
#ifdef NO_JTVS
    , m_ui(new Ui::QJackTrip)
#else
    , m_ui(new Ui::QJackTripVS)
#endif
    , m_netManager(new QNetworkAccessManager(this))
    , m_statsDialog(new MessageDialog(this, "Stats"))
    , m_debugDialog(new MessageDialog(this, "Debug", 2))
    , m_realCout(std::cout.rdbuf())
    , m_realCerr(std::cerr.rdbuf())
    , m_jackTripRunning(false)
    , m_isExiting(false)
    , m_hasIPv4Reply(false)
    , m_argc(1)
    , m_hideWarning(false)
{
    m_ui->setupUi(this);

    QCoreApplication::setOrganizationName("jacktrip");
    QCoreApplication::setOrganizationDomain("jacktrip.org");
    QCoreApplication::setApplicationName("JackTrip");
    
    // Set up our debug window, and relay everything to our real cout.
    std::cout.rdbuf(m_debugDialog->getOutputStream()->rdbuf());
    std::cerr.rdbuf(m_debugDialog->getOutputStream(1)->rdbuf());
    m_debugDialog->setRelayStream(&m_realCout);
    m_debugDialog->setRelayStream(&m_realCerr, 1);

    // Create all our UI connections.
    connect(m_ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &QJackTrip::chooseRunType);
    connect(m_ui->addressComboBox, &QComboBox::currentTextChanged, this,
            &QJackTrip::addressChanged);
    connect(m_ui->connectButton, &QPushButton::clicked, this, &QJackTrip::start);
    connect(m_ui->disconnectButton, &QPushButton::clicked, this, &QJackTrip::stop);
    connect(m_ui->exitButton, &QPushButton::clicked, this, &QJackTrip::exit);
    connect(m_ui->certBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->keyBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->credsBrowse, &QPushButton::clicked, this, &QJackTrip::browseForFile);
    connect(m_ui->commandLineButton, &QPushButton::clicked, this,
            &QJackTrip::showCommandLineMessageBox);
    connect(m_ui->useDefaultsButton, &QPushButton::clicked, this,
            &QJackTrip::resetOptions);
    connect(m_ui->usernameEdit, &QLineEdit::textChanged, this,
            &QJackTrip::credentialsChanged);
    connect(m_ui->passwordEdit, &QLineEdit::textChanged, this,
            &QJackTrip::credentialsChanged);
    connect(m_ui->certEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->keyEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->credsEdit, &QLineEdit::textChanged, this, &QJackTrip::authFilesChanged);
    connect(m_ui->aboutButton, &QPushButton::clicked, this, [=]() {
        About about(this);
        about.exec();
    });
    connect(m_ui->authCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->usernameLabel->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->usernameEdit->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->passwordLabel->setEnabled(m_ui->authCheckBox->isChecked());
        m_ui->passwordEdit->setEnabled(m_ui->authCheckBox->isChecked());
        credentialsChanged();
    });
    connect(m_ui->requireAuthCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->certLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->certEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->certBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->keyBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsLabel->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsEdit->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        m_ui->credsBrowse->setEnabled(m_ui->requireAuthCheckBox->isChecked());
        authFilesChanged();
    });
    connect(m_ui->ioStatsCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->ioStatsLabel->setEnabled(m_ui->ioStatsCheckBox->isChecked());
        m_ui->ioStatsSpinBox->setEnabled(m_ui->ioStatsCheckBox->isChecked());
        if (!m_ui->ioStatsCheckBox->isChecked()) {
            m_statsDialog->hide();
        }
    });
    connect(m_ui->verboseCheckBox, &QCheckBox::stateChanged, this, [=]() {
        gVerboseFlag = m_ui->verboseCheckBox->isChecked();
        if (!gVerboseFlag) {
            m_debugDialog->hide();
            m_debugDialog->clearOutput();
        }
    });
    connect(m_ui->jitterCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->broadcastCheckBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->broadcastQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                              && m_ui->broadcastCheckBox->isChecked());
        m_ui->broadcastQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                                && m_ui->broadcastCheckBox->isChecked());
        m_ui->bufferStrategyLabel->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->bufferStrategyComboBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        // m_ui->strategyExplanationLabel->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->bufferLine->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->autoQueueCheckBox->setEnabled(m_ui->jitterCheckBox->isChecked());
        m_ui->autoQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                         && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                           && m_ui->autoQueueCheckBox->isChecked());
        m_ui->packetsLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                       && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueExplanationLabel->setEnabled(
            m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked());
        if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
            m_autoQueueIndicator.setText("Auto queue: enabled");
        } else {
            m_autoQueueIndicator.setText("Auto queue: disabled");
        }
    });
    connect(m_ui->broadcastCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->broadcastQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                              && m_ui->broadcastCheckBox->isChecked());
        m_ui->broadcastQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                                && m_ui->broadcastCheckBox->isChecked());
    });
    connect(m_ui->autoQueueCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->autoQueueLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                         && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueSpinBox->setEnabled(m_ui->jitterCheckBox->isChecked()
                                           && m_ui->autoQueueCheckBox->isChecked());
        m_ui->packetsLabel->setEnabled(m_ui->jitterCheckBox->isChecked()
                                       && m_ui->autoQueueCheckBox->isChecked());
        m_ui->autoQueueExplanationLabel->setEnabled(
            m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked());
        if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
            m_autoQueueIndicator.setText("Auto queue: enabled");
        } else {
            m_autoQueueIndicator.setText("Auto queue: disabled");
        }
    });

    connect(m_ui->inFreeverbCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->inFreeverbLabel->setEnabled(m_ui->inFreeverbCheckBox->isChecked());
        m_ui->inFreeverbWetnessSlider->setEnabled(m_ui->inFreeverbCheckBox->isChecked());
    });
    connect(m_ui->inZitarevCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->inZitarevLabel->setEnabled(m_ui->inZitarevCheckBox->isChecked());
        m_ui->inZitarevWetnessSlider->setEnabled(m_ui->inZitarevCheckBox->isChecked());
    });

    connect(m_ui->outFreeverbCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->outFreeverbLabel->setEnabled(m_ui->outFreeverbCheckBox->isChecked());
        m_ui->outFreeverbWetnessSlider->setEnabled(
            m_ui->outFreeverbCheckBox->isChecked());
    });
    connect(m_ui->outZitarevCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->outZitarevLabel->setEnabled(m_ui->outZitarevCheckBox->isChecked());
        m_ui->outZitarevWetnessSlider->setEnabled(m_ui->outZitarevCheckBox->isChecked());
    });
    connect(m_ui->outLimiterCheckBox, &QCheckBox::stateChanged, this, [=]() {
        m_ui->outLimiterLabel->setEnabled(m_ui->outLimiterCheckBox->isChecked());
        m_ui->outClientsSpinBox->setEnabled(m_ui->outLimiterCheckBox->isChecked());
    });
    m_ui->autoPatchComboBox->setVisible(false);
    m_ui->autoPatchLabel->setVisible(false);

    connect(m_netManager.data(), &QNetworkAccessManager::finished, this,
            &QJackTrip::receivedIP);
    // Use the ipify API to find our external IP address.
    m_netManager->get(QNetworkRequest(QUrl("https://api.ipify.org")));
    m_netManager->get(QNetworkRequest(QUrl("https://api6.ipify.org")));
    m_ui->statusBar->showMessage(QString("JackTrip version ").append(gVersion));

    // Set up our interface for the default Client run mode.
    //(loadSettings will take care of the UI in all other cases.)
    m_ui->basePortLabel->setVisible(false);
    m_ui->basePortSpinBox->setVisible(false);
    m_ui->upmixCheckBox->setVisible(false);
    m_ui->requireAuthGroupBox->setVisible(false);

#ifdef RT_AUDIO
    connect(m_ui->backendComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [=](int index) {
                if (index == 1) {
                    m_ui->sampleRateComboBox->setEnabled(true);
                    m_ui->sampleRateLabel->setEnabled(true);
                    m_ui->bufferSizeComboBox->setEnabled(true);
                    m_ui->bufferSizeLabel->setEnabled(true);
                    m_ui->inputDeviceComboBox->setEnabled(true);
                    m_ui->inputDeviceLabel->setEnabled(true);
                    m_ui->outputDeviceComboBox->setEnabled(true);
                    m_ui->outputDeviceLabel->setEnabled(true);
                    m_ui->refreshDevicesButton->setEnabled(true);
                    populateDeviceMenu(m_ui->inputDeviceComboBox, true);
                    populateDeviceMenu(m_ui->outputDeviceComboBox, false);
                } else {
                    m_ui->sampleRateComboBox->setEnabled(false);
                    m_ui->sampleRateLabel->setEnabled(false);
                    m_ui->bufferSizeComboBox->setEnabled(false);
                    m_ui->bufferSizeLabel->setEnabled(false);
                    m_ui->inputDeviceComboBox->setEnabled(false);
                    m_ui->inputDeviceLabel->setEnabled(false);
                    m_ui->outputDeviceComboBox->setEnabled(false);
                    m_ui->outputDeviceLabel->setEnabled(false);
                    m_ui->refreshDevicesButton->setEnabled(false);
                }
            });
    connect(m_ui->refreshDevicesButton, &QPushButton::clicked, this, [=]() {
        populateDeviceMenu(m_ui->inputDeviceComboBox, true);
        populateDeviceMenu(m_ui->outputDeviceComboBox, false);
    });
#else
    int tabIndex = findTab("Audio Backend");
    if (tabIndex != -1) {
        m_ui->optionsTabWidget->removeTab(tabIndex);
    }
#endif

    migrateSettings();
    loadSettings();

    QVector<QLabel*> labels;
    labels << m_ui->inFreeverbLabel << m_ui->inZitarevLabel << m_ui->outFreeverbLabel;
    std::srand(std::time(nullptr));
    int index = std::rand() % 4;
    if (index < 3) {
        labels.at(index)->setToolTip(m_ui->outZitarevLabel->toolTip());
        m_ui->outZitarevLabel->setToolTip("");
    }

    // Add an autoqueue indicator to the status bar.
    m_ui->statusBar->addPermanentWidget(&m_autoQueueIndicator);
    if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
        m_autoQueueIndicator.setText("Auto queue: enabled");
    } else {
        m_autoQueueIndicator.setText("Auto queue: disabled");
    }

#ifdef USE_WEAK_JACK
    // Check if Jack is actually available
    if (have_libjack() != 0) {
#ifdef RT_AUDIO
        m_ui->backendComboBox->setCurrentIndex(1);
        m_ui->backendComboBox->setEnabled(false);
        m_ui->backendLabel->setEnabled(false);

        // If we're in Hub Server mode, switch us back to P2P server mode.
        if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
            m_ui->typeComboBox->setCurrentIndex(P2P_SERVER);
        }
        m_ui->typeComboBox->removeItem(HUB_SERVER);

        QSettings settings;
        settings.beginGroup("Audio");
        if (!settings.value("HideJackWarning", false).toBool()) {
            QCheckBox* dontBugMe = new QCheckBox("Don't show this warning again");
            QMessageBox msgBox;
            msgBox.setText(
                "An installation of JACK was not found. Only the RtAudio\nbackend will "
                "be available. (Hub Server mode is not\ncurrently supported in this "
                "configuration.");
            msgBox.setWindowTitle("JACK Not Available");
            msgBox.setCheckBox(dontBugMe);
            QObject::connect(dontBugMe, &QCheckBox::stateChanged, this,
                             [=]() { m_hideWarning = dontBugMe->isChecked(); });
            msgBox.exec();
            if (m_hideWarning) { settings.setValue("HideJackWarning", true); }
        }
        settings.endGroup();
#else
        QMessageBox msgBox;
        msgBox.setText(
            "An installation of JACK was not found, and no other audio\nbackends are "
            "available. JackTrip will not be able to start.\n(Please install JACK to fix "
            "this.)");
        msgBox.setWindowTitle("JACK Not Available");
        msgBox.exec();
#endif  // RT_AUDIO
    }
#endif  // USE_WEAK_JACK

    m_ui->optionsTabWidget->setCurrentIndex(0);
}

void QJackTrip::closeEvent(QCloseEvent* event)
{
    // Ignore the close event so that we can override the handling of it.
    event->ignore();
    exit();
}

void QJackTrip::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    // We need to fix the layout of our word wrapped labels.
    // The font should be the same for all of them so we can reuse the one QFontMetrics
    // object
    QFontMetrics metrics(m_ui->autoQueueExplanationLabel->font());
    // This seems like a convoluted way to get what is effectively our layout geometry,
    // but until we look at the jitter tab, the layout geometry is unset.
    int width = m_ui->JitterTab->contentsRect().width()
                - m_ui->JitterTab->contentsMargins().left()
                - m_ui->JitterTab->contentsMargins().right()
                - m_ui->JitterTab->layout()->contentsMargins().left()
                - m_ui->JitterTab->layout()->contentsMargins().right();

    /*QRect rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
    m_ui->strategyExplanationLabel->text());
    m_ui->strategyExplanationLabel->setMinimumHeight(rect.height());*/
    QRect rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                      m_ui->autoQueueExplanationLabel->text());
    m_ui->autoQueueExplanationLabel->setMinimumHeight(rect.height());

    width = m_ui->requireAuthGroupBox->contentsRect().width()
            - m_ui->requireAuthGroupBox->contentsMargins().left()
            - m_ui->requireAuthGroupBox->contentsMargins().right()
            - m_ui->requireAuthGroupBox->layout()->contentsMargins().left()
            - m_ui->requireAuthGroupBox->contentsMargins().right();
    rect = metrics.boundingRect(0, 0, width, 0, Qt::TextWordWrap,
                                m_ui->authDisclaimerLabel->text());
    m_ui->authDisclaimerLabel->setMinimumHeight(rect.height());
}

void QJackTrip::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    // One of our arguments will always be --gui, so if that's the only one
    // then we don't need to show the warning message.
    if ((!gVerboseFlag && m_argc > 2) || m_argc > 3) {
        QMessageBox msgBox;
        msgBox.setText(
            "The GUI version of JackTrip currently\nignores any command line "
            "options other\nthan the verbose option (-V).\n\nThis may change in future.");
        msgBox.setWindowTitle("Command line options");
        msgBox.exec();
    }
}

void QJackTrip::setArgc(int argc) { m_argc = argc; }

void QJackTrip::processFinished()
{
    if (!m_jackTripRunning) {
        // Don't execute this if our process isn't actually running.
        return;
    }
    m_jackTripRunning = false;
#ifdef __APPLE__
    m_noNap.enableNap();
#endif
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_udpHub.reset();
    } else {
        m_jackTrip.reset();
    }
    if (m_isExiting) {
        emit signalExit();
    } else {
        enableUi(true);
        m_ui->connectButton->setEnabled(true);
        m_ui->statusBar->showMessage("JackTrip Processes Stopped", 2000);
    }
}

void QJackTrip::processError(const QString& errorMessage)
{
    QMessageBox msgBox;
    if (errorMessage == "Peer Stopped") {
        // Report the other end quitting as a regular occurance rather than an error.
        msgBox.setText(errorMessage);
        msgBox.setWindowTitle("Disconnected");
    } else {
        msgBox.setText(QString("Error: ").append(errorMessage));
        msgBox.setWindowTitle("Doh!");
    }
    msgBox.exec();
    processFinished();
}

void QJackTrip::receivedConnectionFromPeer()
{
    m_ui->statusBar->showMessage("Received Connection from Peer!");
}

void QJackTrip::queueLengthChanged(int queueLength)
{
    m_autoQueueIndicator.setText(QString("Auto queue: %1").arg(queueLength));
}

void QJackTrip::udpWaitingTooLong()
{
    m_ui->statusBar->showMessage("UDP waiting too long (more than 30ms)", 1000);
}

void QJackTrip::chooseRunType(int index)
{
    // Update ui to reflect choice of run mode.
    if (index == HUB_CLIENT || index == P2P_CLIENT) {
        m_ui->addressComboBox->setEnabled(true);
        m_ui->addressLabel->setEnabled(true);
        if (index == HUB_CLIENT) {
            credentialsChanged();
        } else {
            m_ui->connectButton->setEnabled(
                !m_ui->addressComboBox->currentText().isEmpty());
        }
        m_ui->remotePortSpinBox->setVisible(true);
        m_ui->remotePortLabel->setVisible(true);
        m_ui->connectButton->setText("Connect");
        m_ui->disconnectButton->setText("Disconnect");
    } else {
        m_ui->addressComboBox->setEnabled(false);
        m_ui->addressLabel->setEnabled(false);
        m_ui->remotePortSpinBox->setVisible(false);
        m_ui->remotePortLabel->setVisible(false);
        m_ui->connectButton->setText("Start");
        m_ui->disconnectButton->setText("Stop");
        m_ui->connectButton->setEnabled(true);
    }

    if (index == HUB_SERVER) {
        m_ui->channelGroupBox->setVisible(false);
        m_ui->timeoutCheckBox->setVisible(false);
        m_ui->autoPatchComboBox->setVisible(true);
        m_ui->autoPatchLabel->setVisible(true);
        m_ui->upmixCheckBox->setVisible(true);
        m_ui->requireAuthGroupBox->setVisible(true);
        advancedOptionsForHubServer(true);
        int index = findTab("Plugins");
        if (index != -1) {
            m_ui->optionsTabWidget->removeTab(index);
        }
        authFilesChanged();
#ifdef RT_AUDIO
        index = findTab("Audio Backend");
        if (index != -1) {
            m_ui->optionsTabWidget->removeTab(index);
        }
#endif
    } else {
        m_ui->autoPatchComboBox->setVisible(false);
        m_ui->autoPatchLabel->setVisible(false);
        m_ui->upmixCheckBox->setVisible(false);
        m_ui->requireAuthGroupBox->setVisible(false);
        m_ui->channelGroupBox->setVisible(true);
        m_ui->timeoutCheckBox->setVisible(true);
        advancedOptionsForHubServer(false);
        if (findTab("Plugins") == -1) {
            m_ui->optionsTabWidget->addTab(m_ui->pluginsTab, "Plugins");
        }
#ifdef RT_AUDIO
        if (findTab("Audio Backend") == -1) {
            m_ui->optionsTabWidget->insertTab(2, m_ui->backendTab, "Audio Backend");
        }
#endif
    }

    if (index == HUB_CLIENT) {
        m_ui->remoteNameEdit->setVisible(true);
        m_ui->remoteNameLabel->setVisible(true);
        m_ui->authGroupBox->setVisible(true);
    } else {
        m_ui->remoteNameEdit->setVisible(false);
        m_ui->remoteNameLabel->setVisible(false);
        m_ui->authGroupBox->setVisible(false);
    }
}

void QJackTrip::addressChanged(const QString& address)
{
    // Make sure we check that JackTrip isn't running.
    //(This also gets called when we save our recent address list on connecting to a
    //server.)
    if (m_jackTripRunning) { return; }
    if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        m_ui->connectButton->setEnabled(!address.isEmpty());
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        credentialsChanged();
    }
}

void QJackTrip::authFilesChanged()
{
    if (m_ui->typeComboBox->currentIndex() != HUB_SERVER) { return; }

    if (m_ui->requireAuthCheckBox->isChecked()
        && (m_ui->certEdit->text().isEmpty() || m_ui->keyEdit->text().isEmpty()
            || m_ui->credsEdit->text().isEmpty())) {
        m_ui->connectButton->setEnabled(false);
    } else {
        m_ui->connectButton->setEnabled(true);
    }
}

void QJackTrip::credentialsChanged()
{
    if (m_ui->typeComboBox->currentIndex() != HUB_CLIENT) { return; }

    if (m_ui->authCheckBox->isChecked()
        && (m_ui->usernameEdit->text().isEmpty()
            || m_ui->passwordEdit->text().isEmpty())) {
        m_ui->connectButton->setEnabled(false);
    } else {
        m_ui->connectButton->setEnabled(!m_ui->addressComboBox->currentText().isEmpty());
    }
}

void QJackTrip::browseForFile()
{
    QPushButton* sender = static_cast<QPushButton*>(QObject::sender());
    QString fileType;
    QLineEdit* fileEdit;
    if (sender == m_ui->certBrowse) {
        fileType = "Certificates (*.crt *.pem)";
        fileEdit = m_ui->certEdit;
    } else if (sender == m_ui->keyBrowse) {
        fileType = "Keys (*.key *.pem)";
        fileEdit = m_ui->keyEdit;
    } else {
        fileType = "";
        fileEdit = m_ui->credsEdit;
    }
    QString fileName =
        QFileDialog::getOpenFileName(this, "Open File", m_lastPath, fileType);
    if (!fileName.isEmpty()) {
        fileEdit->setText(fileName);
        fileEdit->setFocus(Qt::OtherFocusReason);
        m_lastPath = QFileInfo(fileName).canonicalPath();
    }
}

void QJackTrip::receivedIP(QNetworkReply* reply)
{
    QMutexLocker locker(&m_requestMutex);
    // Check whether we're dealing with our IPv4 or IPv6 request.
    if (reply->url().host().startsWith("api6")) {
        if (reply->error() == QNetworkReply::NoError) {
            m_IPv6Address = QString(reply->readAll());
            // Make sure this isn't just a repeat of our IPv4 address.
            if (QHostAddress(m_IPv6Address).protocol() != QAbstractSocket::IPv6Protocol) {
                m_IPv6Address.clear();
                reply->deleteLater();
                return;
            }
            if (m_hasIPv4Reply) {
                m_ui->ipLabel->setText(m_ui->ipLabel->text().append(
                    QString("\n(IPv6: %1)").arg(m_IPv6Address)));
            }
            m_ui->ipLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        }
    } else {
        if (reply->error() != QNetworkReply::NoError) {
            m_ui->ipLabel->setText("Unable to determine external IP address.");
        } else {
            QByteArray address = reply->readAll();
            m_ui->ipLabel->setText(QString("External IP address: ").append(address));
            m_ui->ipLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        }
        if (!m_IPv6Address.isEmpty()) {
            m_ui->ipLabel->setText(
                m_ui->ipLabel->text().append(QString("\n(IPv6: %1)").arg(m_IPv6Address)));
        }
        m_hasIPv4Reply = true;
    }
    reply->deleteLater();
}

void QJackTrip::resetOptions()
{
    // Reset our basic options
    /*m_ui->channelSpinBox->setValue(2);
    m_ui->autoPatchComboBox->setCurrentIndex(0);
    m_ui->zeroCheckBox->setChecked(false);
    m_ui->timeoutCheckBox->setChecked(false);*/

    // Then advanced options
    m_ui->clientNameEdit->setText("");
    m_ui->remoteNameEdit->setText("");
    m_ui->localPortSpinBox->setValue(gDefaultPort);
    m_ui->remotePortSpinBox->setValue(gDefaultPort);
    m_ui->basePortSpinBox->setValue(61002);
    m_ui->queueLengthSpinBox->setValue(gDefaultQueueLength);
    m_ui->redundancySpinBox->setValue(gDefaultRedundancy);
    m_ui->resolutionComboBox->setCurrentIndex(1);
    m_ui->connectAudioCheckBox->setChecked(true);
    m_ui->realTimeCheckBox->setChecked(true);
    m_ui->ioStatsCheckBox->setChecked(false);
    m_ui->ioStatsSpinBox->setValue(1);

    saveSettings();
}

void QJackTrip::start()
{
    m_ui->connectButton->setEnabled(false);
    enableUi(false);
    m_jackTripRunning = true;
    
    if (gVerboseFlag) {
        m_debugDialog->show();
    }

    // Start the appropriate JackTrip process.
    try {
        if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
            m_udpHub.reset(new UdpHubListener(m_ui->localPortSpinBox->value(),
                                              m_ui->basePortSpinBox->value()));
            int hubConnectionMode = m_ui->autoPatchComboBox->currentIndex();
            if (hubConnectionMode > 2) {
                // Adjust for the RESERVEDMATRIX gap.
                hubConnectionMode++;
            }

            m_udpHub->setHubPatch(hubConnectionMode);
            m_udpHub->setStereoUpmix(m_ui->upmixCheckBox->isChecked());

            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_udpHub->setUnderRunMode(JackTrip::ZEROS);
            }

            if (!m_ui->jitterCheckBox->isChecked()) {
                m_udpHub->setBufferStrategy(-1);
                m_udpHub->setBufferQueueLength(m_ui->queueLengthSpinBox->value());
            } else {
                m_udpHub->setBufferStrategy(m_ui->bufferStrategyComboBox->currentIndex()
                                            + 1);
                if (m_ui->broadcastCheckBox->isChecked()) {
                    m_udpHub->setBroadcast(m_ui->broadcastQueueSpinBox->value());
                }
                if (m_ui->autoQueueCheckBox->isChecked()) {
                    m_udpHub->setBufferQueueLength(-(m_ui->autoQueueSpinBox->value()));
                    m_autoQueueIndicator.setText("Auto queue: enabled");
                } else {
                    m_udpHub->setBufferQueueLength(m_ui->queueLengthSpinBox->value());
                }
            }
            m_udpHub->setUseRtUdpPriority(m_ui->realTimeCheckBox->isChecked());

            // Enable authentication if needed
            if (m_ui->requireAuthCheckBox->isChecked()) {
                m_udpHub->setRequireAuth(true);
                m_udpHub->setCertFile(m_ui->certEdit->text());
                m_udpHub->setKeyFile(m_ui->keyEdit->text());
                m_udpHub->setCredsFile(m_ui->credsEdit->text());
            }

            // Open our stats window if needed
            if (m_ui->ioStatsCheckBox->isChecked()) {
                m_statsDialog->clearOutput();
                m_statsDialog->show();
                m_udpHub->setIOStatTimeout(m_ui->ioStatsSpinBox->value());
                m_udpHub->setIOStatStream(m_statsDialog->getOutputStream());
            }

            QObject::connect(m_udpHub.data(), &UdpHubListener::signalStopped, this,
                             &QJackTrip::processFinished, Qt::QueuedConnection);
            QObject::connect(m_udpHub.data(), &UdpHubListener::signalError, this,
                             &QJackTrip::processError, Qt::QueuedConnection);
            m_ui->disconnectButton->setEnabled(true);
            m_udpHub->start();
            m_ui->statusBar->showMessage("Hub Server Started");
        } else {
            JackTrip::jacktripModeT jackTripMode;
            if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
                jackTripMode = JackTrip::CLIENT;
            } else if (m_ui->typeComboBox->currentIndex() == P2P_SERVER) {
                jackTripMode = JackTrip::SERVER;
            } else {
                jackTripMode = JackTrip::CLIENTTOPINGSERVER;
            }

            AudioInterface::audioBitResolutionT resolution;
            if (m_ui->resolutionComboBox->currentIndex() == 0) {
                resolution = AudioInterface::BIT8;
            } else if (m_ui->resolutionComboBox->currentIndex() == 1) {
                resolution = AudioInterface::BIT16;
            } else if (m_ui->resolutionComboBox->currentIndex() == 2) {
                resolution = AudioInterface::BIT24;
            } else {
                resolution = AudioInterface::BIT32;
            }

            m_jackTrip.reset(new JackTrip(jackTripMode, JackTrip::UDP,
                                          m_ui->channelSendSpinBox->value(),
                                          m_ui->channelRecvSpinBox->value(),
#ifdef WAIR  // wair
                                          0,
#endif  // endwhere
                                          m_ui->queueLengthSpinBox->value(),
                                          m_ui->redundancySpinBox->value(), resolution));
            m_jackTrip->setConnectDefaultAudioPorts(
                m_ui->connectAudioCheckBox->isChecked());
            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_jackTrip->setUnderRunMode(JackTrip::ZEROS);
            }

#ifdef RT_AUDIO
            if (m_ui->backendComboBox->currentIndex() == 1) {
                m_jackTrip->setAudiointerfaceMode(JackTrip::RTAUDIO);
                m_jackTrip->setSampleRate(
                    m_ui->sampleRateComboBox->currentText().toInt());
                m_jackTrip->setAudioBufferSizeInSamples(
                    m_ui->bufferSizeComboBox->currentText().toInt());
                // we assume that first entry is "(default)"
                if(m_ui->inputDeviceComboBox->currentIndex() == 0) {
                    m_jackTrip->setInputDevice("");
                } else {
                    m_jackTrip->setInputDevice(
                      m_ui->inputDeviceComboBox->currentText().toStdString());
                }
                if(m_ui->outputDeviceComboBox->currentIndex() == 0) {
                    m_jackTrip->setOutputDevice("");
                } else {
                    m_jackTrip->setOutputDevice(
                      m_ui->outputDeviceComboBox->currentText().toStdString());
                }
            }
#endif

            if (m_ui->timeoutCheckBox->isChecked()) {
                m_jackTrip->setStopOnTimeout(true);
            }

            if (m_ui->jitterCheckBox->isChecked()) {
                m_jackTrip->setBufferStrategy(m_ui->bufferStrategyComboBox->currentIndex()
                                              + 1);
                if (m_ui->broadcastCheckBox->isChecked()) {
                    m_jackTrip->setBroadcast(m_ui->broadcastQueueSpinBox->value());
                }
                if (m_ui->autoQueueCheckBox->isChecked()) {
                    m_jackTrip->setBufferQueueLength(-(m_ui->autoQueueSpinBox->value()));
                    m_autoQueueIndicator.setText("Auto queue: enabled");
                }
            } else {
                m_jackTrip->setBufferStrategy(-1);
            }
            m_jackTrip->setUseRtUdpPriority(m_ui->realTimeCheckBox->isChecked());

            // Set peer address in client mode
            if (jackTripMode == JackTrip::CLIENT
                || jackTripMode == JackTrip::CLIENTTOPINGSERVER) {
                m_jackTrip->setPeerAddress(m_ui->addressComboBox->currentText().trimmed());
                if (jackTripMode == JackTrip::CLIENTTOPINGSERVER
                    && !m_ui->remoteNameEdit->text().isEmpty()) {
                    m_jackTrip->setRemoteClientName(m_ui->remoteNameEdit->text());
                }
            }

            m_jackTrip->setBindPorts(m_ui->localPortSpinBox->value());
            m_jackTrip->setPeerPorts(m_ui->remotePortSpinBox->value());
            m_jackTrip->setPeerHandshakePort(m_ui->remotePortSpinBox->value());

            if (!m_ui->clientNameEdit->text().isEmpty()) {
                m_jackTrip->setClientName(m_ui->clientNameEdit->text());
            }

            // Set credentials if we're using authentication
            if (m_ui->authCheckBox->isChecked()) {
                m_jackTrip->setUseAuth(true);
                m_jackTrip->setUsername(m_ui->usernameEdit->text());
                m_jackTrip->setPassword(m_ui->passwordEdit->text());
            }

            // Open our stats window if needed
            if (m_ui->ioStatsCheckBox->isChecked()) {
                m_statsDialog->clearOutput();
                m_statsDialog->show();
                m_jackTrip->setIOStatTimeout(m_ui->ioStatsSpinBox->value());
                m_jackTrip->setIOStatStream(m_statsDialog->getOutputStream());
            }

            // Append any plugins
            appendPlugins(m_jackTrip.data(), m_ui->channelSendSpinBox->value(),
                          m_ui->channelRecvSpinBox->value());

            QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this,
                             &QJackTrip::processFinished, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this,
                             &QJackTrip::processError, Qt::QueuedConnection);
            QObject::connect(
                m_jackTrip.data(), &JackTrip::signalReceivedConnectionFromPeer, this,
                &QJackTrip::receivedConnectionFromPeer, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalUdpWaitingTooLong, this,
                             &QJackTrip::udpWaitingTooLong, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalQueueLengthChanged, this,
                             &QJackTrip::queueLengthChanged, Qt::QueuedConnection);
            m_ui->statusBar->showMessage("Waiting for Peer...");
            m_ui->disconnectButton->setEnabled(true);
#ifdef WAIRTOHUB  // WAIR
            m_jackTrip->startProcess(
                0);  // for WAIR compatibility, ID in jack client name
#else
            m_jackTrip->startProcess();
#endif  // endwhere
        }
    } catch (const std::exception& e) {
        // Let the user know what our exception was.
        QMessageBox msgBox;
        msgBox.setText(QString("Error: ").append(e.what()));
        msgBox.setWindowTitle("Doh!");
        msgBox.exec();

        m_jackTripRunning = false;
        enableUi(true);
        m_ui->connectButton->setEnabled(true);
        m_ui->disconnectButton->setEnabled(false);
        m_ui->statusBar->clearMessage();

        return;
    }

    // Add the address to our server history.
    QString serverAddress = m_ui->addressComboBox->currentText().trimmed();
    int serverIndex       = m_ui->addressComboBox->findText(serverAddress);
    if (serverIndex != -1) { m_ui->addressComboBox->removeItem(serverIndex); }
    m_ui->addressComboBox->insertItem(0, serverAddress);
    m_ui->addressComboBox->setCurrentIndex(0);

#ifdef __APPLE__
    m_noNap.disableNap();
#endif
}

void QJackTrip::stop()
{
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        m_udpHub->stop();
    } else {
        m_jackTrip->stop();
    }
}

void QJackTrip::exit()
{
    // Only run this once.
    if (m_isExiting) { return; }
    m_isExiting = true;
    m_ui->exitButton->setEnabled(false);
    saveSettings();
    if (m_jackTripRunning) {
        stop();
    } else {
        emit signalExit();
    }
}

int QJackTrip::findTab(const QString& tabName)
{
    for (int i = 0; i < m_ui->optionsTabWidget->count(); i++) {
        if (m_ui->optionsTabWidget->tabText(i) == tabName) {
            return i;
        }
    }
    return -1;
}

void QJackTrip::enableUi(bool enabled)
{
    m_ui->optionsTabWidget->setEnabled(enabled);
    m_ui->typeLabel->setEnabled(enabled);
    m_ui->typeComboBox->setEnabled(enabled);
    m_ui->addressLabel->setEnabled(
        enabled
        && (m_ui->typeComboBox->currentIndex() == P2P_CLIENT
            || m_ui->typeComboBox->currentIndex() == HUB_CLIENT));
    m_ui->addressComboBox->setEnabled(
        enabled
        && (m_ui->typeComboBox->currentIndex() == P2P_CLIENT
            || m_ui->typeComboBox->currentIndex() == HUB_CLIENT));
}

void QJackTrip::advancedOptionsForHubServer(bool isHubServer)
{
    m_ui->clientNameLabel->setVisible(!isHubServer);
    m_ui->clientNameEdit->setVisible(!isHubServer);
    m_ui->redundancyLabel->setVisible(!isHubServer);
    m_ui->redundancySpinBox->setVisible(!isHubServer);
    m_ui->resolutionLabel->setVisible(!isHubServer);
    m_ui->resolutionComboBox->setVisible(!isHubServer);
    m_ui->connectAudioCheckBox->setVisible(!isHubServer);
    m_ui->basePortLabel->setVisible(isHubServer);
    m_ui->basePortSpinBox->setVisible(isHubServer);
    if (isHubServer) {
        m_ui->localPortSpinBox->setToolTip(
            "Set the local TCP port to use for the initial handshake connection. The "
            "default is 4464.");
    } else {
        m_ui->localPortSpinBox->setToolTip(
            "Set the local port to use for the connection. The default is 4464.\n(Useful "
            "for running multiple hub clients behind the same router.)");
    }
}

void QJackTrip::migrateSettings()
{
    // Function to migrate settings for users who previously had QJackTrip installed.
    QSettings settings;
    if (settings.value("Migrated", false).toBool()) { return; }
#ifdef __APPLE__
    QSettings oldSettings("psi-borg.org", "QJackTrip");
#else
    QSettings oldSettings("psi-borg", "QJackTrip");
#endif
    QStringList keys = oldSettings.allKeys();
    for (int i = 0; i < keys.size(); i++) {
        settings.setValue(keys.at(i), oldSettings.value(keys.at(i), QVariant()));
    }
    settings.setValue("Migrated", true);
}

void QJackTrip::loadSettings()
{
    QSettings settings;
    m_ui->typeComboBox->setCurrentIndex(settings.value("RunMode", 2).toInt());

    // Migrate to separate send and receive channel numbers.
    int oldChannelSetting = settings.value("Channels", -1).toInt();
    if (oldChannelSetting != -1) {
        m_ui->channelSendSpinBox->setValue(oldChannelSetting);
        m_ui->channelRecvSpinBox->setValue(oldChannelSetting);
        settings.remove("Channels");
    } else {
        m_ui->channelSendSpinBox->setValue(
            settings.value("ChannelsSend", gDefaultNumInChannels).toInt());
        m_ui->channelRecvSpinBox->setValue(
            settings.value("ChannelsRecv", gDefaultNumOutChannels).toInt());
    }

    m_ui->autoPatchComboBox->setCurrentIndex(settings.value("AutoPatchMode", 0).toInt());
    m_ui->upmixCheckBox->setChecked(settings.value("StereoUpmix", false).toBool());
    m_ui->zeroCheckBox->setChecked(settings.value("ZeroUnderrun", false).toBool());
    m_ui->timeoutCheckBox->setChecked(settings.value("Timeout", false).toBool());
    m_ui->clientNameEdit->setText(settings.value("ClientName", "").toString());
    m_ui->remoteNameEdit->setText(settings.value("RemoteName", "").toString());
    m_ui->localPortSpinBox->setValue(settings.value("LocalPort", gDefaultPort).toInt());
    m_ui->remotePortSpinBox->setValue(settings.value("RemotePort", gDefaultPort).toInt());
    m_ui->basePortSpinBox->setValue(settings.value("BasePort", 61002).toInt());
    m_ui->queueLengthSpinBox->setValue(
        settings.value("QueueLength", gDefaultQueueLength).toInt());
    m_ui->redundancySpinBox->setValue(
        settings.value("Redundancy", gDefaultRedundancy).toInt());
    m_ui->resolutionComboBox->setCurrentIndex(settings.value("Resolution", 1).toInt());
    m_ui->connectAudioCheckBox->setChecked(settings.value("ConnectAudio", true).toBool());
    m_ui->realTimeCheckBox->setChecked(settings.value("RTNetworking", true).toBool());
    // This may have been set by the command line, so don't overwrite if that's the case.
    m_ui->verboseCheckBox->setChecked(gVerboseFlag || settings.value("Debug", 0).toBool());
    m_lastPath = settings.value("LastPath", QDir::homePath()).toString();

    settings.beginGroup("RecentServers");
    for (int i = 1; i <= 5; i++) {
        QString address = settings.value(QString("Server%1").arg(i), "").toString();
        if (!address.isEmpty()) { m_ui->addressComboBox->addItem(address); }
    }
    settings.endGroup();
    // Need to get this here so it isn't overwritten by the previous section.
    m_ui->addressComboBox->setCurrentText(settings.value("LastAddress", "").toString());

#ifdef RT_AUDIO
    settings.beginGroup("Audio");
    m_ui->backendComboBox->setCurrentIndex(settings.value("Backend", 0).toInt());
    m_ui->sampleRateComboBox->setCurrentText(
        settings.value("SampleRate", "48000").toString());
    m_ui->bufferSizeComboBox->setCurrentText(
        settings.value("BufferSize", "128").toString());
    // update device list and set the device
    populateDeviceMenu(m_ui->inputDeviceComboBox, true);
    auto inDevice = settings.value("InputDevice").toString();
    if(!inDevice.isEmpty()) {
        m_ui->inputDeviceComboBox->setCurrentText(inDevice);
    }
    populateDeviceMenu(m_ui->outputDeviceComboBox, false);
    auto outDevice = settings.value("OutputDevice").toString();
    if(!outDevice.isEmpty()) {
        m_ui->outputDeviceComboBox->setCurrentText(outDevice);
    }
    settings.endGroup();
#endif

    settings.beginGroup("Auth");
    m_ui->requireAuthCheckBox->setChecked(settings.value("Require", false).toBool());
    m_ui->certEdit->setText(settings.value("CertFile", "").toString());
    m_ui->keyEdit->setText(settings.value("KeyFile", "").toString());
    m_ui->credsEdit->setText(settings.value("CredsFile", "").toString());
    m_ui->authCheckBox->setChecked(settings.value("Use", false).toBool());
    m_ui->usernameEdit->setText(settings.value("Username", "").toString());
    settings.endGroup();

    settings.beginGroup("IOStats");
    m_ui->ioStatsCheckBox->setChecked(settings.value("Display", false).toBool());
    m_ui->ioStatsSpinBox->setValue(settings.value("ReportingInterval", 1).toInt());
    settings.endGroup();

    settings.beginGroup("JitterBuffer");
    bool jitterAnnounce = settings.value("JitterAnnounce", false).toBool();
    if (!jitterAnnounce && !settings.value("Enabled", true).toBool()) {
        QMessageBox msgBox;
        msgBox.setText(
            "From this build onwards, the new jitter buffer is being enabled by default. "
            "You can turn it off in the Jitter Buffer settings tab.");
        msgBox.setWindowTitle("Jitter Buffer");
        msgBox.exec();
        settings.setValue("Enabled", true);
    }
    settings.setValue("JitterAnnounce", true);
    m_ui->jitterCheckBox->setChecked(settings.value("Enabled", true).toBool());
    m_ui->broadcastCheckBox->setChecked(settings.value("Broadcast", false).toBool());
    m_ui->broadcastQueueSpinBox->setValue(
        settings.value("BroadcastLength", gDefaultQueueLength * 2).toInt());
    m_ui->bufferStrategyComboBox->setCurrentIndex(settings.value("Strategy", 1).toInt()
                                                  - 1);
    m_ui->autoQueueCheckBox->setChecked(settings.value("AutoQueue", true).toBool());
    m_ui->autoQueueSpinBox->setValue(settings.value("TuningParameter", 500).toInt());
    settings.endGroup();

    settings.beginGroup("InPlugins");
    m_ui->inFreeverbCheckBox->setChecked(settings.value("Freeverb", false).toBool());
    m_ui->inFreeverbWetnessSlider->setValue(settings.value("FreeverbWetness", 0).toInt());
    m_ui->inZitarevCheckBox->setChecked(settings.value("Zitarev", false).toBool());
    m_ui->inZitarevWetnessSlider->setValue(settings.value("ZitarevWetness", 0).toInt());
    m_ui->inCompressorCheckBox->setChecked(settings.value("Compressor", false).toBool());
    m_ui->inLimiterCheckBox->setChecked(settings.value("Limiter", false).toBool());
    settings.endGroup();

    settings.beginGroup("OutPlugins");
    m_ui->outFreeverbCheckBox->setChecked(settings.value("Freeverb", false).toBool());
    m_ui->outFreeverbWetnessSlider->setValue(
        settings.value("FreeverbWetness", 0).toInt());
    m_ui->outZitarevCheckBox->setChecked(settings.value("Zitarev", false).toBool());
    m_ui->outZitarevWetnessSlider->setValue(settings.value("ZitarevWetness", 0).toInt());
    m_ui->outCompressorCheckBox->setChecked(settings.value("Compressor", false).toBool());
    m_ui->outLimiterCheckBox->setChecked(settings.value("Limiter", false).toBool());
    m_ui->outClientsSpinBox->setValue(settings.value("Clients", 1).toInt());
    settings.endGroup();

    settings.beginGroup("Window");
    QByteArray geometry = settings.value("Geometry").toByteArray();
    if (geometry.size() > 0) {
        restoreGeometry(geometry);
    } else {
        // Because of hidden elements in our dialog window, it's vertical size in the
        // creator is getting rediculous. Set it to something sensible by default if this
        // is our first load.
        this->resize(QSize(this->size().width(), 600));
    }
    settings.endGroup();
}

void QJackTrip::saveSettings()
{
    QSettings settings;
    settings.setValue("RunMode", m_ui->typeComboBox->currentIndex());
    settings.setValue("LastAddress", m_ui->addressComboBox->currentText());
    settings.setValue("ChannelsSend", m_ui->channelSendSpinBox->value());
    settings.setValue("ChannelsRecv", m_ui->channelRecvSpinBox->value());
    settings.setValue("AutoPatchMode", m_ui->autoPatchComboBox->currentIndex());
    settings.setValue("StereoUpmix", m_ui->upmixCheckBox->isChecked());
    settings.setValue("ZeroUnderrun", m_ui->zeroCheckBox->isChecked());
    settings.setValue("Timeout", m_ui->timeoutCheckBox->isChecked());
    settings.setValue("ClientName", m_ui->clientNameEdit->text());
    settings.setValue("RemoteName", m_ui->remoteNameEdit->text());
    settings.setValue("LocalPort", m_ui->localPortSpinBox->value());
    settings.setValue("RemotePort", m_ui->remotePortSpinBox->value());
    settings.setValue("BasePort", m_ui->basePortSpinBox->value());
    settings.setValue("QueueLength", m_ui->queueLengthSpinBox->value());
    settings.setValue("Redundancy", m_ui->redundancySpinBox->value());
    settings.setValue("Resolution", m_ui->resolutionComboBox->currentIndex());
    settings.setValue("ConnectAudio", m_ui->connectAudioCheckBox->isChecked());
    settings.setValue("RTNetworking", m_ui->realTimeCheckBox->isChecked());
    settings.setValue("Debug", m_ui->verboseCheckBox->isChecked());
    settings.setValue("LastPath", m_lastPath);

    settings.beginGroup("RecentServers");
    for (int i = 0; i < m_ui->addressComboBox->count(); i++) {
        settings.setValue(QString("Server%1").arg(i + 1),
                          m_ui->addressComboBox->itemText(i));
    }
    settings.endGroup();

#ifdef RT_AUDIO
    settings.beginGroup("Audio");
    settings.setValue("Backend", m_ui->backendComboBox->currentIndex());
    settings.setValue("SampleRate", m_ui->sampleRateComboBox->currentText());
    settings.setValue("BufferSize", m_ui->bufferSizeComboBox->currentText());
    settings.setValue("InputDevice", m_ui->inputDeviceComboBox->currentText());
    settings.setValue("OutputDevice", m_ui->outputDeviceComboBox->currentText());
    settings.endGroup();
#endif

    settings.beginGroup("Auth");
    settings.setValue("Require", m_ui->requireAuthCheckBox->isChecked());
    settings.setValue("CertFile", m_ui->certEdit->text());
    settings.setValue("KeyFile", m_ui->keyEdit->text());
    settings.setValue("CredsFile", m_ui->credsEdit->text());
    settings.setValue("Use", m_ui->authCheckBox->isChecked());
    settings.setValue("Username", m_ui->usernameEdit->text());
    settings.endGroup();

    settings.beginGroup("IOStats");
    settings.setValue("Display", m_ui->ioStatsCheckBox->isChecked());
    settings.setValue("ReportingInterval", m_ui->ioStatsSpinBox->value());
    settings.endGroup();

    settings.beginGroup("JitterBuffer");
    settings.setValue("Enabled", m_ui->jitterCheckBox->isChecked());
    settings.setValue("Broadcast", m_ui->broadcastCheckBox->isChecked());
    settings.setValue("BroadcastLength", m_ui->broadcastQueueSpinBox->value());
    settings.setValue("Strategy", m_ui->bufferStrategyComboBox->currentIndex() + 1);
    settings.setValue("AutoQueue", m_ui->autoQueueCheckBox->isChecked());
    settings.setValue("TuningParameter", m_ui->autoQueueSpinBox->value());
    settings.endGroup();

    settings.beginGroup("InPlugins");
    settings.setValue("Freeverb", m_ui->inFreeverbCheckBox->isChecked());
    settings.setValue("FreeverbWetness", m_ui->inFreeverbWetnessSlider->value());
    settings.setValue("Zitarev", m_ui->inZitarevCheckBox->isChecked());
    settings.setValue("ZitarevWetness", m_ui->inZitarevWetnessSlider->value());
    settings.setValue("Compressor", m_ui->inCompressorCheckBox->isChecked());
    settings.setValue("Limiter", m_ui->inLimiterCheckBox->isChecked());
    settings.endGroup();

    settings.beginGroup("OutPlugins");
    settings.setValue("Freeverb", m_ui->outFreeverbCheckBox->isChecked());
    settings.setValue("FreeverbWetness", m_ui->outFreeverbWetnessSlider->value());
    settings.setValue("Zitarev", m_ui->outZitarevCheckBox->isChecked());
    settings.setValue("ZitarevWetness", m_ui->outZitarevWetnessSlider->value());
    settings.setValue("Compressor", m_ui->outCompressorCheckBox->isChecked());
    settings.setValue("Limiter", m_ui->outLimiterCheckBox->isChecked());
    settings.setValue("Clients", m_ui->outClientsSpinBox->value());
    settings.endGroup();

    settings.beginGroup("Window");
    settings.setValue("Geometry", saveGeometry());
    settings.endGroup();
}

void QJackTrip::appendPlugins(JackTrip* jackTrip, int numSendChannels,
                              int numRecvChannels)
{
    if (!jackTrip) { return; }

    // These effects are currently deleted by the AudioInterface of jacktrip.
    // May need to change this code if we move to smart pointers.
    if (m_ui->outCompressorCheckBox->isChecked()) {
        jackTrip->appendProcessPluginToNetwork(
            new Compressor(numSendChannels, false, CompressorPresets::voice));
    }
    if (m_ui->inCompressorCheckBox->isChecked()) {
        jackTrip->appendProcessPluginFromNetwork(
            new Compressor(numRecvChannels, false, CompressorPresets::voice));
    }

    if (m_ui->outZitarevCheckBox->isChecked()) {
        qreal wetness = m_ui->outZitarevWetnessSlider->value() / 100.0;
        jackTrip->appendProcessPluginToNetwork(
            new Reverb(numSendChannels, numSendChannels, 1.0 + wetness));
    }
    if (m_ui->inZitarevCheckBox->isChecked()) {
        qreal wetness = m_ui->inZitarevWetnessSlider->value() / 100.0;
        jackTrip->appendProcessPluginFromNetwork(
            new Reverb(numRecvChannels, numRecvChannels, 1.0 + wetness));
    }

    if (m_ui->outFreeverbCheckBox->isChecked()) {
        qreal wetness = m_ui->outFreeverbWetnessSlider->value() / 100.0;
        jackTrip->appendProcessPluginToNetwork(
            new Reverb(numSendChannels, numSendChannels, wetness));
    }
    if (m_ui->inFreeverbCheckBox->isChecked()) {
        qreal wetness = m_ui->inFreeverbWetnessSlider->value() / 100.0;
        jackTrip->appendProcessPluginFromNetwork(
            new Reverb(numRecvChannels, numRecvChannels, wetness));
    }

    // Limiters go last in the plugin sequence.
    if (m_ui->inLimiterCheckBox->isChecked()) {
        jackTrip->appendProcessPluginFromNetwork(new Limiter(numSendChannels, 1));
    }
    if (m_ui->outLimiterCheckBox->isChecked()) {
        jackTrip->appendProcessPluginToNetwork(
            new Limiter(numRecvChannels, m_ui->outClientsSpinBox->value()));
    }
}

QString QJackTrip::commandLineFromCurrentOptions()
{
    QString commandLine = "jacktrip";

    if (m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        commandLine.append(" -c ").append(m_ui->addressComboBox->currentText());
    } else if (m_ui->typeComboBox->currentIndex() == P2P_SERVER) {
        commandLine.append(" -s");
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        commandLine.append(" -C ").append(m_ui->addressComboBox->currentText());
    } else {
        commandLine.append(" -S");
    }

    if (m_ui->zeroCheckBox->isChecked()) { commandLine.append(" -z"); }

    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        int hubConnectionMode = m_ui->autoPatchComboBox->currentIndex();
        if (hubConnectionMode > 2) {
            // Adjust for the RESERVEDMATRIX gap.
            hubConnectionMode++;
        }
        if (hubConnectionMode > 0) {
            commandLine.append(QString(" -p %1").arg(hubConnectionMode));
        }
        if (m_ui->upmixCheckBox->isChecked()) {
            commandLine.append(" -u");
        }
    } else {
        if (m_ui->channelSendSpinBox->value() != gDefaultNumInChannels
            || m_ui->channelRecvSpinBox->value() != gDefaultNumOutChannels) {
            if (m_ui->channelSendSpinBox->value() == m_ui->channelRecvSpinBox->value()) {
                commandLine.append(
                    QString(" -n %1").arg(m_ui->channelRecvSpinBox->value()));
            } else {
                commandLine.append(QString(" --receivechannels %1 --sendchannels %2")
                                       .arg(m_ui->channelRecvSpinBox->value())
                                       .arg(m_ui->channelSendSpinBox->value()));
            }
        }
        if (m_ui->timeoutCheckBox->isChecked()) { commandLine.append(" -t"); }
    }

    int bufStrategy = -1;
    if (m_ui->jitterCheckBox->isChecked()) {
        bufStrategy = m_ui->bufferStrategyComboBox->currentIndex() + 1;
    }
    if (bufStrategy != 1) {
        commandLine.append(QString(" --bufstrategy %1").arg(bufStrategy));
    }

    if (m_ui->jitterCheckBox->isChecked() && m_ui->autoQueueCheckBox->isChecked()) {
        if (m_ui->autoQueueSpinBox->value() == 500) {
            commandLine.append(QString(" -q auto"));
        } else {
            commandLine.append(
                QString(" -q auto%1").arg(m_ui->autoQueueSpinBox->value()));
        }
    } else if (m_ui->queueLengthSpinBox->value() != gDefaultQueueLength) {
        commandLine.append(QString(" -q %1").arg(m_ui->queueLengthSpinBox->value()));
    }

    if (m_ui->jitterCheckBox->isChecked() && m_ui->broadcastCheckBox->isChecked()) {
        commandLine.append(
            QString(" --broadcast %1").arg(m_ui->broadcastQueueSpinBox->value()));
    }

    // Port settings
    if (m_ui->localPortSpinBox->value() != gDefaultPort) {
        commandLine.append(QString(" -B %1").arg(m_ui->localPortSpinBox->value()));
    }
    if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT
        || m_ui->typeComboBox->currentIndex() == P2P_CLIENT) {
        if (m_ui->remotePortSpinBox->value() != gDefaultPort) {
            commandLine.append(QString(" -P %1").arg(m_ui->remotePortSpinBox->value()));
        }
    }

    // Auth settings
    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        if (m_ui->requireAuthCheckBox->isChecked()) {
            commandLine.append(" -A");
            if (!m_ui->certEdit->text().isEmpty()) {
                commandLine.append(" --certfile ").append(m_ui->certEdit->text());
            }
            if (!m_ui->keyEdit->text().isEmpty()) {
                commandLine.append(" --keyfile ").append(m_ui->keyEdit->text());
            }
            if (!m_ui->credsEdit->text().isEmpty()) {
                commandLine.append(" --credsfile ").append(m_ui->credsEdit->text());
            }
        }
    } else if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT) {
        if (m_ui->authCheckBox->isChecked()) {
            commandLine.append(" -A");
            if (!m_ui->usernameEdit->text().isEmpty()) {
                commandLine.append(" --username ").append(m_ui->usernameEdit->text());
            }
            /*if (!m_ui->passwordEdit->text().isEmpty()) {
                commandLine.append(" --password <password>");
            }*/
        }
    }

    if (m_ui->typeComboBox->currentIndex() == HUB_SERVER) {
        int offset = m_ui->localPortSpinBox->value() - gDefaultPort;
        if (m_ui->basePortSpinBox->value() != 61002 + offset) {
            commandLine.append(QString(" -U %1").arg(m_ui->basePortSpinBox->value()));
        }
    } else {
        if (!m_ui->clientNameEdit->text().isEmpty()) {
            commandLine.append(QString(" -J \"%1\"").arg(m_ui->clientNameEdit->text()));
        }
        if (m_ui->typeComboBox->currentIndex() == HUB_CLIENT
            && !m_ui->remoteNameEdit->text().isEmpty()) {
            commandLine.append(QString(" -K \"%1\"").arg(m_ui->remoteNameEdit->text()));
        }
        if (m_ui->redundancySpinBox->value() > 1) {
            commandLine.append(QString(" -r %1").arg(m_ui->redundancySpinBox->value()));
        }
        if (m_ui->resolutionComboBox->currentText() != "16") {
            commandLine.append(" -b ").append(m_ui->resolutionComboBox->currentText());
        }
        if (!m_ui->connectAudioCheckBox->isChecked()) { commandLine.append(" -D"); }

        if (m_ui->inLimiterCheckBox->isChecked()
            || m_ui->outLimiterCheckBox->isChecked()) {
            commandLine.append(" -O ");
            if (m_ui->inLimiterCheckBox->isChecked()) { commandLine.append("i"); }
            if (m_ui->outLimiterCheckBox->isChecked()) {
                commandLine.append("o");
                if (m_ui->outClientsSpinBox->value() != 2) {
                    commandLine.append(
                        QString(" -a %1").arg(m_ui->outClientsSpinBox->value()));
                }
            }
        }

        bool inEffects = m_ui->inFreeverbCheckBox->isChecked()
                         || m_ui->inZitarevCheckBox->isChecked()
                         || m_ui->inCompressorCheckBox->isChecked();
        bool outEffects = m_ui->outFreeverbCheckBox->isChecked()
                          || m_ui->outZitarevCheckBox->isChecked()
                          || m_ui->outCompressorCheckBox->isChecked();
        if (inEffects || outEffects) {
            commandLine.append(" -f \"");
            if (inEffects) {
                commandLine.append("i:");
                if (m_ui->inCompressorCheckBox->isChecked()) { commandLine.append("c"); }
                if (m_ui->inFreeverbCheckBox->isChecked()) {
                    commandLine.append(QString("f(%1)").arg(
                        m_ui->inFreeverbWetnessSlider->value() / 100.0));
                }
                if (m_ui->inZitarevCheckBox->isChecked()) {
                    commandLine.append(QString("f(%1)").arg(
                        m_ui->inZitarevWetnessSlider->value() / 100.0));
                }
                if (outEffects) { commandLine.append(", "); }
            }
            if (outEffects) {
                commandLine.append("o:");
                if (m_ui->outCompressorCheckBox->isChecked()) { commandLine.append("c"); }
                if (m_ui->outFreeverbCheckBox->isChecked()) {
                    commandLine.append(QString("f(%1)").arg(
                        m_ui->outFreeverbWetnessSlider->value() / 100.0));
                }
                if (m_ui->outZitarevCheckBox->isChecked()) {
                    commandLine.append(QString("f(%1)").arg(
                        m_ui->outZitarevWetnessSlider->value() / 100.0));
                }
            }
            commandLine.append("\"");
        }
    }
    if (m_ui->ioStatsCheckBox->isChecked()) {
        commandLine.append(QString(" -I %1").arg(m_ui->ioStatsSpinBox->value()));
    }
    
    if (m_ui->verboseCheckBox->isChecked()) { commandLine.append(" -V"); }

    if (m_ui->realTimeCheckBox->isChecked()) { commandLine.append(" --udprt"); }

#ifdef RT_AUDIO
    if (m_ui->typeComboBox->currentIndex() != HUB_SERVER
        && m_ui->backendComboBox->currentIndex() == 1) {
        commandLine.append(" --rtaudio");
        commandLine.append(
            QString(" --srate %1").arg(m_ui->sampleRateComboBox->currentText()));
        commandLine.append(
            QString(" --bufsize %1").arg(m_ui->bufferSizeComboBox->currentText()));
        QString inDevice;
        if(m_ui->inputDeviceComboBox->currentIndex() > 0) {
            inDevice = m_ui->inputDeviceComboBox->currentText();
        }
        QString outDevice;
        if(m_ui->outputDeviceComboBox->currentIndex() > 0) {
            outDevice = m_ui->outputDeviceComboBox->currentText();
        }
        commandLine.append(
            QString(" --audiodevice \"%1\",\"%2\"").arg(inDevice, outDevice));
    }
#endif

    return commandLine;
}

#ifdef RT_AUDIO
void QJackTrip::populateDeviceMenu(QComboBox* menu, bool isInput)
{
    RtAudio audio;
    QString previousString = menu->currentText();
    menu->clear();
    // std::cout << "previousString: " << previousString.toStdString() << std::endl;
    menu->addItem("(default)");
    unsigned int devices = audio.getDeviceCount();
    RtAudio::DeviceInfo info;
    for (unsigned int i=0; i<devices; i++) {
        info = audio.getDeviceInfo(i);
        if (info.probed == true) {
            if (isInput && info.inputChannels > 0) {
                menu->addItem(QString::fromStdString(info.name));
            } else if (!isInput && info.outputChannels > 0) {
                menu->addItem(QString::fromStdString(info.name));
            }
        }
    }
    // set the previous value
    menu->setCurrentText(previousString);
}
#endif

void QJackTrip::showCommandLineMessageBox()
{
    QMessageBox msgBox;
    QString messageText =
        QString("The equivalent command line for the current options is:\n\n%1")
            .arg(commandLineFromCurrentOptions());
    msgBox.setText(messageText);
    msgBox.setWindowTitle("Command Line");
    msgBox.setTextInteractionFlags(Qt::TextSelectableByMouse);
    msgBox.exec();
}

QJackTrip::~QJackTrip()
{
    //Restore cout. (Stops a crash on exit.)
    std::cout.rdbuf(m_realCout.rdbuf());
    std::cerr.rdbuf(m_realCerr.rdbuf());
}
