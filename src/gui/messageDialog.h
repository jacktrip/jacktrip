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
#include <QTemporaryFile>
#include <QTimer>

namespace Ui
{
class MessageDialog;
}

class MessageDialog : public QDialog
{
    Q_OBJECT

   public:
    explicit MessageDialog(QWidget* parent = nullptr);
    ~MessageDialog() override;

    void setStatsFile(QSharedPointer<QTemporaryFile> statsFile);
    void startMonitoring();
    void stopMonitoring();

   private slots:
    void writeOutput();

   private:
    QScopedPointer<Ui::MessageDialog> m_ui;
    QSharedPointer<QTemporaryFile> m_ioStatsFile;
    // Using a QFileSystem watcher didn't work on OS X, so use a timer instead.
    QTimer m_ioTimer;
};

#endif  // MESSAGEDIALOG_H
