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
            std::cerr << "JACK server not running"
                      << "\n";
        } else {
            std::cerr << "jack_client_open() failed, "
                      << "status = 0x%2.0x\n"
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
        std::cerr << "ERROR: Could not close the hidden jmess jack client."
                  << "\n";
}

//-------------------------------------------------------------------------------
/*! \brief Write an XML file with the name specified at xmlOutFile.
 *
 */
//-------------------------------------------------------------------------------
void JMess::writeOutput(QString /*xmlOutFile*/)
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
    //    //cout << "Output ===> " <<qPrintable(OutputInput[0]) << "\n";
    //    //cout << "Input ===> " <<qPrintable(OutputInput[1]) << "\n";

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
    //      std::cerr << "Cannot open file for writing: "
    //	   << qPrintable(file.errorString()) << "\n";
    //      exit(1);
    //    }

    //    QTextStream out(&file);
    //    jmess_xml.save(out, Indent);
    //    cout << qPrintable(xmlOutFile) << " written." << "\n";
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

    const char **ports, **connections;  // vector of ports and connections
    QVector<QString> OutputInput(2);    // helper variable

    // Get active output ports.
    ports = jack_get_ports(mClient, NULL, NULL, JackPortIsOutput);

    for (unsigned int out_i = 0; ports[out_i]; ++out_i) {
        if ((connections = jack_port_get_all_connections(
                 mClient, jack_port_by_name(mClient, ports[out_i])))
            != 0) {
            for (unsigned int in_i = 0; connections[in_i]; ++in_i) {
                OutputInput[0] = ports[out_i];
                //    cout << "Output ===> " <<qPrintable(OutputInput[0]) << "\n";
                OutputInput[1] = connections[in_i];
                //    cout << "Input ===> " << qPrintable(OutputInput[1]) << "\n";
                mConnectedPorts.append(OutputInput);
            }
        }
    }

    free(ports);
}
//*******************************************************************************
void JMess::connectSpawnedPorts(int nChans, int hubPatch)
// called from UdpHubListener::connectMesh
{
    QMutexLocker locker(&sJMessMutex);

    QString IPS[gMAX_WAIRS];
    int ctr = 0;

    const char** ports;  //, **connections; //vector of ports and connections

    // Get active output ports.
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
    // for (int i = 0; i<ctr; i++) qDebug() << IPS[i];
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
                // qDebug() << "connect " << IPS[i]+":receive_"+QString::number(l)
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
                    // qDebug() << "connect " << IPS[i]+":receive_"+QString::number(l)
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

//-------------------------------------------------------------------------------
/*! \brief Disconnect all the clients.
 *
 */
//-------------------------------------------------------------------------------
void JMess::disconnectAll()
{
    this->setConnectedPorts();

    for (auto& OutputInput : mConnectedPorts) {
        if (jack_disconnect(mClient, OutputInput[0].toUtf8(), OutputInput[1].toUtf8())) {
            std::cerr << "WARNING: port: " << qPrintable(OutputInput[0])
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
int JMess::parseXML(QString /*xmlInFile*/)
{
    //  mPortsToConnect.clear();
    //  QString errorStr;
    //  int errorLine;
    //  int errorColumn;

    //  QFile file(xmlInFile);
    //  if (!file.open(QIODevice::ReadOnly)) {
    //    std::cerr << "Cannot open file for reading: "
    //	 << qPrintable(file.errorString()) << "\n";
    //    return 1;
    //  }

    //  QDomDocument doc;
    //  if (!doc.setContent(&file, true, &errorStr, &errorLine,
    //		      &errorColumn)) {
    //    std::cerr << "===================================================\n"
    //	 << "Error parsing XML input file:\n"
    //	 << "Parse error at line " << errorLine
    //	 << ", column " << errorColumn << "\n"
    //	 << qPrintable(errorStr) << "\n"
    //	 << "===================================================\n";
    //    return 1;
    //  }

    //  QDomElement jmess = doc.documentElement();
    //  if (jmess.tagName() != "jmess") {
    //    std::cerr << "Error: Root tag should be <jmess>: "
    //	 << qPrintable(jmess.tagName()) << "\n";
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
    //	//cout << qPrintable(sck.tagName()) << "\n";
    //	//cout << qPrintable(sck.text()) << "\n";
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
void JMess::connectPorts(QString /*xmlInFile*/)
{
    //    QVector<QString> OutputInput(2);

    //  if ( !(this->parseXML(xmlInFile)) ) {
    //    for (QVector<QVector<QString> >::iterator it = mPortsToConnect.begin();
    //	 it != mPortsToConnect.end(); ++it) {
    //      OutputInput = *it;

    //      if (jack_connect(mClient, OutputInput[0].toLatin1(),
    //      OutputInput[1].toLatin1())) {
    //	//Display a warining only if the error is not because the ports are already
    //	//connected, in case the program doesn't display anyting.
    //	if (EEXIST !=
    //        jack_connect(mClient, OutputInput[0].toLatin1(), OutputInput[1].toLatin1()))
    //        {
    //	  std::cerr << "WARNING: port: " << qPrintable(OutputInput[0])
    //	       << "and port: " << qPrintable(OutputInput[1])
    //	       << " could not be connected.\n";
    //	}
    //      }
    //    }
    //  }
}
