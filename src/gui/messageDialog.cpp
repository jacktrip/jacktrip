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
#include <iostream>
#include <QScrollBar>
#include <QMenu>
#include <QSettings>

MessageDialog::MessageDialog(QWidget* parent, QString windowFunction, quint32 streamCount)
    : QDialog(parent)
    , m_ui(new Ui::MessageDialog)
    , m_outStreams(streamCount)
    , m_outBufs(streamCount)
    , m_windowFunction(windowFunction)
{
    m_ui->setupUi(this);
    for (int i = 0; i < streamCount; i++) {
        m_outBufs[i].reset(new textbuf);
        m_outStreams[i].reset(new std::ostream(m_outBufs.at(i).data()));
        connect(m_outBufs.at(i).data(), &textbuf::outputString, this, &MessageDialog::receiveOutput, Qt::QueuedConnection);
    }
    
    m_ui->messagesTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->messagesTextEdit, &QPlainTextEdit::customContextMenuRequested, this, &MessageDialog::provideContextMenu);
    
    if (!m_windowFunction.isEmpty()) {
        setWindowTitle(m_windowFunction);
    }
}

void MessageDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if (!m_windowFunction.isEmpty()) {
        QSettings settings;
        settings.beginGroup("Window");
        QByteArray geometry = settings.value(m_windowFunction + "Geometry").toByteArray();
        if (geometry.size() > 0) {
            restoreGeometry(geometry);
        }
    }
}

void MessageDialog::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);
    if (!m_windowFunction.isEmpty()) {
        QSettings settings;
        settings.beginGroup("Window");
        settings.setValue(m_windowFunction + "Geometry", saveGeometry());
        settings.endGroup();
    }
}

QSharedPointer<std::ostream> MessageDialog::getOutputStream(quint32 index)
{
    if (index < m_outStreams.size()) {
        return m_outStreams.at(index);
    }
    return QSharedPointer<std::ostream>();
}

bool MessageDialog::setRelayStream(std::ostream *relay, quint32 index)
{
    if (index < m_outBufs.size()) {
        m_outBufs.at(index)->setOutStream(relay);
    }
    return false;
}

void MessageDialog::clearOutput()
{
    m_ui->messagesTextEdit->clear();
}

void MessageDialog::receiveOutput(const QString& output)
{
    // Automatically scroll if we're at the bottom of the text box.
    bool autoScroll = (m_ui->messagesTextEdit->verticalScrollBar()->value() == m_ui->messagesTextEdit->verticalScrollBar()->maximum());
    // Make sure our cursor is at the end.
    m_ui->messagesTextEdit->moveCursor(QTextCursor::End);
    m_ui->messagesTextEdit->insertPlainText(output);
    if (autoScroll) {
        m_ui->messagesTextEdit->verticalScrollBar()->setValue(m_ui->messagesTextEdit->verticalScrollBar()->maximum());
    }
}

void MessageDialog::provideContextMenu()
{
    // Add a custom context menu entry to clear the output.
    QMenu *menu = m_ui->messagesTextEdit->createStandardContextMenu();
    QAction *action = menu->addAction(QIcon::fromTheme("edit-delete"), "Clear");
    connect(action, &QAction::triggered, this, &MessageDialog::clearOutput);
    menu->exec(QCursor::pos());
}

MessageDialog::~MessageDialog() = default;
