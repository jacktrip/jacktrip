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

#include <QDateTime>
#include <QMenu>
#include <QScrollBar>
#include <QSettings>
#include <iostream>

#include "ui_messageDialog.h"

MessageDialog::MessageDialog(QWidget* parent, const QString& windowFunction,
                             quint32 streamCount)
    : QDialog(parent)
    , m_ui(new Ui::MessageDialog)
    , m_outStreams(streamCount)
    , m_outBufs(streamCount)
    , m_windowFunction(windowFunction)
    , m_addTimeStamp(true)
    , m_timeStampFormat(QStringLiteral("hh:mm:ss: "))
    , m_startOfLine(true)
{
    m_ui->setupUi(this);
    for (quint32 i = 0; i < streamCount; i++) {
        m_outBufs[i].reset(new textbuf);
        m_outStreams[i].reset(new std::ostream(m_outBufs.at(i).data()));
        connect(m_outBufs.at(i).data(), &textbuf::outputString, this,
                &MessageDialog::receiveOutput, Qt::QueuedConnection);
    }

    m_ui->messagesTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_ui->messagesTextEdit, &QPlainTextEdit::customContextMenuRequested, this,
            &MessageDialog::provideContextMenu);
    m_ui->messagesTextEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    connect(this, &QDialog::rejected, this, &MessageDialog::savePosition);

    if (!m_windowFunction.isEmpty()) {
        setWindowTitle(m_windowFunction);
        if (m_windowFunction == QLatin1String("Stats")) {
            m_addTimeStamp = false;
        } else {
            // Create an indent for wrapped lines if we're adding a timestamp.
            // Because we're using a fixed width font we can just multiply our
            // timeStamp length by the average character width.
            QTextBlockFormat indent;
            QFontMetrics metrics(m_ui->messagesTextEdit->font());
            int marginWidth = metrics.averageCharWidth() * m_timeStampFormat.length();
            indent.setLeftMargin(marginWidth);
            indent.setTextIndent(-marginWidth);
            m_ui->messagesTextEdit->textCursor().setBlockFormat(indent);
        }
    }
}

void MessageDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    if (!m_windowFunction.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QStringLiteral("Window"));
        QByteArray geometry = settings.value(m_windowFunction + "Geometry").toByteArray();
        if (geometry.size() > 0) {
            restoreGeometry(geometry);
        }
    }
}

void MessageDialog::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);
    savePosition();
}

QSharedPointer<std::ostream> MessageDialog::getOutputStream(int index)
{
    if (index >= 0 && index < m_outStreams.size()) {
        return m_outStreams.at(index);
    }
    return QSharedPointer<std::ostream>();
}

bool MessageDialog::setRelayStream(std::ostream* relay, int index)
{
    if (index >= 0 && index < m_outBufs.size()) {
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
    if (output.isEmpty()) {
        return;
    }

    // Automatically scroll if we're at the bottom of the text box.
    int scrollLocation = (m_ui->messagesTextEdit->verticalScrollBar()->value());
    bool autoScroll =
        (scrollLocation == m_ui->messagesTextEdit->verticalScrollBar()->maximum());

    // Make sure our cursor is at the end.
    m_ui->messagesTextEdit->moveCursor(QTextCursor::End);

    if (m_addTimeStamp) {
        QString timeStamp = QDateTime::currentDateTime().toString(m_timeStampFormat);
        if (m_startOfLine) {
            m_ui->messagesTextEdit->insertPlainText(timeStamp);
        }
        if (output.indexOf(QChar('\n')) == -1) {
            m_ui->messagesTextEdit->insertPlainText(output);
        } else {
            QStringList lines = output.split(QChar('\n'));
            m_ui->messagesTextEdit->insertPlainText(
                QStringLiteral("%1\n").arg(lines.at(0)));
            int length = lines.length();
            if (output.endsWith(QChar('\n'))) {
                length--;
            }
            for (int i = 1; i < length; i++) {
                m_ui->messagesTextEdit->insertPlainText(
                    QStringLiteral("%1%2\n").arg(timeStamp, lines.at(i)));
            }
        }
        m_startOfLine = output.endsWith(QChar('\n'));
    } else {
        m_ui->messagesTextEdit->insertPlainText(output);
    }
    if (autoScroll) {
        m_ui->messagesTextEdit->verticalScrollBar()->setValue(
            m_ui->messagesTextEdit->verticalScrollBar()->maximum());
    } else {
        m_ui->messagesTextEdit->verticalScrollBar()->setValue(scrollLocation);
    }
}

void MessageDialog::provideContextMenu()
{
    // Add a custom context menu entry to clear the output.
    QMenu* menu     = m_ui->messagesTextEdit->createStandardContextMenu();
    QAction* action = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                      QStringLiteral("Clear"));
    connect(action, &QAction::triggered, this, &MessageDialog::clearOutput);
    menu->exec(QCursor::pos());
}

void MessageDialog::savePosition()
{
    if (!m_windowFunction.isEmpty()) {
        QSettings settings;
        settings.beginGroup(QStringLiteral("Window"));
        settings.setValue(m_windowFunction + "Geometry", saveGeometry());
        settings.endGroup();
    }
}

MessageDialog::~MessageDialog()
{
    if (isVisible()) {
        savePosition();
    }
}
