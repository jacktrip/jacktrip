//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008-2022 Juan-Pablo Caceres, Chris Chafe.
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
 * \file vsServerInfo.h
 * \author Aaron Wyatt
 * \date March 2022
 */

#ifndef VSSERVERINFO_H
#define VSSERVERINFO_H

#include <QObject>

class VsServerInfo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString type READ type CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    // Q_PROPERTY(QString host READ host CONSTANT)
    Q_PROPERTY(bool canConnect READ canConnect NOTIFY canConnectChanged)
    Q_PROPERTY(bool canStart READ canStart CONSTANT)
    // Q_PROPERTY(quint16 port READ port CONSTANT)
    Q_PROPERTY(bool isPublic READ isPublic CONSTANT)
    Q_PROPERTY(QString flag READ flag CONSTANT)
    Q_PROPERTY(QString location READ location CONSTANT)
    Q_PROPERTY(bool isManageable READ isManageable CONSTANT)
    Q_PROPERTY(quint16 period READ period CONSTANT)
    Q_PROPERTY(quint32 sampleRate READ sampleRate CONSTANT)
    Q_PROPERTY(quint16 queueBuffer READ queueBuffer CONSTANT)

   public:
    enum serverSectionT { YOUR_STUDIOS, SUBSCRIBED_STUDIOS, PUBLIC_STUDIOS };

    explicit VsServerInfo(QObject* parent = nullptr);
    ~VsServerInfo() override;

    serverSectionT section();
    QString type();
    void setSection(serverSectionT section);
    QString name();
    void setName(const QString& name);
    QString host();
    bool canConnect();
    bool canStart();
    void setHost(const QString& host);
    quint16 port();
    void setPort(quint16 port);
    bool isPublic();
    void setIsPublic(bool isPublic);
    QString region();
    QString flag();
    QString location();
    void setRegion(const QString& region);
    bool isManageable();
    void setIsManageable(bool isManageable);
    quint16 period();
    void setPeriod(quint16 period);
    quint32 sampleRate();
    void setSampleRate(quint32 sampleRate);
    quint16 queueBuffer();
    void setQueueBuffer(quint16 queueBuffer);
    QString id();
    void setId(const QString& id);

   signals:
    void canConnectChanged();

   private:
    serverSectionT m_section = PUBLIC_STUDIOS;
    QString m_name;
    QString m_host;
    quint16 m_port;
    bool m_isPublic;
    QString m_region;
    bool m_isManageable;
    quint16 m_period;
    quint32 m_sampleRate;
    quint16 m_queueBuffer;
    QString m_id;

    /* Remaining JSON fields
    "loopback": true,
    "stereo": true,
    "type": "JackTrip",
    "managed": true,
    "size": "c5.large",
    "mixBranch": "main",
    "mixCode": "SimpleMix(~maxClients).masterVolume_(1).connect.start;",
    "enabled": true,
    "admin": true,
    "cloudId": "string",
    "owner": true,
    "ownerId": "string",
    "status": "Ready",
    "sessionId": "1636042722abcdefg",
    "subStatus": "Active",
    "createdAt": "2021-09-07T17:15:38Z",
    "expiresAt": "2021-09-07T17:15:38Z",
    "updatedAt": "2021-09-07T17:15:38Z"
    */
};

#endif  // VSSERVERINFO_H
