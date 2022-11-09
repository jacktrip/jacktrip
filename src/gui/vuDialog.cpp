//*****************************************************************
/*
  QJackTrip: Bringing a graphical user interface to JackTrip, a
  system for high quality audio network performance over the
  internet.

  Copyright (c) 2022 Aaron Wyatt.

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

#include "vuDialog.h"

#include <QSettings>
#include <QGridLayout>
#include <QLabel>
#include <QSpacerItem>

#include "ui_vuDialog.h"

VuDialog::VuDialog(QWidget* parent, quint32 inputChannels, quint32 outputChannels)
    : QDialog(parent)
    , m_ui(new Ui::VuDialog)
    , m_inputChannels(inputChannels)
    , m_outputChannels(outputChannels)
{
    m_ui->setupUi(this);
    
    // Setting the parent should set the ownership for all of these objects.
    // (They should be deleted when the window is.)
    QGridLayout *inputLayout = new QGridLayout(this);
    for (int i = 0; i < inputChannels; i++) {
        VuMeter *meter = new VuMeter(this);
        m_inputMeters.append(meter);
        QLabel *label = new QLabel(QString::number(i + 1), this);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        inputLayout->addWidget(label, i, 0, 1, 1);
        inputLayout->addWidget(meter, i, 1, 1, 1);
    }
    // Effectively add a spacer at the bottom.
    inputLayout->setRowStretch(inputChannels, 100);
    m_ui->inputGroupBox->setLayout(inputLayout);
    
    QGridLayout *outputLayout = new QGridLayout(this);
    for (int i = 0; i < outputChannels; i++) {
        VuMeter *meter = new VuMeter(this);
        m_outputMeters.append(meter);
        QLabel *label = new QLabel(QString::number(i + 1), this);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        outputLayout->addWidget(label, i, 0, 1, 1);
        outputLayout->addWidget(meter, i, 1, 1, 1);
    }
    outputLayout->setRowStretch(outputChannels, 100);
    m_ui->outputGroupBox->setLayout(outputLayout);
}

void VuDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    QSettings settings;
    settings.beginGroup(QStringLiteral("Window"));
    QByteArray geometry = settings.value(QStringLiteral("VuGeometry")).toByteArray();
    if (geometry.size() > 0) {
        restoreGeometry(geometry);
    }
}

void VuDialog::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);
    savePosition();
}

void VuDialog::updatedInputMeasurements(const QVector<float> valuesInDb)
{
    for (int i = 0; i < m_inputChannels; i++) {
        // Determine decibel reading
        qreal dB = m_meterMin;
        if (i < valuesInDb.size()) {
            dB = std::max(m_meterMin, valuesInDb.at(i));
        }

        // Produce a normalized value from 0 to 1
        float level = (dB - m_meterMin) / (m_meterMax - m_meterMin);
        m_inputMeters.at(i)->setLevel(level);
    }
}

void VuDialog::updatedOutputMeasurements(const QVector<float> valuesInDb)
{
    for (int i = 0; i < m_outputChannels; i++) {
        // Determine decibel reading
        qreal dB = m_meterMin;
        if (i < valuesInDb.size()) {
            dB = std::max(m_meterMin, valuesInDb.at(i));
        }

        // Produce a normalized value from 0 to 1
        float level = (dB - m_meterMin) / (m_meterMax - m_meterMin);
        m_outputMeters.at(i)->setLevel(level);
    }
}

void VuDialog::savePosition()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Window"));
    settings.setValue(QStringLiteral("VuGeometry"), saveGeometry());
    settings.endGroup();
}

VuDialog::~VuDialog()
{
    if (isVisible()) {
        savePosition();
    }
}
