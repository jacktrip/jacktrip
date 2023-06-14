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

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include <QDialog>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QVector>

#include "textbuf.h"

namespace Ui
{
class MessageDialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT

   public:
    explicit MessageDialog(QWidget* parent               = nullptr,
                           const QString& windowFunction = QLatin1String(""),
                           quint32 streamCount           = 1);
    ~MessageDialog() override;

    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    QSharedPointer<std::ostream> getOutputStream(int index = 0);
    bool setRelayStream(std::ostream* relay, int index = 0);

   public slots:
    void clearOutput();

   private slots:
    void receiveOutput(const QString& output);
    void provideContextMenu();
    void savePosition();

   private:
    QScopedPointer<Ui::MessageDialog> m_ui;
    QVector<QSharedPointer<std::ostream>> m_outStreams;
    QVector<QSharedPointer<textbuf>> m_outBufs;
    QString m_windowFunction;
    bool m_addTimeStamp;
    QString m_timeStampFormat;
    bool m_startOfLine;
};

#endif  // MESSAGEDIALOG_H
