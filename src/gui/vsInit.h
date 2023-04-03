//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2023 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.

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
 * \file vsInit.h
 * \author Aaron Wyatt, based on code by Matt Horton
 * \date February 2023
 */

#ifndef __VSINIT_H__
#define __VSINIT_H__

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QScopedPointer>

#include "virtualstudio.h"

class VsInit : public QObject
{
    Q_OBJECT

   public:
    VsInit() = default;

    static QString parseDeeplink(QCoreApplication* app);
    void checkForInstance(QString& deeplink);
    void setVs(QSharedPointer<VirtualStudio> vs) { m_vs = vs; }
#ifdef _WIN32
    static void setUrlScheme();
#endif

   private slots:
    void connectionReceived();
    void connectionFailed(QLocalSocket::LocalSocketError socketError);
    void responseReceived();

   private:
    QScopedPointer<QLocalServer> m_instanceServer;
    QScopedPointer<QLocalSocket> m_instanceCheckSocket;
    QSharedPointer<VirtualStudio> m_vs;
    QString m_deeplink;
};

#endif  // __VSINIT_H__
