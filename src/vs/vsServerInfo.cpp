//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2022-2025 JackTrip Labs, Inc.

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
 * \file vsServerInfo.cpp
 * \author Aaron Wyatt
 * \date March 2022
 */

#include "vsServerInfo.h"

VsServerInfo::VsServerInfo(QObject* parent) : QObject(parent) {}

VsServerInfo& VsServerInfo::operator=(const VsServerInfo& info)
{
    m_section     = info.m_section;
    m_name        = info.m_name;
    m_host        = info.m_host;
    m_port        = info.m_port;
    m_enabled     = info.m_enabled;
    m_owner       = info.m_owner;
    m_admin       = info.m_admin;
    m_isManaged   = info.m_isManaged;
    m_isPublic    = info.m_isPublic;
    m_region      = info.m_region;
    m_period      = info.m_period;
    m_sampleRate  = info.m_sampleRate;
    m_queueBuffer = info.m_queueBuffer;
    m_bannerURL   = info.m_bannerURL;
    m_id          = info.m_id;
    m_sessionId   = info.m_sessionId;
    m_streamId    = info.m_streamId;
    m_status      = info.m_status;
    m_cloudId     = info.m_cloudId;
    m_inviteKey   = info.m_inviteKey;
    return *this;
}

VsServerInfo::serverSectionT VsServerInfo::section()
{
    return m_section;
}

QString VsServerInfo::type() const
{
    if (m_section == YOUR_STUDIOS) {
        return QStringLiteral("Your Studios");
    } else if (m_section == SUBSCRIBED_STUDIOS) {
        return QStringLiteral("Subscribed Studios");
    } else {
        return QStringLiteral("Public Studios");
    }
}

void VsServerInfo::setSection(serverSectionT section)
{
    m_section = section;
}

QString VsServerInfo::name() const
{
    return m_name;
}

void VsServerInfo::setName(const QString& name)
{
    m_name = name;
}

QString VsServerInfo::host() const
{
    return m_host;
}

QString VsServerInfo::status() const
{
    return m_status;
}

bool VsServerInfo::canConnect() const
{
    return m_status == "Ready";
}

bool VsServerInfo::canStart() const
{
    return m_owner || m_admin;
}

void VsServerInfo::setHost(const QString& host)
{
    m_host = host;
}

void VsServerInfo::setStatus(const QString& status)
{
    m_status = status;
}

quint16 VsServerInfo::port() const
{
    return m_port;
}

void VsServerInfo::setPort(quint16 port)
{
    m_port = port;
}

bool VsServerInfo::enabled() const
{
    return m_enabled;
}

void VsServerInfo::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool VsServerInfo::isOwner() const
{
    return m_owner;
}

void VsServerInfo::setIsOwner(bool owner)
{
    m_owner = owner;
}

bool VsServerInfo::isAdmin() const
{
    return m_admin;
}

void VsServerInfo::setIsAdmin(bool admin)
{
    m_admin = admin;
}

bool VsServerInfo::isPublic() const
{
    return m_isPublic;
}

void VsServerInfo::setIsPublic(bool isPublic)
{
    m_isPublic = isPublic;
}

QString VsServerInfo::region() const
{
    return m_region;
}

QString VsServerInfo::flag() const
{
    QStringList parts = m_region.split(QStringLiteral("-"));
    if (parts.count() > 1) {
        QString countryCode = parts.at(1).toUpper();
        if (countryCode == QStringLiteral("TF")) {
            countryCode = QStringLiteral("TW");
        }
        return QStringLiteral("flags/%1.svg").arg(countryCode);
    }
    // Have a fallback here
    return QStringLiteral("flags/US.svg");
}

QString VsServerInfo::location() const
{
    return m_region;
}

void VsServerInfo::setRegion(const QString& region)
{
    m_region = region;
}

bool VsServerInfo::isManaged() const
{
    return m_isManaged;
}

void VsServerInfo::setIsManaged(bool isManaged)
{
    m_isManaged = isManaged;
}

quint16 VsServerInfo::period() const
{
    return m_period;
}

void VsServerInfo::setPeriod(quint16 period)
{
    m_period = period;
}

quint32 VsServerInfo::sampleRate() const
{
    return m_sampleRate;
}

void VsServerInfo::setSampleRate(quint32 sampleRate)
{
    m_sampleRate = sampleRate;
}

quint16 VsServerInfo::queueBuffer() const
{
    return m_queueBuffer;
}

void VsServerInfo::setQueueBuffer(quint16 queueBuffer)
{
    m_queueBuffer = queueBuffer;
}

QString VsServerInfo::bannerURL() const
{
    return m_bannerURL;
}

void VsServerInfo::setBannerURL(const QString& bannerURL)
{
    m_bannerURL = bannerURL;
}

QString VsServerInfo::id() const
{
    return m_id;
}

void VsServerInfo::setId(const QString& id)
{
    m_id = id;
}

QString VsServerInfo::sessionId() const
{
    return m_sessionId;
}

void VsServerInfo::setSessionId(const QString& sessionId)
{
    m_sessionId = (sessionId == "undefined") ? "" : sessionId;
}

QString VsServerInfo::streamId() const
{
    return m_streamId;
}

void VsServerInfo::setStreamId(const QString& streamId)
{
    m_streamId = (streamId == "undefined") ? "" : streamId;
}

QString VsServerInfo::inviteKey() const
{
    return m_inviteKey;
}

void VsServerInfo::setInviteKey(const QString& inviteKey)
{
    m_inviteKey = inviteKey;
}

QString VsServerInfo::cloudId() const
{
    return m_cloudId;
}

void VsServerInfo::setCloudId(const QString& cloudId)
{
    m_cloudId = cloudId;
}

bool VsServerInfo::operator<(const VsServerInfo& other) const
{
    if (status() == QStringLiteral("Ready")) {
        if (other.status() != QStringLiteral("Ready")) {
            return true;
        }
    } else if (other.status() == QStringLiteral("Ready")) {
        return false;
    }
    return name() < other.name();
}

void VsServerInfo::clear()
{
    setId("");
    setHost("");
    setPort(0);
    setSessionId("");
}

VsServerInfo::~VsServerInfo() = default;
