/*
  JMess: A simple utility so save your jack-audio mess.

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
#include <QDebug>
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER "SuperCollider"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN ":in_"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT ":out_"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO "panpot9toStereo"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO "freeverbStereo"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_ppBast "ppBast"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_fvBcast "fvBcast"
#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND "ecasound"

// sJackMutex definition
QMutex JMess::sJMessMutex;

//-------------------------------------------------------------------------------
/*! \brief Constructs a JMess object that has a jack client.
 *
 */
//-------------------------------------------------------------------------------
JMess::JMess()
{
    //Open a client connection to the JACK server.  Starting a
    //new server only to list its ports seems pointless, so we
    //specify JackNoStartServer.
    mClient = jack_client_open("lsp", JackNoStartServer, &mStatus);
    if (mClient == NULL) {
        if (mStatus & JackServerFailed) {
            cerr << "JACK server not running" << endl;
        } else {
            cerr << "jack_client_open() failed, "
                 << "status = 0x%2.0x\n"
                 << mStatus << endl;
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
        cerr << "ERROR: Could not close the hidden jmess jack client." << endl;
}

//-------------------------------------------------------------------------------
/*! \brief Write an XML file with the name specified at xmlOutFile.
 *
 */
//-------------------------------------------------------------------------------
void JMess::writeOutput(__attribute__((unused)) QString xmlOutFile)
{
    //  QDomDocument jmess_xml;   QDomElement root;
    //  QDomElement connection;   QDomElement output;
    //  QDomElement input;        QDomText output_name;
    //  QDomText input_name;

    //  QVector<QString> OutputInput(2);

    //  this->setConnectedPorts();

    //  root = jmess_xml.createElement("jmess");
    //  for (QVector<QVector<QString> >::iterator it = mConnectedPorts.begin();
    //       it != mConnectedPorts.end(); ++it) {
    //    OutputInput = *it;
    //    //cout << "Output ===> " <<qPrintable(OutputInput[0]) << endl;
    //    //cout << "Input ===> " <<qPrintable(OutputInput[1]) << endl;

    //    //Initialize XML elements
    //    connection = jmess_xml.createElement("connection");
    //    output = jmess_xml.createElement("output");
    //    input = jmess_xml.createElement("input");
    //    output_name = jmess_xml.createTextNode(OutputInput[0]);
    //    input_name = jmess_xml.createTextNode(OutputInput[1]);

    //    jmess_xml.appendChild(root);      root.appendChild(connection);
    //    connection.appendChild(output);   connection.appendChild(input);
    //    output.appendChild(output_name);  input.appendChild(input_name);
    //  }

    //  //Write output file
    //  QFile file(xmlOutFile);
    //  string answer = "";
    //  //Check for existing file first, and confirm before overwriting
    //  if (file.exists()) {
    //    while ((answer != "yes") && (answer != "no")) {
    //      cout << "WARNING: The File " <<qPrintable(xmlOutFile)
    //	   << " exists. Do you want to overwrite it? (yes/no): ";
    //      cin >> answer;
    //    }
    //  }
    //  else {
    //    answer = "yes";
    //  }

    //  if (answer == "yes") {
    //    if (!file.open(QIODevice::WriteOnly)) {
    //      cerr << "Cannot open file for writing: "
    //	   << qPrintable(file.errorString()) << endl;
    //      exit(1);
    //    }

    //    QTextStream out(&file);
    //    jmess_xml.save(out, Indent);
    //    cout << qPrintable(xmlOutFile) << " written." << endl;
    //  }
}

//-------------------------------------------------------------------------------
/*! \brief Set list of ouput ports that have connections.
 *
 */
//-------------------------------------------------------------------------------
void JMess::setConnectedPorts()
{
    mConnectedPorts.clear();

    const char **ports, **connections;  //vector of ports and connections
    QVector<QString> OutputInput(2);    //helper variable

    //Get active output ports.
    ports = jack_get_ports(mClient, NULL, NULL, JackPortIsOutput);

    for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
        if ((connections = jack_port_get_all_connections(
                 mClient, jack_port_by_name(mClient, ports[out_i])))
            != 0) {
            for (unsigned int in_i = 0; connections[in_i]; ++in_i) {
                OutputInput[0] = ports[out_i];
                //    cout << "Output ===> " <<qPrintable(OutputInput[0]) << endl;
                OutputInput[1] = connections[in_i];
                //    cout << "Input ===> " << qPrintable(OutputInput[1]) << endl;
                mConnectedPorts.append(OutputInput);
            }
        }
    }

    free(ports);
}
//*******************************************************************************
void JMess::connectSpawnedPorts(int nChans, int hubPatch)
// called from UdpHubListener::connectMesh
// this gets run on the ensemble's hub server with
// CLIENTECHO
// ./jacktrip -S -p1
// CLIENTFOFI
// ./jacktrip -S -p2
// FULLMIX
// ./jacktrip -S -p4

// FULLMIX might include ecasound for example in server with looping clap track
{
    QMutexLocker locker(&sJMessMutex);

    QString IPS[gMAX_WAIRS];
    int ctr = 0;

    const char** ports;               //, **connections; //vector of ports and connections
    QVector<QString> OutputInput(2);  //helper variable

    //Get active output ports.
    ports = jack_get_ports(mClient, NULL, NULL, JackPortIsOutput);

    for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
        //        qDebug() << QString(ports[out_i]);
        bool systemPort = QString(ports[out_i]).contains(QString("system"));

        QString str = QString(ports[out_i]);
        //  for example              "171.64.197.121:receive_1"
        QString s = str.section(':', 0, 0);
        //        qDebug() << s << systemPort;
        //  for example              "171.64.197.121"

        bool newOne = !systemPort;
        for (int i = 0; i < ctr; i++)
            if (newOne && (IPS[i] == s)) newOne = false;
        if (newOne) {
            IPS[ctr] = s;
            ctr++;
            //                        qDebug() << ports[out_i] << systemPort << s;
        }
    }
    //for (int i = 0; i<ctr; i++) qDebug() << IPS[i];
    disconnectAll();

    int k      = 0;
    int jLimit = 1;

    // FULLMIX is the union of CLIENTFOFI, CLIENTECHO

    // implements CLIENTFOFI, CLIENTECHO -- also FULLMIX part which is CLIENTECHO
    for (int i = 0; i < ctr; i++) {
        if (hubPatch == JackTrip::CLIENTFOFI) jLimit = (ctr - 1);
        for (int j = 0; j < jLimit; j++) {
            if ((hubPatch == JackTrip::CLIENTECHO) || (hubPatch == JackTrip::FULLMIX))
                k = i;
            else if (hubPatch == JackTrip::CLIENTFOFI)
                k = (j + (i + 1)) % ctr;
            for (int l = 1; l <= nChans; l++) {  // chans are 1-based
                //qDebug() << "connect " << IPS[i]+":receive_"+QString::number(l)
                //<<"with " << IPS[k]+":send_"+QString::number(l);

                QString left  = IPS[i] + ":receive_" + QString::number(l);
                QString right = IPS[k] + ":send_" + QString::number(l);

                if (0
                    != jack_connect(mClient, left.toStdString().c_str(),
                                    right.toStdString().c_str())) {
                    qDebug() << "WARNING: port: " << left << "and port: " << right
                             << " could not be connected.";
                }
            }
        }
    }

    // do it again to implement the FULLMIX part which is CLIENTFOFI
    if (hubPatch == JackTrip::FULLMIX) {
        jLimit = (ctr - 1);  // same as CLIENTFOFI
        /*************/
        // todo: the next block should be in a method, it's a repeat of the above
        for (int i = 0; i < ctr; i++) {
            for (int j = 0; j < jLimit; j++) {
                k = (j + (i + 1)) % ctr;
                for (int l = 1; l <= nChans; l++) {  // chans are 1-based
                    //qDebug() << "connect " << IPS[i]+":receive_"+QString::number(l)
                    //<<"with " << IPS[k]+":send_"+QString::number(l);

                    QString left = IPS[i] +
                            ":receive_" + QString::number(l);
                    QString right = IPS[k] +
                            ":send_" + QString::number(l);
                    if (IPS[i] == HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND)
                        left = IPS[i] +
                                ":out_" + QString::number(l);
                    if (0
                        != jack_connect(mClient, left.toStdString().c_str(),
                                        right.toStdString().c_str())) {
                        qDebug() << "WARNING: port: " << left << "and port: " << right
                                 << " could not be connected.";
                    }
                }
            }
        }
    }

    free(ports);
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
//#define HARDWIRED_AUDIO_PROCESS_ON_SERVER "par20straightWire"
//#define ENUMERATE ""
//#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN ":in_"
//#define HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT ":out_"

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

// this is brute force, does not look at individual clients, just patches the whole ensemble
// each time
void JMess::connectTUB(int /*nChans*/)
// called from UdpHubListener::connectPatch
{
    for (int i = 0; i <= gMAX_TUB - gMIN_TUB; i++)  // last IP decimal octet
        for (int l = 1; l <= 1; l++)  // mono for now // chans are 1-based, 1...2
        {
            // jacktrip to SC
            QString client =
                gDOMAIN_TRIPLE + QString(".") + QString::number(gMIN_TUB + i);
            QString serverAudio = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER);
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
            tmp += 4;  // increase tmp for port offest
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

//*******************************************************************************
// connectPAN is called when in hubpatch mode = PANSTEREO
// panning and reverb effectively acting like FULLMIX above
// used by Stanford ensembles, Fall 2020
// this gets run on the ensemble's hub server with
// ./jacktrip -S -p6
// 6=stereo room w/pan9+reverb and connects a variable set of client jacktrips
// to a known hardwired audio process with known hardwired audio port names
// when clients connect / disconnect dynamically this just runs through the
// audio connection sequence bruteforce at every new connection change
// panning input has 9 slots in the stereo field
#define NPANINCHANS 9
// distribution across the slots is a function of how many clients
// stereo field pipes through a stereo (input) version of freeverb

// two faust-generated audio processes need to be running, .dsp code in
// panpot9toStereo
// freeverbStereo
// (optional) ecasound playback feeds to stereo left if it's running
// generate the faust applications with faust2jackconsole
// faust2jackconsole panpot9toStereo.dsp
// faust2jackconsole freeverbStereo.dsp

void JMess::connectPAN(int /*nChans*/)
// called from UdpHubListener::connectPatch
{
    int hubPatch = JackTrip::PANSTEREO;
    { // variant of FOFI
        QMutexLocker locker(&sJMessMutex);

        QString IPS[gMAX_WAIRS];
        int ctr = 0;

        const char **ports, **connections; //vector of ports and connections
        QVector<QString> OutputInput(2); //helper variable

        //Get active output ports.
        ports = jack_get_ports (mClient, NULL, NULL, JackPortIsOutput);

        for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
            //        qDebug() << QString(ports[out_i]);
            bool systemPort =
                    QString(ports[out_i]).contains(QString("system")) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND))                    ;

            QString str = QString(ports[out_i]);
            //  for example              "171.64.197.121:receive_1"
            QString s = str.section(':', 0, 0);
            //        qDebug() << s << systemPort;
            //  for example              "171.64.197.121"

            bool newOne = !systemPort;
            for (int i = 0; i<ctr; i++) if (newOne && (IPS[i]==s)) newOne = false;
            if (newOne)
            {
                IPS[ctr] = s;
                ctr++;
                //                        qDebug() << ports[out_i] << systemPort << s;
            }
        }
        //    for (int i = 0; i<ctr; i++) qDebug() << IPS[i];
        disconnectAll();
        //ctr = 1;
        int zones = NPANINCHANS;
        int pctr = ctr;
        if (pctr>NPANINCHANS) pctr = NPANINCHANS;
        if (pctr) zones /= pctr;
        int halfZone = zones / 2;
        if (!halfZone) halfZone++;
        int quarterZone = halfZone / 2;
        if (!quarterZone) quarterZone++;
        for (int i = 0; i<ctr; i++) {
            // network in to panner
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
//                int slot = (halfZone + ((i%NPANINCHANS)*zones) );
//                int slot = (halfZone + ( ( (i+((ch-1)*halfZone)) %NPANINCHANS)*zones) );
                int slot = (halfZone + ((ch-1)*quarterZone) + ( ( i %NPANINCHANS)*zones) );

                //  needed % otherwise clients > NPANINCHANS results in all connected to slot 1
                qDebug() << "i " << i << "ctr " << ctr  << "pctr " << pctr  << "ch " << ch
                         << "slot " << slot << "slot%NPANINCHANS " << ( slot % NPANINCHANS )
                         << "halfZone " << halfZone << "quarterZone " << quarterZone;
                QString left = IPS[i] +
                        ":receive_" + QString::number(ch);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( slot % NPANINCHANS ) );
                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // panner to reverb
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch-1);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( (ch-1) % NPANINCHANS ) );

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // reverb to network out
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch-1);

                QString right = IPS[i] +
                        ":send_" + QString::number(ch);

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // ecasound to panner left
            for (int ch = 1; ch<=1; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( 0 % NPANINCHANS ) );

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

        }

        free(ports);
    }

    for (int i = 0; i<0; i++) // last IP decimal octet
        for (int l = 1; l<=1; l++) // mono for now // chans are 1-based, 1...2
        {
            // jacktrip to SC
            QString client = gDOMAIN_TRIPLE + QString(".") + QString::number(gMIN_TUB+i);
            QString serverAudio = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO);
            int tmp = i + l; // only works for mono... completely wrong for 2 or more chans
            qDebug() << "connect " << client << ":receive_ " << l
                     <<"with " << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN << tmp;

            QString left = QString(client + ":receive_" + QString::number(l));
            QString right = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN +
                                    QString::number(tmp));

            if (0 !=
                    jack_connect(mClient, left.toStdString().c_str(),
                                 right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left
                         << "and port: " << right
                         << " could not be connected.";
            }

            // SC to jacktrip
            tmp += 4; // increase tmp for port offest
            qDebug() << "connect " << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT
                     << tmp <<"with " << client << ":send_" << l;

            left = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT +
                           QString::number(tmp));
            right = QString(client + ":send_" + QString::number(l));

            if (0 !=
                    jack_connect(mClient, left.toStdString().c_str(),
                                 right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left
                         << "and port: " << right
                         << " could not be connected.";
            }

        }
} // end connectPAN


//*******************************************************************************
// connectPANbroadcast is called when in hubpatch mode = PANBCAST
// same as -p6 above but adds client outputs tapped from "broadcast_1" and "broadcast_2"
// to ppBcast panner and send from fvBcast to one special client presumably running OBS
// used by Stanford ensembles, Winter 2021
// this gets run on the ensemble's hub server with
// ./jacktrip -S -p7
// 7=stereo room w/pan9+reverb which uses broadcast outputs
// client outputs are tapped from "broadcast_1" and "broadcast_2"
#define NPANINCHANS 9

// four faust-generated audio processes need to be running, .dsp code in
// panpot9toStereo + ppBcast (which are the same)
// freeverbStereo + fvBcast (which are the same)
// (optional) ecasound playback feeds to stereo left if it's running
// generate the faust applications with faust2jackconsole
// faust2jackconsole panpot9toStereo.dsp
// faust2jackconsole freeverbStereo.dsp
// cp -p panpot9toStereo ppBcast
// cp -p freeverbStereo fvBcast

void JMess::connectPANbroadcast(int /*nChans*/)
// called from UdpHubListener::connectPatch
{
    int hubPatch = JackTrip::PANBCAST;
    { // variant of FOFI
        QMutexLocker locker(&sJMessMutex);

        QString IPS[gMAX_WAIRS];
        int ctr = 0;

        const char **ports, **connections; //vector of ports and connections
        QVector<QString> OutputInput(2); //helper variable

        //Get active output ports.
        ports = jack_get_ports (mClient, NULL, NULL, JackPortIsOutput);

        for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
            //        qDebug() << QString(ports[out_i]);
            bool systemPort =
                    QString(ports[out_i]).contains(QString("system")) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_ppBast)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_fvBcast)) ||
                    QString(ports[out_i]).contains(QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND))                    ;

            QString str = QString(ports[out_i]);
            //  for example              "171.64.197.121:receive_1"
            QString s = str.section(':', 0, 0);
            //        qDebug() << s << systemPort;
            //  for example              "171.64.197.121"

            bool newOne = !systemPort;
            for (int i = 0; i<ctr; i++) if (newOne && (IPS[i]==s)) newOne = false;
            if (newOne)
            {
                IPS[ctr] = s;
                ctr++;
                //                        qDebug() << ports[out_i] << systemPort << s;
            }
        }
        //    for (int i = 0; i<ctr; i++) qDebug() << IPS[i];
        disconnectAll();
        //ctr = 1;
        int zones = NPANINCHANS;
        int pctr = ctr;
        if (pctr>NPANINCHANS) pctr = NPANINCHANS;
        if (pctr) zones /= pctr;
        int halfZone = zones / 2;
        if (!halfZone) halfZone++;
        int quarterZone = halfZone / 2;
        if (!quarterZone) quarterZone++;
        for (int i = 0; i<ctr; i++) {
            // network in to panner
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
//                int slot = (halfZone + ((i%NPANINCHANS)*zones) );
//                int slot = (halfZone + ( ( (i+((ch-1)*halfZone)) %NPANINCHANS)*zones) );
                int slot = (halfZone + ((ch-1)*quarterZone) + ( ( i %NPANINCHANS)*zones) );

                //  needed % otherwise clients > NPANINCHANS results in all connected to slot 1
                qDebug() << "i " << i << "ctr " << ctr  << "pctr " << pctr  << "ch " << ch
                         << "slot " << slot << "slot%NPANINCHANS " << ( slot % NPANINCHANS )
                         << "halfZone " << halfZone << "quarterZone " << quarterZone;
                QString left = IPS[i] +
                        ":receive_" + QString::number(ch);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( slot % NPANINCHANS ) );
                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // panner to reverb
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch-1);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( (ch-1) % NPANINCHANS ) );

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // reverb to network out
            for (int ch = 1; ch<=2; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_FREEVERBSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch-1);

                QString right = IPS[i] +
                        ":send_" + QString::number(ch);

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

            // ecasound to panner left
            for (int ch = 1; ch<=1; ch++) { // chans are 1-based
                QString left = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_ECASOUND) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT + QString::number(ch);

                QString right = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO) +
                        HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN + QString::number(
                            ( 0 % NPANINCHANS ) );

                qDebug() << "connect " << left <<"with " << right;
                if (0 !=
                        jack_connect(mClient, left.toStdString().c_str(), right.toStdString().c_str())) {
                    qDebug() << "WARNING FROM JACK: port: " << left
                             << "and port: " << right
                             << " could not be connected.";
                }
            }

        }

        free(ports);
    }

    for (int i = 0; i<0; i++) // last IP decimal octet
        for (int l = 1; l<=1; l++) // mono for now // chans are 1-based, 1...2
        {
            // jacktrip to SC
            QString client = gDOMAIN_TRIPLE + QString(".") + QString::number(gMIN_TUB+i);
            QString serverAudio = QString(HARDWIRED_AUDIO_PROCESS_ON_SERVER_PANSTEREO);
            int tmp = i + l; // only works for mono... completely wrong for 2 or more chans
            qDebug() << "connect " << client << ":receive_ " << l
                     <<"with " << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN << tmp;

            QString left = QString(client + ":receive_" + QString::number(l));
            QString right = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_IN +
                                    QString::number(tmp));

            if (0 !=
                    jack_connect(mClient, left.toStdString().c_str(),
                                 right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left
                         << "and port: " << right
                         << " could not be connected.";
            }

            // SC to jacktrip
            tmp += 4; // increase tmp for port offest
            qDebug() << "connect " << serverAudio << HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT
                     << tmp <<"with " << client << ":send_" << l;

            left = QString(serverAudio + HARDWIRED_AUDIO_PROCESS_ON_SERVER_OUT +
                           QString::number(tmp));
            right = QString(client + ":send_" + QString::number(l));

            if (0 !=
                    jack_connect(mClient, left.toStdString().c_str(),
                                 right.toStdString().c_str())) {
                qDebug() << "WARNING: port: " << left
                         << "and port: " << right
                         << " could not be connected.";
            }

        }
} // end connectPANbroadcast

//-------------------------------------------------------------------------------
/*! \brief Disconnect all the clients.
 *
 */
//-------------------------------------------------------------------------------
void JMess::disconnectAll()
{
    QVector<QString> OutputInput(2);

    this->setConnectedPorts();

    for (QVector<QVector<QString> >::iterator it = mConnectedPorts.begin();
         it != mConnectedPorts.end(); ++it) {
        OutputInput = *it;

        if (jack_disconnect(mClient, OutputInput[0].toUtf8(), OutputInput[1].toUtf8())) {
            cerr << "WARNING: port: " << qPrintable(OutputInput[0])
                 << "and port: " << qPrintable(OutputInput[1])
                 << " could not be disconnected.\n";
        }
    }
}

//-------------------------------------------------------------------------------
/*! \brief Parse the XML input file.
 *
 * Returns 0 on success, or 1 if the file has an incorrect format or cannot
 * read the file.
 */
//-------------------------------------------------------------------------------
int JMess::parseXML(__attribute__((unused)) QString xmlInFile)
{
    //  mPortsToConnect.clear();
    //  QString errorStr;
    //  int errorLine;
    //  int errorColumn;

    //  QFile file(xmlInFile);
    //  if (!file.open(QIODevice::ReadOnly)) {
    //    cerr << "Cannot open file for reading: "
    //	 << qPrintable(file.errorString()) << endl;
    //    return 1;
    //  }

    //  QDomDocument doc;
    //  if (!doc.setContent(&file, true, &errorStr, &errorLine,
    //		      &errorColumn)) {
    //    cerr << "===================================================\n"
    //	 << "Error parsing XML input file:\n"
    //	 << "Parse error at line " << errorLine
    //	 << ", column " << errorColumn << "\n"
    //	 << qPrintable(errorStr) << "\n"
    //	 << "===================================================\n";
    //    return 1;
    //  }

    //  QDomElement jmess = doc.documentElement();
    //  if (jmess.tagName() != "jmess") {
    //    cerr << "Error: Root tag should be <jmess>: "
    //	 << qPrintable(jmess.tagName()) << endl;
    //    return 1;
    //  }

    //  QVector<QString> OutputInput(2);
    //  //First check for <connection> tag
    //  for(QDomNode n_cntn = jmess.firstChild();
    //      !n_cntn.isNull(); n_cntn = n_cntn.nextSibling()) {
    //    QDomElement cntn = n_cntn.toElement();
    //    if (cntn.tagName() == "connection") {
    //      //Now check for ouput & input tag
    //      for(QDomNode n_sck = cntn.firstChild();
    //	  !n_sck.isNull(); n_sck = n_sck.nextSibling()) {
    //	QDomElement sck = n_sck.toElement();
    //	//cout << qPrintable(sck.tagName()) << endl;
    //	//cout << qPrintable(sck.text()) << endl;
    //	if (sck.tagName() == "output") {
    //	  OutputInput[0] = sck.text();
    //	}
    //	else if (sck.tagName() == "input") {
    //	  OutputInput[1] = sck.text();
    //	}
    //      }
    //      mPortsToConnect.append(OutputInput);
    //    }
    //  }

    return 0;
}

//-------------------------------------------------------------------------------
/*! \brief Connect ports specified in input XML file xmlInFile
 *
 */
//-------------------------------------------------------------------------------
void JMess::connectPorts(__attribute__((unused)) QString xmlInFile)
{
    QVector<QString> OutputInput(2);

    //  if ( !(this->parseXML(xmlInFile)) ) {
    //    for (QVector<QVector<QString> >::iterator it = mPortsToConnect.begin();
    //	 it != mPortsToConnect.end(); ++it) {
    //      OutputInput = *it;

    //      if (jack_connect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1())) {
    //	//Display a warining only if the error is not because the ports are already
    //	//connected, in case the program doesn't display anyting.
    //	if (EEXIST !=
    //        jack_connect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1())) {
    //	  cerr << "WARNING: port: " << qPrintable(OutputInput[0])
    //	       << "and port: " << qPrintable(OutputInput[1])
    //	       << " could not be connected.\n";
    //	}
    //      }
    //    }
    //  }
}
