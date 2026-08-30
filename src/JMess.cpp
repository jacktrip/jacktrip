/*
  JMess: A simple utility so save your jack-audio mess.
  Incorporated into JackTrip:
  Copyright (c) 2008-2021 Juan-Pablo Caceres, Chris Chafe.
  SoundWIRE group at CCRMA, Stanford University.
  JMess itself is:
  Copyright (C) 2007-2010 Juan-Pablo Caceres.

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

/*
 * JMess.cpp
 */

#include "JMess.h"

#include <QDebug>

#include "JackTrip.h"
#include "jacktrip_globals.h"

// sJackMutex definition
QMutex JMess::sJMessMutex;

//-------------------------------------------------------------------------------
/*! \brief Constructs a JMess object that has a jack client.
 *
 */
//-------------------------------------------------------------------------------
JMess::JMess()
{
    // Open a client connection to the JACK server.  Starting a
    // new server only to list its ports seems pointless, so we
    // specify JackNoStartServer.
    mClient = jack_client_open("lsp", JackNoStartServer, &mStatus);
    if (mClient == NULL) {
        if (mStatus & JackServerFailed) {
            std::cerr << "JACK server not running" << "\n";
        } else {
            std::cerr << "jack_client_open() failed, " << "status = 0x%2.0x\n"
                      << mStatus << "\n";
        }
        exit(1);
    }
}

//-------------------------------------------------------------------------------
/*! \brief Distructor closes the jmess jack audio client.
 *
 */
//-------------------------------------------------------------------------------
JMess::~JMess()
{
    if (jack_client_close(mClient))
        std::cerr << "ERROR: Could not close the hidden jmess jack client." << "\n";
}

//*******************************************************************************
// connectTUB is called when in hubpatch mode 4 = RESERVEDMATRIX
// TU Berlin Raspberry Pi ensemble, Winter 2019
// this gets run on the ensemble's hub server with
// ./jacktrip -S -p3
// it connects a set of client jacktrips with known hardwired IP addresses
// to a known hardwired audio process with known hardwired audio port names
// when clients connect / disconnect dynamically this just runs through the
// audio connection sequence bruteforce at every new connection change
// those that are preexisting won't change
// a new one will connect accordingly and
// those that fail because they don't exist will fail, no worries

// setting the connections tested with jacktrip_globals.h
// const QString gDOMAIN_TRIPLE = QString("130.149.23"); // for TUB multiclient hub
// const int gMIN_TUB = 215; // lowest client address
// const int gMAX_TUB = 215; // highest client address

///////////////////////////////
// test NUC as server
// #define HARDWIRED_AUDIO_PROCESS_ON_SERVER "par20straightWire"
// #define ENUMERATE ""
// #define HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN ":in_"
// #define HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT ":out_"

///////////////////////////////
// test Riviera as server
// for deployment change jacktrip_globals.h to
// const QString gDOMAIN_TRIPLE = QString("192.168.0"); // for TUB multiclient hub
// const int gMIN_TUB = 11; // lowest client address
// const int gMAX_TUB = 20; // highest client address
// and give the proper audio process and connection names

#define HARDWIRED_AUDIO_PROCESS_ON_SERVER     "SuperCollider"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN  ":in_"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT ":out_"
// On server side it is SC jack-clients with indivisual names:
// POE_0...POE_16
// and each has (at this moment) one port in/out:
// receive_1
// send_1
// I think it should be extended to 4 in/out ports per client.

// this is brute force, does not look at individual clients, just patches the whole
// ensemble each time
void JMess::connectTUB(int /*nChans*/)
// called from UdpHubListener::connectPatch
{
    for (int i = 0; i <= gMAX_TUB - gMIN_TUB; i++)  // last IP decimal octet
        for (int l = 1; l <= 1; l++)  // mono for now // chans are 1-based, 1...2
        {
            // jacktrip to SC
            QString client =
                gDOMAIN_TRIPLE + QStringLiteral(".") + QString::number(gMIN_TUB + i);
            QString serverAudio = QStringLiteral(HARDWIRED_AUDIO_PROCESS_ON_SERVER);
            int tmp =
                i + l;  // only works for mono... completely wrong for 2 or more chans
            qDebug() << "connect " << client << ":receive_ " << l << "with "
                     << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN << tmp;

            QString left  = QString(client + ":receive_" + QString::number(l));
            QString right = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN
                                    + QString::number(tmp));

            if (0
                != jack_connect(mClient, left.toStdString().c_str(),
                                right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left << "and port: " << right
                         << " could not be connected.";
            }

            // SC to jacktrip
            tmp += 4;  // increase tmp for port offset
            qDebug() << "connect " << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT
                     << tmp << "with " << client << ":send_" << l;

            left  = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT
                            + QString::number(tmp));
            right = QString(client + ":send_" + QString::number(l));

            if (0
                != jack_connect(mClient, left.toStdString().c_str(),
                                right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left << "and port: " << right
                         << " could not be connected.";
            }
        }
}
