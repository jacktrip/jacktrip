//*****************************************************************

// Copyright (c) 2020 Aaron Wyatt.
// SPDX-FileCopyrightText: 2021 Aaron Wyatt
//
// SPDX-License-Identifier: GPL-3.0-or-later

//*****************************************************************

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QScopedPointer>
#include <QTextDocument>

namespace Ui
{
class About;
}

class About : public QDialog
{
    Q_OBJECT

   public:
    explicit About(QWidget* parent = nullptr);
    ~About() override;

    void resizeEvent(QResizeEvent* event) override;

    static const QString s_buildType;
    static const QString s_buildID;

   private:
    QScopedPointer<Ui::About> m_ui;
    QTextDocument aboutText;
};

#endif  // ABOUT_H
