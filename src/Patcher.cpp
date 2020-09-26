//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2008 Juan-Pablo Caceres, Chris Chafe.
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

Patcher::Patcher() :
    m_patchMode(JackTrip::SERVERTOCLIENT)
{
    m_jackClient = jack_client_open("jthubpatcher", JackNullOption, &m_status);
    if (m_jackClient == NULL) {
        std::cout << "Unable to start patcher JACK client: Patching disabled\n" << std::endl;
    }
}

void Patcher::setPatchMode(JackTrip::hubConnectionModeT patchMode)
{
    //If our jack client didn't open, keeping this set to the default value will disable patching.
    if (m_jackClient) {
        m_patchMode = patchMode;
    }
}

void Patcher::registerClient(const QString &clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    
    if (!(m_patchMode == JackTrip::CLIENTECHO || m_patchMode == JackTrip::CLIENTFOFI || m_patchMode == JackTrip::FULLMIX)) {
        //Nothing to do here other than track our clients.
        m_clients.append(clientName);
        return;
    }
    
    const char **outPorts, **inPorts;
    outPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsOutput);
    inPorts = jack_get_ports(m_jackClient, NULL, NULL, JackPortIsInput);
    
    //Start with our receiving ports.
    for (int i = 0; outPorts[i]; i++) {
        QString client = QString(outPorts[i]).section(":", 0, 0);
        if (client == clientName) {
            QString channel = QString(outPorts[i]).section("_", -1, -1);
            for (int j = 0; inPorts[j]; j++) {
                //First check if this is one of our other clients. (Fan out/in and full mix.)
                if (m_patchMode == JackTrip::CLIENTFOFI || m_patchMode == JackTrip::FULLMIX) {
                    if (m_clients.contains(QString(inPorts[j]).section(":", 0, 0)) 
                        && QString(inPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[i], inPorts[j]);
                    }
                }
                //Then check if it's our registering client. (Client Echo and full mix.)
                if (m_patchMode == JackTrip::CLIENTECHO || m_patchMode == JackTrip::FULLMIX) {
                    if (QString(inPorts[j]).section(":", 0, 0) == clientName 
                        && QString(inPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[i], inPorts[j]);
                    }
                }
            }
        }
    }
    
    //Then our sending ports. We only need to check for other clients here.
    //(Any loopback connections will have been made in the previous loop.)
    if (m_patchMode == JackTrip::CLIENTECHO || m_patchMode == JackTrip::FULLMIX) {
        for (int i = 0; inPorts[i]; i++) {
            QString client = QString(inPorts[i]).section(":", 0, 0);
            if (client == clientName) {
                QString channel = QString(inPorts[i]).section("_", -1, -1);
                for (int j = 0; outPorts[j]; j++) {
                    if (m_clients.contains(QString(outPorts[j]).section(":", 0, 0)) 
                        && QString(outPorts[j]).section("_", -1, -1) == channel) {
                        jack_connect(m_jackClient, outPorts[j], inPorts[i]);
                    }
                }
            }
        }
    }
    
    m_clients.append(clientName);
    jack_free(outPorts);
    jack_free(inPorts);
}

void Patcher::unregisterClient(const QString &clientName)
{
    QMutexLocker locker(&m_connectionMutex);
    m_clients.removeAll(clientName);
}

Patcher::~Patcher()
{
    jack_client_close(m_jackClient);
}
