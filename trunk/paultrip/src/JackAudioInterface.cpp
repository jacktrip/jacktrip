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


//------------------------------------------------------------------------------
/** \brief The class constructor
 * 
 * \param NumInChans Number of Input Channels
 * \param NumOutChans Number of Output Channels
 */
//-------------------------------------------------------------------------------
JackAudioInterface::JackAudioInterface(int NumInChans, int NumOutChans)
  : mNumInChans(NumInChans), mNumOutChans(NumOutChans)
{
  this->setupClient();
}


//------------------------------------------------------------------------------
/** \brief Overloaded class constructor with same inputs and output channels
 * 
 * \param NumChans Number of Input and Output Channels
 */
//-------------------------------------------------------------------------------
JackAudioInterface::JackAudioInterface(int NumChans)
  : mNumInChans(NumChans), mNumOutChans(NumChans)
{
  this->setupClient();
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
/** \brief Private method to setup a client of the Jack server. This method
 *  is called by the class constructors. It does the following:\n
 *  - Connects to the JACK server
 *  - Sets the shutdown process callback
 *  - Creates the appropriate number of input and output channels
 */
//-------------------------------------------------------------------------------
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
uint32_t JackAudioInterface::getSampleRate() const 
{
  return jack_get_sample_rate(mClient);
}


//------------------------------------------------------------------------------
/** \brief Get the Jack Server Buffer Size, in samples
 * 
 */
//-------------------------------------------------------------------------------
uint32_t JackAudioInterface::getBufferSize() const 
{
  return jack_get_buffer_size(mClient);
}


//------------------------------------------------------------------------------
/** \brief setProcessCallback passes a function pointer process to be called by
 *  Jack the JACK server whenever there is work to be done.
 * 
 * \param process Function to be called to process audio. This function is 
 * of the type JackProcessCallback, which is defined as:\n
 * <tt>typedef int(* JackProcessCallback)(jack_nframes_t nframes, void *arg)</tt>
 * \n
 * See
 * http://jackaudio.org/files/docs/html/types_8h.html#4923142208a8e7dacf00ca7a10681d2b
 * for more details
 *
 * \return 0 on success, otherwise a non-zero error code,
 * causing JACK to remove that client from the process() graph.
 */
//-------------------------------------------------------------------------------
int JackAudioInterface::setProcessCallback(JackProcessCallback process) const
{
  if( int code = (jack_set_process_callback (mClient, process, 0)) )
    {
      cerr << "Could not set the process callback" << endl;
      return(code);
    }
  return(0);
}


//------------------------------------------------------------------------------
/** \brief
 *
 * \return 0 on success, otherwise a non-zero error code
 */
//-------------------------------------------------------------------------------
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


//------------------------------------------------------------------------------
/** \brief
 *
 * \return 0 on success, otherwise a non-zero error code
 */
//-------------------------------------------------------------------------------
int JackAudioInterface::stopProcess() const
{
  if ( int code = (jack_client_close(mClient)) )
    {
      cerr << "Cannot disconnect client" << endl;
      return(code);
    }
  return(0);
}


//------------------------------------------------------------------------------
/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
//------------------------------------------------------------------------------
void JackAudioInterface::jackShutdown (void*)
{
  cout << "The Jack Server was shut down!" << endl;
  cout << "Exiting program..." << endl;
  exit (1);
}
