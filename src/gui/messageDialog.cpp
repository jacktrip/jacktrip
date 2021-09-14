//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
