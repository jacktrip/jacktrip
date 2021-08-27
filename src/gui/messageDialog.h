//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
