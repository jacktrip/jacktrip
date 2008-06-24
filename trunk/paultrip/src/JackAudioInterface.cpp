//*****************************************************************
/*
  PaulTrip: A System for High-Quality Audio Network Performance
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
 * \file JackAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#include "JackAudioInterface.h"
#include <QTextStream>

using namespace std;

//*******************************************************************************
JackAudioInterface::JackAudioInterface(int NumInChans, int NumOutChans)
  : mNumInChans(NumInChans), mNumOutChans(NumOutChans)
{
  this->setupClient();
}


//*******************************************************************************
JackAudioInterface::JackAudioInterface(int NumChans)
  : mNumInChans(NumChans), mNumOutChans(NumChans)
{
  this->setupClient();
}


//*******************************************************************************
JackAudioInterface::~JackAudioInterface()
{
  //TODO: Write Destructor
}


//*******************************************************************************
void JackAudioInterface::setupClient()
{
  //TODO: Return an int as en error code

  //TODO: Get this name from global variable
  const char* client_name = "PaulTrip";//APP_NAME;
  const char* server_name = NULL;
  jack_options_t options = JackNoStartServer;
  jack_status_t status;

  //Try to connect to the server
  //TODO: Write better warning messages
  mClient = jack_client_open (client_name, options, &status, server_name);
  if (mClient == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
	     "status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(mClient);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  // Set function to call if Jack shuts down
  jack_on_shutdown (mClient, this->jackShutdown, 0);

  // Create input and output channels
  this->createChannels();
}


//*******************************************************************************
void JackAudioInterface::createChannels()
{
  //Create Input Ports
  mInPorts.resize(mNumInChans);
  for (int i = 0; i < mNumInChans; i++) 
    {
      QString inName;
      QTextStream (&inName) << "input_" << i+1;
      mInPorts[i] = jack_port_register (mClient, inName.toLatin1(),
					JACK_DEFAULT_AUDIO_TYPE,
					JackPortIsInput, 0);
    }

  //Create Output Ports
  mOutPorts.resize(mNumOutChans);
  for (int i = 0; i < mNumInChans; i++) 
    {
      QString outName;
      QTextStream (&outName) << "output_" << i+1;
      mInPorts[i] = jack_port_register (mClient, outName.toLatin1(),
					JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsOutput, 0);
    }
}


//*******************************************************************************
uint32_t JackAudioInterface::getSampleRate() const 
{
  return jack_get_sample_rate(mClient);
}


//*******************************************************************************
uint32_t JackAudioInterface::getBufferSize() const 
{
  return jack_get_buffer_size(mClient);
}


//*******************************************************************************
int JackAudioInterface::setProcessCallback(JackProcessCallback process) const
{
  if( int code = (jack_set_process_callback (mClient, process, 0)) )
    {
      cerr << "Could not set the process callback" << endl;
      return(code);
    }
  return(0);
}


//*******************************************************************************
int JackAudioInterface::startProcess() const
{
  //Tell the JACK server that we are ready to roll.  Our
  //process() callback will start running now.
  if ( int code = (jack_activate(mClient)) ) 
    {
    cerr << "Cannot activate client" << endl;
    return(code);
    }
  return(0);
}


//*******************************************************************************
int JackAudioInterface::stopProcess() const
{
  if ( int code = (jack_client_close(mClient)) )
    {
      cerr << "Cannot disconnect client" << endl;
      return(code);
    }
  return(0);
}


//*******************************************************************************
void JackAudioInterface::jackShutdown (void*)
{
  cout << "The Jack Server was shut down!" << endl;
  cout << "Exiting program..." << endl;
  exit (1);
}
