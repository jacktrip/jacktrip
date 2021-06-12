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

#include "messageDialog.h"

#include "ui_messageDialog.h"

MessageDialog::MessageDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::MessageDialog), m_ioTimer(new QTimer(this))
{
    m_ui->setupUi(this);
    connect(&m_ioTimer, &QTimer::timeout, this, &MessageDialog::writeOutput);
    m_ioTimer.setInterval(1000);
}

void MessageDialog::setStatsFile(QSharedPointer<QTemporaryFile> statsFile)
{
    m_ui->messagesTextEdit->clear();
    m_ioStatsFile = statsFile;
}

void MessageDialog::startMonitoring() { m_ioTimer.start(); }

void MessageDialog::stopMonitoring() { m_ioTimer.stop(); }

void MessageDialog::writeOutput()
{
    while (!m_ioStatsFile->atEnd()) {
        m_ui->messagesTextEdit->moveCursor(QTextCursor::End);
        m_ui->messagesTextEdit->insertPlainText(m_ioStatsFile->readLine());
        (m_ioStatsFile->readLine());
    }
}

MessageDialog::~MessageDialog() = default;
