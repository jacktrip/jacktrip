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
 * \file JackAudioInterface.h
 * \author Juan-Pablo Caceres
 * \date June 2008
 */


#ifndef __JACKAUDIOINTERFACE_H__
#define __JACKAUDIOINTERFACE_H__

#include <iostream>
#include <tr1/memory> //for shared_ptr
#include <functional> //for mem_fun_ref
#include <jack/jack.h>

#include <QVector>
#include <QVarLengthArray>
#include <QMutex>


#include "jacktrip_types.h"
#include "ProcessPlugin.h"

class JackTrip; //forward declaration


/** \brief Class that provides an interface with the Jack Audio Server
 *
 * \todo implement srate_callback
 * \todo automatically starts jack with buffer and sample rate settings specified by the user
 */
class JackAudioInterface
{
public:

  /// \brief Enum for Audio Resolution in bits
  /// \todo implement this into the class, now it's using jack default of 32 bits
  enum audioBitResolutionT {
    BIT8  = 1, ///< 8 bits
    BIT16 = 2, ///< 16 bits (default)
    BIT24 = 3, ///< 24 bits
    BIT32 = 4  ///< 32 bits
  };
  
  /// \brief Sampling Rates supported by JACK
  enum samplingRateT {
    SR22, ///<  22050 Hz
    SR32, ///<  32000 Hz
    SR44, ///<  44100 Hz
    SR48, ///<  48000 Hz
    SR88, ///<  88200 Hz
    SR96, ///<  96000 Hz
    SR192, ///< 192000 Hz
    UNDEF ///< Undefined
  };

  /** \brief The class constructor
   * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
   * \param NumInChans Number of Input Channels
   * \param NumOutChans Number of Output Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   */
  JackAudioInterface(JackTrip* jacktrip,
		     int NumInChans, int NumOutChans,
		     audioBitResolutionT AudioBitResolution = BIT16);

  /** \brief The class destructor
   */
  virtual ~JackAudioInterface();

  /** \brief Get the Jack Server Sampling Rate, in samples/second
   */
  uint32_t getSampleRate() const;

  /** \brief Get the Jack Server Sampling Rate Enum Type samplingRateT
   * \return  JackAudioInterface::samplingRateT enum type
   */
  samplingRateT getSampleRateType() const;

  /** \brief Helper function to get the sample rate (in Hz) for a
   * JackAudioInterface::samplingRateT
   * \param rate_type  JackAudioInterface::samplingRateT enum type
   * \return Sample Rate in Hz
   */
  static int getSampleRateFromType(samplingRateT rate_type);

  /** \brief Get the Jack Server Buffer Size, in samples
   */
  uint32_t getBufferSizeInSamples() const;

  /** \brief Get the Jack Server Buffer Size, in bytes
   */
  uint32_t getBufferSizeInBytes() const 
  { 
    return (getBufferSizeInSamples() * getAudioBitResolution()/8);
  }
  
  /** \brief Get the Audio Bit Resolution, in bits
   *
   * This is one of the audioBitResolutionT set in construction
   */
  int getAudioBitResolution() const;

  /// \brief Get Number of Input Channels
  int getNumInputChannels() const;

  /// \brief Get Number of Output Channels
  int getNumOutputChannels() const;

  /// \brief Get size of each audio per channel, in bytes
  size_t getSizeInBytesPerChannel() const;

  /** \brief Tell the JACK server that we are ready to roll. The
   * process-callback will start running. This runs on its own thread.
   * \return 0 on success, otherwise a non-zero error code
   */
  int startProcess() const;

  /** \brief Stops the process-callback thread
   * \return 0 on success, otherwise a non-zero error code
   */
  int stopProcess() const;

  /** \brief Set the pointer to the Input and Output RingBuffer
   * that'll be use to read and write audio
   *
   * These RingBuffer<EM>s</EM> are used to read and write audio samples on 
   * each JACK callback.
   * \todo If the RingBuffer is blocked, the callback should stay
   * on the last buffer, as in JackTrip (wavetable synth) 
   * \param InRingBuffer RingBuffer to read samples <B>from</B>
   * \param OutRingBuffer RingBuffer to write samples <B>to</B>
   */
  /*
  void setRingBuffers(const std::tr1::shared_ptr<RingBuffer> InRingBuffer,
		      const std::tr1::shared_ptr<RingBuffer> OutRingBuffer);
  */

  /** \brief Append a ProcessPlugin. The order of processing is determined by
   * the order by which appending is done.
   * \param plugin a ProcesPlugin smart pointer. Create the object instance
   * using something like:\n
   * <tt>std::tr1::shared_ptr<ProcessPluginName> loopback(new ProcessPluginName);</tt>
   */
  //void appendProcessPlugin(const std::tr1::shared_ptr<ProcessPlugin> plugin);
  void appendProcessPlugin(ProcessPlugin* plugin);

  /** \brief Convert a 32bit number (sample_t) into one of the bit resolution
   * supported (audioBitResolutionT).
   *
   * The result is stored in an int_8 array of the
   * appropriate size to hold the value. The caller is responsible to allocate 
   * enough space to store the result.
   */
  static void fromSampleToBitConversion(const sample_t* const input,
					int8_t* output,
					const audioBitResolutionT targetBitResolution);

  /** \brief Convert a audioBitResolutionT bit resolution number into a 
   * 32bit number (sample_t)
   *
   * The result is stored in an sample_t array of the
   * appropriate size to hold the value. The caller is responsible to allocate 
   * enough space to store the result.
   */
  static void fromBitToSampleConversion(const int8_t* const input,
					sample_t* output,
					const audioBitResolutionT sourceBitResolution);

  /// \brief Connect the default ports, capture to sends, and receives to playback
  void connectDefaultPorts();

private:

  /** \brief Private method to setup a client of the Jack server.
   *
   * This method is called by the class constructors. It does the following:\n
   *  - Connects to the JACK server
   *  - Sets the shutdown process callback
   *  - Creates the appropriate number of input and output channels
   */
  void setupClient();

  /** \brief Creates input and output channels in the Jack client
   */
  void createChannels();
 
  /** \brief JACK calls this shutdown_callback if the server ever shuts down or
   * decides to disconnect the client.
   */
  static void jackShutdown(void*);
  
  /// \brief Sets the part of the process callback that sends and receive packets
  //void computeNetworkProcess();

  /// \brief Compute the process to receive packets to JACK
  void computeNetworkProcessFromNetwork();

  /// \brief Compute the process from JACK to send packets
  void computeNetworkProcessToNetwork();

  /** \brief Set the process callback of the member function processCallback.
   * This process will be called by the JACK server whenever there is work to be done.
   * \return 0 on success, otherwise a non-zero error code,
   * causing JACK to remove that client from the process() graph.
   */
  int setProcessCallback();

  /** \brief JACK process callback
   * 
   * This is the function to be called to process audio. This function is 
   * of the type JackProcessCallback, which is defined as:\n
   * <tt>typedef int(* JackProcessCallback)(jack_nframes_t nframes, void *arg)</tt>
   * \n
   * See
   * http://jackaudio.org/files/docs/html/types_8h.html#4923142208a8e7dacf00ca7a10681d2b
   * for more details
   */
  int processCallback(jack_nframes_t nframes);
  
  /** \brief Wrapper to cast the member processCallback to a static function pointer
   * that can be used with <tt>jack_set_process_callback</tt>
   *
   * <tt>jack_set_process_callback</tt> needs a static member function pointer. A normal
   * member function won't work because a <b><i>this</i></b> pointer is passed under the scenes.
   * That's why we 
   * need to cast the member funcion processCallback to the static function
   * wrapperProcessCallback. The callback is then set as:\n
   * <tt>jack_set_process_callback(mClient, JackAudioInterface::wrapperProcessCallback,
   *                              this)</tt>
   */
  // reference : http://article.gmane.org/gmane.comp.audio.jackit/12873
  static int wrapperProcessCallback(jack_nframes_t nframes, void *arg) ;


  int mNumInChans;///< Number of Input Channels
  int mNumOutChans; ///<  Number of Output Channels
  int mNumFrames; ///< Buffer block size, in samples
  int mAudioBitResolution; ///< Bit resolution in audio samples
  audioBitResolutionT mBitResolutionMode; ///< Bit resolution (audioBitResolutionT) mode

  jack_client_t* mClient; ///< Jack Client
  QVarLengthArray<jack_port_t*> mInPorts; ///< Vector of Input Ports (Channels)
  QVarLengthArray<jack_port_t*> mOutPorts; ///< Vector of Output Ports (Channels)
  //jack_port_t** mInPorts; ///< Vector of Input Ports (Channels)
  //jack_port_t** mOutPorts; ///< Vector of Output Ports (Channels)
  QVarLengthArray<sample_t*> mInBuffer; ///< Vector of Input buffers/channel read from JACK
  QVarLengthArray<sample_t*> mOutBuffer; ///< Vector of Output buffer/channel to write to JACK

  QVarLengthArray<sample_t*> mInProcessBuffer;///< Vector of Input buffers/channel for ProcessPlugin
  QVarLengthArray<sample_t*> mOutProcessBuffer;///< Vector of Output buffers/channel for ProcessPlugin

  int8_t* mInputPacket; ///< Packet containing all the channels to read from the RingBuffer
  int8_t* mOutputPacket;  ///< Packet containing all the channels to send to the RingBuffer
  size_t mSizeInBytesPerChannel; ///< Size in bytes per audio channel

  /// Vector of Smart Pointer to ProcesPlugin<EM>s</EM>
  //QVector<std::tr1::shared_ptr<ProcessPlugin> > mProcessPlugins;
  QVector<ProcessPlugin*> mProcessPlugins;
  JackTrip* mJackTrip; ///< JackTrip mediator class

  const char** mPorts;
  static QMutex sJackMutex; ///< Mutex to make thread safe jack functions that are not
};


#endif
