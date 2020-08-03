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
#include "ui_qjacktrip.h"
#include "about.h"
#include <QMessageBox>
#include <QSettings>

QJackTrip::QJackTrip(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::QJackTrip),
    m_netManager(new QNetworkAccessManager(this)),
    m_jackTripRunning(false),
    m_isExiting(false)
{
    m_ui->setupUi(this);
    
    connect(m_ui->typeComboBox, &QComboBox::currentTextChanged, this, &QJackTrip::chooseRunType);
    connect(m_ui->addressEdit, &QLineEdit::textChanged, this, &QJackTrip::addressChanged);
    connect(m_ui->connectButton, &QPushButton::released, this, &QJackTrip::start);
    connect(m_ui->disconnectButton, &QPushButton::released, this, &QJackTrip::stop);
    connect(m_ui->exitButton, &QPushButton::released, this, &QJackTrip::exit);
    connect(m_ui->useDefaultsButton, &QPushButton::released, this, &QJackTrip::resetOptions);
    connect(m_ui->aboutButton, &QPushButton::released, this, [=](){
            About about(this);
            about.exec();
        } );
    m_ui->autoPatchComboBox->setVisible(false);
    m_ui->autoPatchLabel->setVisible(false);
    
    connect(m_netManager.data(), &QNetworkAccessManager::finished, this, &QJackTrip::receivedIP);
    //Use the ipify API to find our external IP address.
    m_netManager->get(QNetworkRequest(QUrl("https://api.ipify.org")));
    m_ui->statusBar->showMessage(QString("QJackTrip version ").append(gVersion));
    
    loadSettings();
}

void QJackTrip::closeEvent(QCloseEvent *event)
{
    //Ignore the close event so that we can override the handling of it.
    event->ignore();
    exit();
}

void QJackTrip::processFinished()
{
    if (!m_jackTripRunning) {
        return;
    }
    m_jackTripRunning = false;
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentText() == "Hub Server") {
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
    msgBox.setText(QString("Error: ").append(errorMessage));
    msgBox.setWindowTitle("Doh!");
    msgBox.exec();
    processFinished();
}

void QJackTrip::receivedConnectionFromPeer()
{
    m_ui->statusBar->showMessage("Received Connection from Peer!");
}

void QJackTrip::udpWaitingTooLong()
{
    m_ui->statusBar->showMessage("UDP waiting too long (more than 30ms)", 1000);
}

void QJackTrip::chooseRunType(const QString &type)
{
    //Update ui to reflect choice of run mode.
    if (type.endsWith("Client")) {
        m_ui->addressEdit->setEnabled(true);
        m_ui->addressLabel->setEnabled(true);
        m_ui->connectButton->setEnabled(!m_ui->addressEdit->text().isEmpty());
        m_ui->connectButton->setText("Connect");
        m_ui->disconnectButton->setText("Disconnect");
    } else {
        m_ui->addressEdit->setEnabled(false);
        m_ui->addressLabel->setEnabled(false);
        m_ui->connectButton->setText("Start");
        m_ui->disconnectButton->setText("Stop");
        m_ui->connectButton->setEnabled(true);
    }
    
    if (type == "Hub Server") {
        m_ui->channelSpinBox->setVisible(false);
        m_ui->channelLabel->setVisible(false);
        m_ui->autoPatchComboBox->setVisible(true);
        m_ui->autoPatchLabel->setVisible(true);
        advancedOptionsForHubServer(true);
    } else {
        m_ui->autoPatchComboBox->setVisible(false);
        m_ui->autoPatchLabel->setVisible(false);
        m_ui->channelSpinBox->setVisible(true);
        m_ui->channelLabel->setVisible(true);
        advancedOptionsForHubServer(false);
    }

    if (type == "Hub Client") {
        m_ui->remoteNameEdit->setVisible(true);
        m_ui->remoteNameLabel->setVisible(true);
    } else {
        m_ui->remoteNameEdit->setVisible(false);
        m_ui->remoteNameLabel->setVisible(false);
    }
}

void QJackTrip::addressChanged(const QString &address)
{
    if (m_ui->typeComboBox->currentText().endsWith("Client")) {
        m_ui->connectButton->setEnabled(!address.isEmpty());
    }
}

void QJackTrip::receivedIP(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        m_ui->ipLabel->setText("Unable to determine external IP address.");
    } else {
        QByteArray address = reply->readAll();
        m_ui->ipLabel->setText(QString("External IP address: ").append(address));
        m_ui->ipLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
    reply->deleteLater();
}

void QJackTrip::resetOptions()
{
    //Reset our basic options
    m_ui->channelSpinBox->setValue(2);
    m_ui->autoPatchComboBox->setCurrentIndex(0);
    m_ui->zeroCheckBox->setChecked(false);
    
    //Then advanced options
    m_ui->clientNameEdit->setText("");
    m_ui->remoteNameEdit->setText("");
    m_ui->portOffsetSpinBox->setValue(0);
    m_ui->queueLengthSpinBox->setValue(gDefaultQueueLength);
    m_ui->redundancySpinBox->setValue(1);
    m_ui->resolutionComboBox->setCurrentIndex(1);
    m_ui->connectAudioCheckBox->setChecked(true);
    
    saveSettings();
}

void QJackTrip::start()
{
    m_ui->connectButton->setEnabled(false);
    enableUi(false);
    m_jackTripRunning = true;
    
    //Start the appropriate JackTrip process.
    try {
        if (m_ui->typeComboBox->currentText() == "Hub Server") {
            m_udpHub.reset(new UdpHubListener());
            int hubConnectionMode = m_ui->autoPatchComboBox->currentIndex();
            if (hubConnectionMode > 2) {
                //Adjust for the RESERVEDMATRIX gap.
                hubConnectionMode++;
            }
            
            m_udpHub->setHubPatch(hubConnectionMode);
            if (hubConnectionMode == JackTrip::NOAUTO) {
                m_udpHub->setConnectDefaultAudioPorts(false);
            } else {
                m_udpHub->setConnectDefaultAudioPorts(true);
            }
            
            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_udpHub->setUnderRunMode(JackTrip::ZEROS);
            }
            m_udpHub->setBufferQueueLength(m_ui->queueLengthSpinBox->value());
            
            QObject::connect(m_udpHub.data(), &UdpHubListener::signalStopped, this, &QJackTrip::processFinished,
                             Qt::QueuedConnection);
            QObject::connect(m_udpHub.data(), &UdpHubListener::signalError, this, &QJackTrip::processError, 
                             Qt::QueuedConnection);
            m_ui->disconnectButton->setEnabled(true);
            m_udpHub->start();
            m_ui->statusBar->showMessage("Hub Server Started");
        } else {
            JackTrip::jacktripModeT jackTripMode;
            if (m_ui->typeComboBox->currentText() == "Client") {
                jackTripMode = JackTrip::CLIENT;
            } else if (m_ui->typeComboBox->currentText() == "Server") {
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
            
            m_jackTrip.reset(new JackTrip(jackTripMode, JackTrip::UDP, m_ui->channelSpinBox->value(),
                                          m_ui->queueLengthSpinBox->value(), m_ui->redundancySpinBox->value(),
                                          resolution));
            m_jackTrip->setConnectDefaultAudioPorts(true);
            if (m_ui->zeroCheckBox->isChecked()) {
                // Set buffers to zero when underrun
                m_jackTrip->setUnderRunMode(JackTrip::ZEROS);
            }
            
            // Set peer address in client mode
            if (jackTripMode == JackTrip::CLIENT || jackTripMode == JackTrip::CLIENTTOPINGSERVER) {
                m_jackTrip->setPeerAddress(m_ui->addressEdit->text());
                if (jackTripMode == JackTrip::CLIENTTOPINGSERVER && !m_ui->remoteNameEdit->text().isEmpty()) {
                    m_jackTrip->setRemoteClientName(m_ui->remoteNameEdit->text());
                }
            }
            
            m_jackTrip->setBindPorts(gDefaultPort + m_ui->portOffsetSpinBox->value());
            m_jackTrip->setPeerPorts(gDefaultPort + m_ui->portOffsetSpinBox->value());
            
            if (!m_ui->clientNameEdit->text().isEmpty()) {
                m_jackTrip->setClientName(m_ui->clientNameEdit->text());
            }
            
            QObject::connect(m_jackTrip.data(), &JackTrip::signalProcessesStopped, this, &QJackTrip::processFinished, 
                             Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalError, this, &QJackTrip::processError, 
                             Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalReceivedConnectionFromPeer, this,
                             &QJackTrip::receivedConnectionFromPeer, Qt::QueuedConnection);
            QObject::connect(m_jackTrip.data(), &JackTrip::signalUdpWaitingTooLong, this,
                             &QJackTrip::udpWaitingTooLong);
            m_ui->statusBar->showMessage("Waiting for Peer...");
            m_ui->disconnectButton->setEnabled(true);
#ifdef WAIRTOHUB // WAIR
            m_jackTrip->startProcess(0); // for WAIR compatibility, ID in jack client name
#else
            m_jackTrip->startProcess();
#endif // endwhere
        }
    } catch(const std::exception &e) {
        //Let the user know what our exception was.
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
}

void QJackTrip::stop()
{
    m_ui->disconnectButton->setEnabled(false);
    if (m_ui->typeComboBox->currentText() == "Hub Server") {
        m_udpHub->stop();
    } else {
        m_jackTrip->stop();
    }
}

void QJackTrip::exit()
{
    //Only run this once.
    if (m_isExiting) {
        return;
    }
    m_isExiting = true;
    m_ui->exitButton->setEnabled(false);
    saveSettings();
    if (m_jackTripRunning) {
        stop();
    } else {
        emit signalExit();
    }
}

void QJackTrip::enableUi(bool enabled)
{
    m_ui->optionsTabWidget->setEnabled(enabled);
    m_ui->typeLabel->setEnabled(enabled);
    m_ui->typeComboBox->setEnabled(enabled);
    m_ui->addressLabel->setEnabled(enabled && m_ui->typeComboBox->currentText().endsWith("Client"));
    m_ui->addressEdit->setEnabled(enabled && m_ui->typeComboBox->currentText().endsWith("Client"));
}

void QJackTrip::advancedOptionsForHubServer(bool isHubServer)
{
    m_ui->clientNameLabel->setVisible(!isHubServer);
    m_ui->clientNameEdit->setVisible(!isHubServer);
    m_ui->portOffsetLabel->setVisible(!isHubServer);
    m_ui->portOffsetSpinBox->setVisible(!isHubServer);
    m_ui->redundancyLabel->setVisible(!isHubServer);
    m_ui->redundancySpinBox->setVisible(!isHubServer);
    m_ui->resolutionLabel->setVisible(!isHubServer);
    m_ui->resolutionComboBox->setVisible(!isHubServer);
    m_ui->connectAudioCheckBox->setVisible(!isHubServer);
}

void QJackTrip::loadSettings()
{
#ifdef __MAC_OSX__
    QSettings settings("psi-borg.org", "QJackTrip");
#else
    QSettings settings("psi-borg", "QJackTrip");
#endif
    m_ui->typeComboBox->setCurrentIndex(settings.value("RunMode", 0).toInt());
    m_ui->addressEdit->setText(settings.value("LastAddress", "").toString());
    m_ui->channelSpinBox->setValue(settings.value("Channels", 2).toInt());
    m_ui->autoPatchComboBox->setCurrentIndex(settings.value("AutoPatchMode", 0).toInt());
    m_ui->zeroCheckBox->setChecked(settings.value("ZeroUnderrun", false).toBool());
    m_ui->clientNameEdit->setText(settings.value("ClientName", "").toString());
    m_ui->remoteNameEdit->setText(settings.value("RemoteName", "").toString());
    m_ui->portOffsetSpinBox->setValue(settings.value("PortOffset", 0).toInt());
    m_ui->queueLengthSpinBox->setValue(settings.value("QueueLength", 4).toInt());
    m_ui->redundancySpinBox->setValue(settings.value("Redundancy", 1).toInt());
    m_ui->resolutionComboBox->setCurrentIndex(settings.value("resolution", 1).toInt());
    m_ui->connectAudioCheckBox->setChecked(settings.value("ConnectAudio", true).toBool());
}

void QJackTrip::saveSettings()
{
#ifdef __MAC_OSX__
    QSettings settings("psi-borg.org", "QJackTrip");
#else
    QSettings settings("psi-borg", "QJackTrip");
#endif
    settings.setValue("RunMode", m_ui->typeComboBox->currentIndex());
    settings.setValue("LastAddress", m_ui->addressEdit->text());
    settings.setValue("Channels", m_ui->channelSpinBox->value());
    settings.setValue("AutoPatchMode", m_ui->autoPatchComboBox->currentIndex());
    settings.setValue("ZeroUnderrun", m_ui->zeroCheckBox->isChecked());
    settings.setValue("ClientName", m_ui->clientNameEdit->text());
    settings.setValue("RemoteName", m_ui->remoteNameEdit->text());
    settings.setValue("PortOffset", m_ui->portOffsetSpinBox->value());
    settings.setValue("QueueLength", m_ui->queueLengthSpinBox->value());
    settings.setValue("Redundancy", m_ui->redundancySpinBox->value());
    settings.setValue("Resolution", m_ui->resolutionComboBox->currentIndex());
    settings.setValue("ConnectAudio", m_ui->connectAudioCheckBox->isChecked());
}

QJackTrip::~QJackTrip() = default;
