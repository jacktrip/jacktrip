//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsPermissions.h
 * \author Matt Horton
 * \date Nov 2022
 */

#ifndef __VSPERMISSIONS_H__
#define __VSPERMISSIONS_H__

#include <QDebug>
#include <QObject>
#include <QString>

class VsPermissions : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString micPermission READ micPermission NOTIFY micPermissionUpdated)

   public:
    VsPermissions() = default;  // define here and there

    QString micPermission();              // define here
    virtual bool micPermissionChecked();  // define here and there
    Q_INVOKABLE virtual void getMicPermission();
    void setMicPermission(QString status);  // define here

   signals:
    void micPermissionUpdated();  // leave here

   protected:
#if __APPLE__
    QString m_micPermission     = "unknown";
    bool m_micPermissionChecked = false;
#else
    QString m_micPermission     = "granted";
    bool m_micPermissionChecked = true;
#endif
};

#endif  // __VSPERMISSIONS_H__
