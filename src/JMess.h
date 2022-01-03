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
 * JMess.h
 */

#ifndef __JMESS_H
#define __JMESS_H

#include <QIODevice>
#include <QString>
#include <QVector>
#include <cerrno>
#include <iostream>
#include <string>
//#include <QtXml>
//#include <QXmlSimpleReader>
//#include <QXmlInputSource>
//#include <QXmlContentHandler>
#ifdef USE_WEAK_JACK
#include "weak_libjack.h"
#else
#include <jack/jack.h>
#endif

#include <QMutexLocker>

const int Indent = 2;

//-------------------------------------------------------------------------------
/*! \brief Class to save and load all jack client connections.
 *
 * Saves an XML file with all the current jack connections. This same file can
 * be loaded to connect evrything again. The XML file can also be edited.
 *
 * Has also an option to disconnect all the clients.
 */
//-------------------------------------------------------------------------------
class JMess
{
   public:
    JMess();
    virtual ~JMess();

    void disconnectAll();
    void writeOutput(QString xmlOutFile);
    void connectPorts(QString xmlInFile);
    void setConnectedPorts();
    /// \brief Cross connect ports between net combs, -l LAIR mode
    void connectSpawnedPorts(int nChans, int hubPatch);
    void connectTUB(int nChans);

   private:
    int parseXML(QString xmlInFile);

    jack_client_t* mClient;  // Class client
    jack_status_t mStatus;   // Class client status

    // Vectors of Connected Ports and Ports to connects
    // This are a matrix (Nx2) of string like this:
    // OuputPort1 InputPort1
    // ...
    // OuputPortN InputPortN
    QVector<QVector<QString> > mConnectedPorts;
    QVector<QVector<QString> > mPortsToConnect;
    static QMutex sJMessMutex;  ///< Mutex to make thread safe jack functions that are not
};
#endif
