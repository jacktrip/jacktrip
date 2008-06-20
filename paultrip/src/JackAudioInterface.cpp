/**
 * \file JackAudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date June 2008
 */

#include "JackAudioInterface.h"
#include <QTextStream>

using namespace std;


//------------------------------------------------------------------------------
/** \brief The class constructor
 * 
 */
//-------------------------------------------------------------------------------
JackAudioInterface::JackAudioInterface(int NumInChans, int NumOutChans)
  : mNumInChans(NumInChans), mNumOutChans(NumOutChans)
{
  this->openClientConnection();
}


//------------------------------------------------------------------------------
/** \brief The class destructor
 * 
 */
//-------------------------------------------------------------------------------
JackAudioInterface::~JackAudioInterface()
{
  //TODO: Write Destructor
}


//------------------------------------------------------------------------------
/** \brief Creates a client connection to the JACK server
 * 
 */
//-------------------------------------------------------------------------------
void JackAudioInterface::openClientConnection()
{
  //TODO: Get this name from global variable
  const char* client_name = "paultrip";//APP_NAME;
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

  // Function to call if Jack shuts down
  //jack_on_shutdown (mClient, this->jackShutdown, 0);

  this->createChannels();
}


//------------------------------------------------------------------------------
/** \brief Creates input and output channels in the Jack client
 * 
 */
//-------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------
/** \brief Get the Jack Server Sampling Rate, in samples/second
 * 
 */
//-------------------------------------------------------------------------------
quint32 JackAudioInterface::getSampleRate() const 
{
  return jack_get_sample_rate(mClient);
}


//------------------------------------------------------------------------------
/** \brief Get the Jack Server Buffer Size, in samples
 * 
 */
//-------------------------------------------------------------------------------
quint32 JackAudioInterface::getBufferSize() const 
{
  return jack_get_buffer_size(mClient);
}

//------------------------------------------------------------------------------
/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
//------------------------------------------------------------------------------
/*
void JackAudioInterface::jackShutdown (void* arg)
{
  cout << "The Jack Server was shut down!" << endl;
  cout << "Exiting program..." << endl;
  exit (1);
}
*/
