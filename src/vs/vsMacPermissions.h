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
 * \file vsMacPermissions.h
 * \author Matt Horton
 * \date Oct 2022
 */

#ifndef __VSMACPERMISSIONS_H__
#define __VSMACPERMISSIONS_H__

#include <objc/objc.h>

#include <QDebug>
#include <QObject>
#include <QString>

#include "vsPermissions.h"

class VsMacPermissions : public VsPermissions
{
    Q_OBJECT

   public:
    explicit VsMacPermissions();

    bool micPermissionChecked() override;
    Q_INVOKABLE void getMicPermission() override;
    Q_INVOKABLE void openSystemPrivacy();

   private:
    QString m_micPermission     = "unknown";
    bool m_micPermissionChecked = false;
};

#endif  // __VSMACPERMISSIONS_H__
