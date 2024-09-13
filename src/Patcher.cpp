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
 * \file Patcher.cpp
 * \author Aaron Wyatt
 * \date September 2020
 */

#include "Patcher.h"

#include <QVector>

void Patcher::setPatchMode(JackTrip::hubConnectionModeT patchMode)
{
    QMutexLocker locker(&m_connectionMutex);
    m_fan = patchMode == JackTrip::CLIENTFOFI || patchMode == JackTrip::FULLMIX
            || patchMode == JackTrip::SERVFOFI || patchMode == JackTrip::SERVFULLMIX;
    m_loop = patchMode == JackTrip::CLIENTECHO || patchMode == JackTrip::FULLMIX
             || patchMode == JackTrip::SERVFULLMIX;
    m_includeServer = patchMode == JackTrip::SERVERTOCLIENT
                      || patchMode == JackTrip::SERVFOFI
                      || patchMode == JackTrip::SERVFULLMIX;
}

void Patcher::setStereoUpmix(bool upmix)
{
    m_steroUpmix = upmix;
}

void Patcher::registerClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);

    // this works around a JACK timing bug found under pipewire
    // if registerClient is called for a second (or subsequent) hub client
    // jack_client won't have properly updated its ports
    // the workaround is to sleep here and let JACK update
    if (m_jackClient)
        QThread::msleep(100);

    // If our jack client isn't running, start it.
    if (!m_jackClient) {
        m_jackClient = jack_client_open("jthubpatcher", JackNoStartServer, &m_status);
        if (!m_jackClient) {
            std::cout << "Unable to start patcher JACK client: Patching disabled\n"
                      << std::endl;
            return;
        } else {
            jack_on_shutdown(m_jackClient, &Patcher::shutdownCallback, this);
        }
    }

    const char **outPorts, **inPorts;
    outPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsOutput);
    inPorts  = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsInput);

    // Find the ports belonging to our client.
    QVector<const char*> clientOutPorts;
    QVector<const char*> clientInPorts;

    for (int i = 0; outPorts[i]; i++) {
        // Exclude broadcast ports.
        if (QString(outPorts[i]).section(QStringLiteral(":"), 0, 0) == clientName
            && !QString(outPorts[i]).contains(QLatin1String("broadcast"))) {
            clientOutPorts.append(outPorts[i]);
        }
    }

    for (int i = 0; inPorts[i]; i++) {
        if (QString(inPorts[i]).section(QStringLiteral(":"), 0, 0) == clientName) {
            clientInPorts.append(inPorts[i]);
        }
    }

    bool clientIsMono = (clientOutPorts.count() == 1);

    if (m_includeServer && clientIsMono && m_steroUpmix) {
        // Most connections in server to client modes are created already by the code in
        // JackAudioInterface. We only need to handle any upmixing of mono clients here.
        const char** systemInPorts = jack_get_ports(m_jackClient, NULL, NULL,
                                                    JackPortIsPhysical | JackPortIsInput);
        if (systemInPorts[0] && systemInPorts[1]) {
            jack_connect(m_jackClient, clientOutPorts.at(0), systemInPorts[1]);
        }
        jack_free(systemInPorts);
    }

    if (m_fan || m_loop) {
        // Start with our receiving ports.
        for (int i = 0; i < clientOutPorts.count(); i++) {
            QString channel =
                QString(clientOutPorts.at(i)).section(QStringLiteral("_"), -1, -1);
            for (int j = 0; inPorts[j]; j++) {
                QString otherClient =
                    QString(inPorts[j]).section(QStringLiteral(":"), 0, 0);
                QString otherChannel =
                    QString(inPorts[j]).section(QStringLiteral("_"), -1, -1);

                // First check if this is one of our other clients. (Fan out/in and full
                // mix.)
                if (m_fan) {
                    if (m_clients.contains(otherClient) && otherChannel == channel) {
                        jack_connect(m_jackClient, clientOutPorts.at(i), inPorts[j]);
                    } else if (m_steroUpmix && clientIsMono) {
                        // Deal with the special case of stereo upmix
                        if (m_clients.contains(otherClient)
                            && otherChannel == QLatin1String("2")) {
                            jack_connect(m_jackClient, clientOutPorts.at(i), inPorts[j]);
                        }
                    }
                }

                // Then check if it's our registering client. (Client Echo and full mix.)
                if (m_loop) {
                    if (otherClient == clientName && otherChannel == channel) {
                        jack_connect(m_jackClient, clientOutPorts.at(i), inPorts[j]);
                    } else if (m_steroUpmix && clientIsMono) {
                        if (otherClient == clientName
                            && otherChannel == QLatin1String("2")) {
                            jack_connect(m_jackClient, clientOutPorts.at(i), inPorts[j]);
                        }
                    }
                }
            }
        }

        // Then our sending ports. We only need to check for other clients here.
        // (Any loopback connections will have been made in the previous loop.)
        if (m_fan) {
            for (int i = 0; i < clientInPorts.count(); i++) {
                QString channel =
                    QString(clientInPorts.at(i)).section(QStringLiteral("_"), -1, -1);
                for (int j = 0; outPorts[j]; j++) {
                    QString otherClient =
                        QString(outPorts[j]).section(QStringLiteral(":"), 0, 0);
                    QString otherChannel =
                        QString(outPorts[j]).section(QStringLiteral("_"), -1, -1);
                    if (m_clients.contains(otherClient)
                        && !QString(outPorts[j]).contains(QLatin1String("broadcast"))) {
                        if (otherChannel == channel
                            || (m_steroUpmix && channel == QLatin1String("2")
                                && m_monoClients.contains(otherClient))) {
                            jack_connect(m_jackClient, outPorts[j], clientInPorts.at(i));
                        }
                    }
                }
            }
        }
    }

    m_clients.append(clientName);
    if (clientIsMono) {
        m_monoClients.append(clientName);
    }
    jack_free(outPorts);
    jack_free(inPorts);
}

void Patcher::unregisterClient(const QString& clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    m_clients.removeAll(clientName);
    m_monoClients.removeAll(clientName);
}

void Patcher::shutdownCallback(void* arg)
{
    Patcher* patcher = static_cast<Patcher*>(arg);
    jack_client_close(patcher->m_jackClient);
    patcher->m_jackClient = nullptr;
}

Patcher::~Patcher()
{
    if (m_jackClient) {
        jack_client_close(m_jackClient);
    }
}
