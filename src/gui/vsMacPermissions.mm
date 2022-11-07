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
 * \file vsMacPermissions.mm
 * \author Matt Horton
 * \date Oct 2022
 */

#include "vsMacPermissions.h"
#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>
#include <QDesktopServices>
#include <QUrl>

QString VsMacPermissions::micPermission()
{
    return m_micPermission;
}

void VsMacPermissions::getMicPermission()
{
    // Request permission to access.
    switch ([AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio])
    {
        case AVAuthorizationStatusAuthorized:
        {
            // The user has previously granted access.
            setMicPermission(QStringLiteral("granted"));
            break;
        }
        case AVAuthorizationStatusNotDetermined:
        {
            // The app hasn't yet asked the user for access.
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
                if (granted) {
                    //do something eventually
                    if (granted) {
                        setMicPermission(QStringLiteral("granted"));
                    } else {
                        setMicPermission(QStringLiteral("denied"));
                    }
                }
            }];
            setMicPermission(QStringLiteral("unknown"));
            break;
        }
        case AVAuthorizationStatusDenied:
        {
            // The user has previously denied access.
            setMicPermission(QStringLiteral("denied"));
        }
        case AVAuthorizationStatusRestricted:
        {
            // The user can't grant access due to restrictions.
            setMicPermission(QStringLiteral("denied"));
        }
    }
}

void VsMacPermissions::setMicPermission(QString status)
{
    m_micPermission = status;
    emit micPermissionUpdated();
}

void VsMacPermissions::openSystemPrivacy()
{
    QDesktopServices::openUrl(QUrl("x-apple.systempreferences:com.apple.preference.security?Privacy_Microphone"));
}
