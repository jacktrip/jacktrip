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
 * \file AudioInterface.h
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#ifndef __AUDIOINTERFACE_H__
#define __AUDIOINTERFACE_H__

#include "jacktrip_types.h"

#include <QVarLengthArray>
//#include "jacktrip_globals.h"

class JackTrip; //forward declaration

class AudioInterface
{
public:

  /// \brief Enum for Audio Resolution in bits
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
  AudioInterface(JackTrip* jacktrip,
                 int NumInChans, int NumOutChans,
                 AudioInterface::audioBitResolutionT AudioBitResolution =
                 AudioInterface::BIT16);

  virtual ~AudioInterface();

  /// \brief Setup the client
  virtual void setup();

  int processCallback(float* output_buffer,
                      float* input_buffer,
                      unsigned int num_buffer_frames,
                      unsigned int num_channels);

  void computeProcessFromNetwork(float* output_buffer,
                                 float* input_buffer,
                                 unsigned int num_buffer_frames,
                                 unsigned int num_channels);

  void computeNetworkProcessToNetwork(float* output_buffer,
                                      float* input_buffer,
                                      unsigned int num_buffer_frames,
                                      unsigned int num_channels);

  //--------------SETTERS---------------------------------------------
  virtual void setNumInputChannels(int nchannels)
  { mNumInChans = nchannels; }
  virtual void setNumOutputChannels(int nchannels)
  { mNumOutChans = nchannels; }
  virtual int setNumBufferFramesPerChannel(int nbuf_frames)
  { mNumBufferFramesPerChannel = nbuf_frames; }
  //------------------------------------------------------------------

  //--------------GETTERS---------------------------------------------
  /// \brief Get Number of Input Channels
  virtual int getNumInputChannels() const { return mNumInChans; }
  /// \brief Get Number of Output Channels
  virtual int getNumOutputChannels() const  { return mNumOutChans; }
  virtual int getNumBufferFramesPerChannel() const
  { return mNumBufferFramesPerChannel; }
  virtual uint32_t getBufferSizeInSamples() const;
  virtual size_t getSizeInBytesPerChannel() const;
  //------------------------------------------------------------------

private:

  int mNumInChans;///< Number of Input Channels
  int mNumOutChans; ///<  Number of Output Channels
  int mNumBufferFramesPerChannel; ///< Buffer block size, in samples

  int8_t* mInputPacket; ///< Packet containing all the channels to read from the RingBuffer
  int8_t* mOutputPacket;  ///< Packet containing all the channels to send to the RingBuffer
  unsigned int mBufferSize;
};

#endif // __AUDIOINTERFACE_H__
