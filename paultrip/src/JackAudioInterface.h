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

#include "types.h"
#include "RingBuffer.h"

/** \brief Class that provides an interface with the Jack Audio Server
 *
 * \todo implement srate_callback
 * \todo automatically starts jack with buffer and sample rate settings specified by the user
 * \todo get jack_port_get_buffer for input and output ports (channels), in a way
 * that they can be used in other classes
 * \todo set the Bit Resolution here (now it is defined in PaulTrip.h
 */
class JackAudioInterface
{
public:

  /// \brief Enum for Audio Resolution in bits
  /// \todo implement this into the class, now it's using jack default of 32 bits
  enum audioBitResolutionT {
    BIT8  = 8,  ///< 8 bits
    BIT16 = 16, ///< 16 bits (default)
    BIT24 = 24, ///< 24 bits
    BIT32 = 32  ///< 32 bits
  };
  
  /** \brief The class constructor
   * \param NumInChans Number of Input Channels
   * \param NumOutChans Number of Output Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   */
  JackAudioInterface(int NumInChans, int NumOutChans,
		     audioBitResolutionT AudioBitResolution = BIT16);

  /** \brief Overloaded class constructor with same inputs and output channels
   * \param NumChans Number of Input and Output Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   */
  JackAudioInterface(int NumChans, audioBitResolutionT AudioBitResolution = BIT16);

  /** \brief The class destructor
   */
  virtual ~JackAudioInterface();

  /** \brief Get the Jack Server Sampling Rate, in samples/second
   */
  uint32_t getSampleRate() const;

  /** \brief Get the Jack Server Buffer Size, in samples
   */
  uint32_t getBufferSize() const;

  /** \brief Get the Audio Bit Resolution, in bits
   *
   * This is one of the audioBitResolutionT set in construction
   */
  int getAudioBitResolution() const;

  /** \brief
   * \return 0 on success, otherwise a non-zero error code
   */
  int startProcess() const;

  /** \brief
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
  void setRingBuffers(const std::tr1::shared_ptr<RingBuffer> InRingBuffer,
		      const std::tr1::shared_ptr<RingBuffer> OutRingBuffer);


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
  void computeNetworkProcess();

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

  jack_client_t* mClient; ///< Jack Client
  QVector<jack_port_t*> mInPorts; ///< Vector of Input Ports (Channels)
  QVector<jack_port_t*> mOutPorts; ///< Vector of Output Ports (Channels)

  QVector<sample_t*> mInBuffer; ///< Vector of Input buffers/channel read from JACK
  QVector<sample_t*> mOutBuffer; ///< Vector of Output buffer/channel to write to JACK

  /// Smart Pointer to RingBuffer to read from (input)
  std::tr1::shared_ptr<RingBuffer> mInRingBuffer;
  /// Smart Pointer to RingBuffer to write from (output)
  std::tr1::shared_ptr<RingBuffer> mOutRingBuffer; 
};

#endif
