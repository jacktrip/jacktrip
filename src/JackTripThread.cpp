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
 * \file JackTripThread.cpp
 * \author Juan-Pablo Caceres
 * \date September 2008
 */


#include "JackTripThread.h"
#include "NetKS.h"

#include <iostream>
#include <cstdlib>

using std::cout; using std::endl;


//*******************************************************************************
void JackTripThread::run()
{
    JackTrip jacktrip(mJackTripMode);
    jacktrip.setAllPorts(mPortNum);

    if ( mJackTripMode == JackTrip::CLIENT )
    {
        jacktrip.setPeerAddress(mPeerAddress);
    }

    NetKS netks;
    jacktrip.appendProcessPluginFromNetwork(&netks);
    //netks.play();


    //QThread::sleep(1);
    //jacktrip.start();
    //netks.play();
    //jacktrip.wait();


    cout << "******** AFTER JACKTRIPTHREAD START **************" << endl;
    //QThread::sleep(9999999);



    /*
  jack_client_t* mClient;
  const char* client_name = "JackThread";
  const char* server_name = NULL;
  jack_options_t options = JackNoStartServer;
  jack_status_t status;

  mClient = jack_client_open (client_name, options, &status, server_name);

  if (mClient == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
             "status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    std::exit(1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(mClient);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }
  */


}
