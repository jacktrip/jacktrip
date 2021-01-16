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
//#include <tr1/memory> //for shared_ptr
#include <functional> //for mem_fun_ref
#include <jack/jack.h>

#include <QVector>
#include <QVarLengthArray>
#include <QMutex>


#include "jacktrip_types.h"
#include "ProcessPlugin.h"
#include "AudioInterface.h"

//class JackTrip; //forward declaration


/** \brief Class that provides an interface with the Jack Audio Server
 *
 * \todo implement srate_callback
 * \todo automatically starts jack with buffer and sample rate settings specified by the user
 */
class JackAudioInterface : public AudioInterface
{
public:

    /** \brief The class constructor
   * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
   * \param NumInChans Number of Input Channels
   * \param NumOutChans Number of Output Channels
   * \param AudioBitResolution Audio Sample Resolutions in bits
   * \param ClientName Client name in Jack
   */
    JackAudioInterface(JackTrip* jacktrip,
                       int NumInChans, int NumOutChans,
                   #ifdef WAIR // wair
                       int NumNetRevChans,
                   #endif // endwhere
                       AudioInterface::audioBitResolutionT AudioBitResolution = AudioInterface::BIT16,
                       QString ClientName = "JackTrip");
    /// \brief The class destructor
    virtual ~JackAudioInterface();

    /// \brief Setup the client
    virtual void setup();
    /** \brief Tell the JACK server that we are ready to roll. The
   * process-callback will start running. This runs on its own thread.
   * \return 0 on success, otherwise a non-zero error code
   */
    virtual int startProcess() const;
    /** \brief Stops the process-callback thread
   * \return 0 on success, otherwise a non-zero error code
   */
    virtual int stopProcess() const;
    /// \brief Connect the default ports, capture to sends, and receives to playback
    void connectDefaultPorts();

    //--------------SETTERS---------------------------------------------
    /// \brief Set Client Name to something different that the default (JackTrip)
    virtual void setClientName(QString ClientName)
    { mClientName = ClientName; }
    virtual void setSampleRate(uint32_t /*sample_rate*/)
    { std::cout << "WARNING: Setting the Sample Rate in Jack mode has no effect." << std::endl; }
    virtual void setBufferSizeInSamples(uint32_t /*buf_size*/)
    { std::cout << "WARNING: Setting the Sample Rate in Jack mode has no effect." << std::endl; }
    virtual void enableBroadcastOutput() {mBroadcast = true;}
    //------------------------------------------------------------------

    //--------------GETTERS---------------------------------------------
    /// \brief Get the Jack Server Sampling Rate, in samples/second
    virtual uint32_t getSampleRate() const;
    /// \brief Get the Jack Server Buffer Size, in samples
    virtual uint32_t getBufferSizeInSamples() const;
    /// \brief Get the Jack Server Buffer Size, in bytes
    virtual uint32_t getBufferSizeInBytes() const
    { return (getBufferSizeInSamples() * getAudioBitResolution() / 8); }
    /// \brief Get size of each audio per channel, in bytes
    virtual size_t getSizeInBytesPerChannel() const;
    //------------------------------------------------------------------

private:

    /** \brief Private method to setup a client of the Jack server.
   * \exception std::runtime_error Can't start Jack
   *
   * This method is called by the class constructors. It does the following:\n
   *  - Connects to the JACK server
   *  - Sets the shutdown process callback
   *  - Creates the appropriate number of input and output channels
   */
    void setupClient();
    /// \brief Creates input and output channels in the Jack client
    void createChannels();
    /** \brief JACK calls this shutdown_callback if the server ever shuts down or
   * decides to disconnect the client.
   */
    static void jackShutdown(void*);
    /** \brief Set the process callback of the member function processCallback.
   * This process will be called by the JACK server whenever there is work to be done.
   */
    void setProcessCallback();
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
#ifdef WAIR // WAIR
    int mNumNetRevChans; ///<  Number of Network Audio Channels (network comb filters
#endif // endwhere
    int mNumFrames; ///< Buffer block size, in samples
    //int mAudioBitResolution; ///< Bit resolution in audio samples
    AudioInterface::audioBitResolutionT mBitResolutionMode; ///< Bit resolution (audioBitResolutionT) mode

    jack_client_t* mClient; ///< Jack Client
    QString mClientName; ///< Jack Client Name
    QVarLengthArray<jack_port_t*> mInPorts; ///< Vector of Input Ports (Channels)
    QVarLengthArray<jack_port_t*> mOutPorts; ///< Vector of Output Ports (Channels)
    QVarLengthArray<jack_port_t*> mBroadcastPorts; ///< Vector of Output Ports (Channels)
    QVarLengthArray<sample_t*> mInBuffer; ///< Vector of Input buffers/channel read from JACK
    QVarLengthArray<sample_t*> mOutBuffer; ///< Vector of Output buffer/channel to write to JACK
    QVarLengthArray<sample_t*> mBroadcastBuffer; ///< Vector of Output buffer/channel to write to JACK
    bool mBroadcast;
    size_t mSizeInBytesPerChannel; ///< Size in bytes per audio channel
    QVector<ProcessPlugin*> mProcessPlugins; ///< Vector of ProcesPlugin<EM>s</EM>
    JackTrip* mJackTrip; ///< JackTrip mediator class
    static QMutex sJackMutex; ///< Mutex to make thread safe jack functions that are not
};


#endif
