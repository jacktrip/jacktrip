//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2024 JackTrip Labs, Inc.

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
//*****************************************************************

/**
 * \file vsQuickView.h
 * \author Aaron Wyatt
 * \date March 2022
 */

#ifndef VSQUICKVIEW_H
#define VSQUICKVIEW_H

#include <QQuickView>
#ifdef Q_OS_MACOS
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#endif

class VsQuickView : public QQuickView
{
    Q_OBJECT

   public:
    VsQuickView(QWindow* parent = nullptr);
    bool event(QEvent* event) override;

   signals:
    void windowClose();
    void focusGained();
    void focusLost();

   private slots:
    void closeWindow();
};

#endif  // VSQUICKVIEW_H
