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
#include "globals.h"
#include <QTextStream>

/// \todo Check that the RingBuffer Pointer have indeed been initialized before
/// computing anything


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
  // \todo Return an int as en error code

  // \todo Get this name from global variable
  const char* client_name = "PaulTrip";//APP_NAME;
  const char* server_name = NULL;
  jack_options_t options = JackNoStartServer;
  jack_status_t status;

  // Try to connect to the server
  /// \todo Write better warning messages. This following line displays very
  /// verbose message, check how to desable them.
  mClient = jack_client_open (client_name, options, &status, server_name);
  if (mClient == NULL) {
    fprintf (stderr, "jack_client_open() failed, "
    	     "status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
      std::cerr << "ERROR: Maybe the JACK server is not running?" << std::endl;
      std::cerr << SEPARATOR << std::endl;
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
      QTextStream (&inName) << "send_" << i+1;
      mInPorts[i] = jack_port_register (mClient, inName.toLatin1(),
					JACK_DEFAULT_AUDIO_TYPE,
					JackPortIsInput, 0);
    }

  //Create Output Ports
  mOutPorts.resize(mNumOutChans);
  for (int i = 0; i < mNumInChans; i++) 
    {
      QString outName;
      QTextStream (&outName) << "receive_" << i+1;
      mOutPorts[i] = jack_port_register (mClient, outName.toLatin1(),
					JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsOutput, 0);
    }
  
  /// \todo Put this in a better place
  mInBuffer.resize(mNumInChans);
  mOutBuffer.resize(mNumOutChans);
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
/// \todo This function may not be needed anymore as it is.
int JackAudioInterface::setProcessCallback(JackProcessCallback process) const
{
  std::cout << "JACK PROCESS CALLBACK" << std::endl;
  if( int code = (jack_set_process_callback (mClient, process, 0)) )
    {
      std::cerr << "Could not set the process callback" << std::endl;
      return(code);
    }
  return(0);
}


//*******************************************************************************
int JackAudioInterface::startProcess()
{
  std::cout << "Starting JACK PROCESS" << std::endl;
  int code = jack_set_process_callback(mClient,
				       JackAudioInterface::wrapperProcessCallback, this);
  std::cout << "Starting PROCESS CALLED back" << std::endl;

  //Tell the JACK server that we are ready to roll.  Our
  //process() callback will start running now.
  if ( int code = (jack_activate(mClient)) ) 
    {
      std::cerr << "Cannot activate client" << std::endl;
    return(code);
    }
  return(0);
}


//*******************************************************************************
int JackAudioInterface::stopProcess() const
{
  if ( int code = (jack_client_close(mClient)) )
    {
      std::cerr << "Cannot disconnect client" << std::endl;
      return(code);
    }
  return(0);
}


//*******************************************************************************
void JackAudioInterface::jackShutdown (void*)
{
  std::cout << "The Jack Server was shut down!" << std::endl;
  std::cout << "Exiting program..." << std::endl;
  std::exit(1);
}


//*******************************************************************************
void JackAudioInterface::setRingBuffers(std::tr1::shared_ptr<RingBuffer> InRingBuffer,
				       std::tr1::shared_ptr<RingBuffer> OutRingBuffer)
{
  mInRingBuffer = InRingBuffer;
  mOutRingBuffer = OutRingBuffer;
}


//*******************************************************************************
void JackAudioInterface::computeNetworkProcess() 
{
  /// \todo Fix this, I need to read just one packet for all the channels
  /// and then copy that to each channel
  for (int i = 0; i < mNumInChans; i++) {  
    mInRingBuffer->readSlot( (int8_t*) mInBuffer[i] );
  }

  for (int i = 0; i < mNumOutChans; i++) {  
    mOutRingBuffer->writeSlot( (int8_t*) mOutBuffer[i] );
  }
}


//*******************************************************************************
int JackAudioInterface::processCallback(jack_nframes_t nframes)
{
  // Get input and output buffers
  for (int i = 0; i < mNumInChans; i++) {
    mInBuffer[i] = (sample_t*) jack_port_get_buffer(mInPorts[i], nframes);
  }
  for (int i = 0; i < mNumOutChans; i++) {
    mOutBuffer[i] = (sample_t*) jack_port_get_buffer(mOutPorts[i], nframes);
  }

  //-------------------------------------------------------------------
  // TEST: Loopback
  // To test, uncomment and send audio to client input. The same audio
  // should come out as output
  memcpy (mOutBuffer[0], mInBuffer[0], sizeof(sample_t)* nframes);
  //-------------------------------------------------------------------

  /// \todo UNCOMMENT THIS
  //this->computeNetworkProcess();
  /// \todo Dynamically alocate other processes (from FAUST for instance) here
  return 0;
}


//*******************************************************************************
int JackAudioInterface::wrapperProcessCallback(jack_nframes_t nframes, void *arg) 
{
  return static_cast<JackAudioInterface*>(arg)->processCallback(nframes);
}
