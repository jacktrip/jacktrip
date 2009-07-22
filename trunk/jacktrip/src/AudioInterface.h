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

#include "ProcessPlugin.h"
#include "jacktrip_types.h"

#include <QVarLengthArray>
#include <QVector>
//#include "jacktrip_globals.h"

// Forward declarations
class JackTrip;


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

  /** \brief Setup the client
    *
    * This will set the audio client with the specified Sampling Rate,
    * Packet Size, Bit Resolution, etc...
    */
  virtual void setup() = 0;
  virtual void close() = 0;
  virtual int startProcess() const = 0;
  virtual int stopProcess() const = 0;
  virtual void appendProcessPlugin(ProcessPlugin* plugin);
  virtual void connectDefaultPorts() = 0;

  virtual void callback(QVarLengthArray<sample_t*>& in_buffer,
                        QVarLengthArray<sample_t*>& out_buffer,
                        int8_t* input_packet,
                        int8_t* output_packet,
                        unsigned int n_frames,
                        QVarLengthArray<sample_t*>& in_process_buffer,
                        QVarLengthArray<sample_t*>& out_process_buffer);

  /** \brief Convert a 32bit number (sample_t) into one of the bit resolution
   * supported (audioBitResolutionT).
   *
   * The result is stored in an int_8 array of the
   * appropriate size to hold the value. The caller is responsible to allocate
   * enough space to store the result.
   */
  static void fromSampleToBitConversion(const sample_t* const input,
                                        int8_t* output,
                                        const AudioInterface::audioBitResolutionT targetBitResolution);

  /** \brief Convert a audioBitResolutionT bit resolution number into a
   * 32bit number (sample_t)
   *
   * The result is stored in an sample_t array of the
   * appropriate size to hold the value. The caller is responsible to allocate
   * enough space to store the result.
   */
  static void fromBitToSampleConversion(const int8_t* const input,
                                        sample_t* output,
                                        const AudioInterface::audioBitResolutionT sourceBitResolution);


  //--------------SETTERS---------------------------------------------
  virtual void setNumInputChannels(int nchannels)
  { mNumInChans = nchannels; }
  virtual void setNumOutputChannels(int nchannels)
  { mNumOutChans = nchannels; }
  virtual void setSampleRate(uint32_t sample_rate)
  { mSampleRate = sample_rate; }
  virtual void setBufferSizeInSamples(uint32_t buf_size)
  { mBufferSizeInSamples = buf_size; }
  /// \brief Set Client Name to something different that the default (JackTrip)
  virtual void setClientName(const char* ClientName) = 0;
  //------------------------------------------------------------------

  //--------------GETTERS---------------------------------------------
  /// \brief Get Number of Input Channels
  virtual int getNumInputChannels() const { return mNumInChans; }
  /// \brief Get Number of Output Channels
  virtual int getNumOutputChannels() const  { return mNumOutChans; }
  virtual uint32_t getBufferSizeInSamples() const
  { return mBufferSizeInSamples; }
  virtual size_t getSizeInBytesPerChannel() const;
  /// \brief Get the Jack Server Sampling Rate, in samples/second
  virtual uint32_t getSampleRate() const
  { return mSampleRate; }
  /// \brief Get the Jack Server Sampling Rate Enum Type samplingRateT
  /// \return  AudioInterface::samplingRateT enum type
  virtual samplingRateT getSampleRateType() const;
  /** \brief Get the Audio Bit Resolution, in bits
   *
   * This is one of the audioBitResolutionT set in construction
   */
  virtual int getAudioBitResolution() const { return mAudioBitResolution; }
  //------------------------------------------------------------------

private:
  void computeProcessFromNetwork(QVarLengthArray<sample_t*>& in_buffer,
                                 QVarLengthArray<sample_t*>& out_buffer,
                                 int8_t* input_packet,
                                 int8_t* output_packet,
                                 unsigned int n_frames);
  void computeProcessToNetwork(QVarLengthArray<sample_t*>& in_buffer,
                               QVarLengthArray<sample_t*>& out_buffer,
                               int8_t* input_packet,
                               int8_t* output_packet,
                               unsigned int n_frames,
                               QVarLengthArray<sample_t*>& in_process_buffer,
                               QVarLengthArray<sample_t*>& out_process_buffer);

  JackTrip* mJackTrip;

  int mNumInChans;///< Number of Input Channels
  int mNumOutChans; ///<  Number of Output Channels
  //int mNumFrames; ///< Buffer block size, in samples
  int mAudioBitResolution; ///< Bit resolution in audio samples
  AudioInterface::audioBitResolutionT mBitResolutionMode; ///< Bit resolution (audioBitResolutionT) mode

  uint32_t mSampleRate; ///< Sampling Rate
  uint32_t mBufferSizeInSamples; ///< Buffer size in samples

  //int8_t* mInputPacket; ///< Packet containing all the channels to read from the RingBuffer
  //int8_t* mOutputPacket;  ///< Packet containing all the channels to send to the RingBuffer
  unsigned int mBufferSize;
  size_t mSizeInBytesPerChannel; ///< Size in bytes per audio channel

  QVector<ProcessPlugin*> mProcessPlugins; ///< Vector of ProcesPlugin<EM>s</EM>
};

#endif // __AUDIOINTERFACE_H__
