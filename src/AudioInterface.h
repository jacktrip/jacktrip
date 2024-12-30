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

#include <QSharedPointer>
#include <QVarLengthArray>
#include <QVector>
#include <functional>

#include "AudioTester.h"
#include "WaitFreeFrameBuffer.h"
#include "jacktrip_types.h"

// Forward declarations
class JackTrip;
class ProcessPlugin;

// using namespace JackTripNamespace;

// callback function for audio interface errors
typedef std::function<void(const std::string& errorText)> AudioErrorCallback;

/** \brief Base Class that provides an interface with audio
 */
class AudioInterface
{
   public:
    /// \brief Enum for Audio Resolution in bits
    enum audioBitResolutionT {
        BIT8  = 1,  ///< 8 bits
        BIT16 = 2,  ///< 16 bits (default)
        BIT24 = 3,  ///< 24 bits
        BIT32 = 4   ///< 32 bits
    };

    /// \brief Sampling Rates supported by JACK
    enum samplingRateT {
        SR22,   ///<  22050 Hz
        SR32,   ///<  32000 Hz
        SR44,   ///<  44100 Hz
        SR48,   ///<  48000 Hz
        SR88,   ///<  88200 Hz
        SR96,   ///<  96000 Hz
        SR192,  ///< 192000 Hz
        UNDEF   ///< Undefined
    };

    enum warningMessageT {
        DEVICE_WARN_NONE,
        DEVICE_WARN_BUFFER_LATENCY,
        DEVICE_WARN_ASIO_LATENCY,
        DEVICE_WARN_ALSA_LATENCY,
        DEVICE_WARN_SPEAKERS
    };

    enum errorMessageT {
        DEVICE_ERR_NONE,
        DEVICE_ERR_INCOMPATIBLE,
        DEVICE_ERR_NO_INPUTS,
        DEVICE_ERR_NO_OUTPUTS,
        DEVICE_ERR_NO_DEVICES,
#ifdef _WIN32
        DEVICE_ERR_SAME_ASIO
#endif
    };

    enum inputMixModeT : int {
        MIX_UNSET = 0,
        MONO      = 1,
        STEREO    = 2,
        MIXTOMONO = 3,
    };

    /** \brief The class constructor
     * \param NumInChans Number of Input Channels
     * \param NumOutChans Number of Output Channels
     * \param AudioBitResolution Audio Sample Resolutions in bits
     * \param processWithNetwork Send audio to and from the network
     * \param jacktrip Pointer to the JackTrip class that connects all classes (mediator)
     */
    AudioInterface(
        QVarLengthArray<int> InputChans, QVarLengthArray<int> OutputChans,
        inputMixModeT InputMixMode,
#ifdef WAIR  // wair
        int NumNetRevChans,
#endif  // endwhere
        AudioInterface::audioBitResolutionT AudioBitResolution = AudioInterface::BIT16,
        bool processWithNetwork = false, JackTrip* jacktrip = nullptr);

    /// \brief The class destructor
    virtual ~AudioInterface();

    /** \brief Setup the client. This function should be called just before
     *
     * starting the audio processes, it will setup the audio client with
     * the class parameters, like Sampling Rate,
     * Packet Size, Bit Resolution, etc... Sub-classes should also call the parent
     * method to ensure correct inizialization.
     */
    virtual void setup(bool verbose = true);

    /// \brief Tell the audio server that we are ready to roll. The
    /// process-callback will start running. This runs on its own thread.
    /// \return 0 on success, otherwise a non-zero error code
    virtual int startProcess() = 0;

    /// \brief Stops the process-callback thread
    /// \return 0 on success, otherwise a non-zero error code
    virtual int stopProcess() = 0;

    /** \brief Broadcast callback. Subclass should call this callback after
     * obtaining the mon_buffer pointer.
     *
     * \param in_buffer Array of input audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     * \param in_buffer Array of output audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     */
    virtual void broadcastCallback(QVarLengthArray<sample_t*>& mon_buffer,
                                   unsigned int n_frames);

    /** \brief Audio interface callback. Subclass should call this callback after
     * obtaining the in_buffer and out_buffer pointers (for duplux mode).
     *
     * \param in_buffer Array of input audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     * \param out_buffer Array of output audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     */
    virtual void callback(QVarLengthArray<sample_t*>& in_buffer,
                          QVarLengthArray<sample_t*>& out_buffer, unsigned int n_frames);

    /** \brief Audio input process callback. Subclass should call this callback
     * after obtaining the in_buffer pointer (for input only).
     *
     * \param in_buffer Array of input audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     */
    virtual void audioInputCallback(QVarLengthArray<sample_t*>& in_buffer,
                                    unsigned int n_frames);

    /** \brief Audio output process callback. Subclass should call this callback
     * after obtaining the out_buffer pointer (for output only).
     *
     * \param out_buffer Array of output audio samplers for each channel. The user
     * is responsible to check that each channel has n_frames samplers
     */
    virtual void audioOutputCallback(QVarLengthArray<sample_t*>& out_buffer,
                                     unsigned int n_frames);

    /** \brief appendProcessPluginToNetwork(): Append a ProcessPlugin for outgoing audio.
     * The processing order equals order they were appended.
     * This processing is in the JackTrip client before sending to the network.
     * \param plugin a ProcessPlugin smart pointer. Create the object instance
     * using something like:\n
     * <tt>std::tr1::shared_ptr<ProcessPluginName> loopback(new ProcessPluginName);</tt>
     */
    virtual void appendProcessPluginToNetwork(QSharedPointer<ProcessPlugin>& plugin);

    /** \brief appendProcessPluginFromNetwork():
     * Same as appendProcessPluginToNetwork() except that these plugins operate
     * on the audio received from the network (typically from a JackTrip server).
     * The complete processing chain then looks like this:
     * audio -> JACK -> JackTrip client -> processPlugin to network
     *               -> remote JackTrip server
     *               -> JackTrip client -> processPlugin from network -> JACK -> audio
     */
    virtual void appendProcessPluginFromNetwork(QSharedPointer<ProcessPlugin>& plugin);

    /** \brief appendProcessPluginToMonitor():
     * Appends plugins used for local monitoring
     */
    virtual void appendProcessPluginToMonitor(QSharedPointer<ProcessPlugin>& plugin);

    /** \brief initPlugins():
     * Initialize all ProcessPlugin modules.
     * The audio sampling rate (mSampleRate) must be set at this time.
     */
    void initPlugins(bool verbose = true);

    virtual void connectDefaultPorts() = 0;

    /** \brief Convert a 32bit number (sample_t) into one of the bit resolution
     * supported (audioBitResolutionT).
     *
     * The result is stored in an int_8 array of the
     * appropriate size to hold the value. The caller is responsible to allocate
     * enough space to store the result.
     */
    static void fromSampleToBitConversion(
        const sample_t* const input, int8_t* output,
        const AudioInterface::audioBitResolutionT targetBitResolution);

    /** \brief Convert a audioBitResolutionT bit resolution number into a
     * 32bit number (sample_t)
     *
     * The result is stored in an sample_t array of the
     * appropriate size to hold the value. The caller is responsible to allocate
     * enough space to store the result.
     */
    static void fromBitToSampleConversion(
        const int8_t* const input, sample_t* output,
        const AudioInterface::audioBitResolutionT sourceBitResolution);

    /** \brief Sets PIPEWIRE_LATENCY environment variable on unix */
    static void setPipewireLatency(unsigned int bufferSize, unsigned int sampleRate);

    //--------------SETTERS---------------------------------------------
    virtual void setInputChannels(QVarLengthArray<int> inputChans)
    {
        mInputChans = inputChans;
    }
    virtual void setOutputChannels(QVarLengthArray<int> outputChans)
    {
        mOutputChans = outputChans;
    }
    virtual void setInputMixMode(inputMixModeT mode) { mInputMixMode = mode; }
    virtual void setSampleRate(uint32_t sample_rate) { mSampleRate = sample_rate; }
    virtual void setBufferSize(uint32_t buffersize) { mBufferSizeInSamples = buffersize; }
    virtual void setDeviceID(uint32_t device_id) { mDeviceID = device_id; }
    virtual void setInputDevice(std::string device_name)
    {
        mInputDeviceName = device_name;
    }
    virtual void setOutputDevice(std::string device_name)
    {
        mOutputDeviceName = device_name;
    }
    virtual void setBufferSizeInSamples(uint32_t buf_size)
    {
        mBufferSizeInSamples = buf_size;
    }
    /// \brief Set Client Name to something different that the default (JackTrip)
    virtual void setClientName(const QString& ClientName) = 0;
    virtual void setLoopBack(bool b) { mLoopBack = b; }
    virtual void enableBroadcastOutput() {}
    virtual void setAudioTesterP(AudioTester* atp) { mAudioTesterP = atp; }
    void setErrorCallback(AudioErrorCallback c) { mErrorCallback = c; }
    //------------------------------------------------------------------

    //--------------GETTERS---------------------------------------------
    /// \brief Get Number of Input Channels
    virtual int getNumInputChannels() const { return mInputChans.size(); }
    /// \brief Get Number of Output Channels
    virtual int getNumOutputChannels() const { return mOutputChans.size(); }
    /// \brief Get Number of Monitor Channels
    virtual int getNumMonChannels() const { return mOutputChans.size(); }
    virtual QVarLengthArray<int> getInputChannels() const { return mInputChans; }
    virtual QVarLengthArray<int> getOutputChannels() const { return mOutputChans; }
    virtual inputMixModeT getInputMixMode() const { return mInputMixMode; }
    virtual uint32_t getBufferSizeInSamples() const { return mBufferSizeInSamples; }
    virtual uint32_t getDeviceID() const { return mDeviceID; }
    virtual std::string getInputDevice() const { return mInputDeviceName; }
    virtual std::string getOutputDevice() const { return mOutputDeviceName; }
    virtual size_t getSizeInBytesPerChannel() const;
    /// \brief Get the Jack Server Sampling Rate, in samples/second
    virtual uint32_t getSampleRate() const { return mSampleRate; }
    /// \brief Get the Jack Server Sampling Rate Enum Type samplingRateT
    /// \return  AudioInterface::samplingRateT enum type
    virtual samplingRateT getSampleRateType() const;
    /** \brief Get the Audio Bit Resolution, in bits
     *
     * This is one of the audioBitResolutionT set in construction
     */
    virtual int getAudioBitResolution() const { return mAudioBitResolution; }
    /** \brief Helper function to get the sample rate (in Hz) for a
     * JackAudioInterface::samplingRateT
     * \param rate_type  JackAudioInterface::samplingRateT enum type
     * \return Sample Rate in Hz
     */
    static int getSampleRateFromType(samplingRateT rate_type);
    const std::string& getDevicesWarningMsg() const { return mWarningMsg; }
    const std::string& getDevicesErrorMsg() const { return mErrorMsg; }
    const std::string& getDevicesWarningHelpUrl() const { return mWarningHelpUrl; }
    const std::string& getDevicesErrorHelpUrl() const { return mErrorHelpUrl; }
    bool highLatencyBufferSize() const { return getBufferSizeInSamples() > 256; }
    bool getHighLatencyFlag() const { return mHighLatencyFlag; }
    //------------------------------------------------------------------

   private:
    /// \brief Compute the process to receive packets
    void computeProcessFromNetwork(QVarLengthArray<sample_t*>& out_buffer,
                                   unsigned int n_frames);
    /// \brief Compute the process to send packets
    void computeProcessToNetwork(QVarLengthArray<sample_t*>& in_buffer,
                                 unsigned int n_frames);

    QVarLengthArray<int> mInputChans;
    QVarLengthArray<int> mOutputChans;
#ifdef WAIR               // wair
    int mNumNetRevChans;  ///<  Number of Network Audio Channels (net comb filters)
    QVarLengthArray<sample_t*>
        mNetInBuffer;  ///< Vector of Input buffers/channel read from net
    QVarLengthArray<sample_t*>
        mAPInBuffer;          ///< Vector of Input buffers/channel for AllPass input
#endif                        // endwhere
    int mAudioBitResolution;  ///< Bit resolution in audio samples
    AudioInterface::audioBitResolutionT
        mBitResolutionMode;  ///< Bit resolution (audioBitResolutionT) mode
    uint32_t mSampleRate;    ///< Sampling Rate
    uint32_t mDeviceID;      ///< RTAudio DeviceID
    std::string mInputDeviceName, mOutputDeviceName;  ///< RTAudio device names
    uint32_t mBufferSizeInSamples;                    ///< Buffer size in samples
    size_t mSizeInBytesPerChannel;                    ///< Size in bytes per audio channel
    QVector< QSharedPointer<ProcessPlugin> >
        mProcessPluginsFromNetwork;  ///< Vector of ProcessPlugin<EM>s</EM>
    QVector< QSharedPointer<ProcessPlugin> >
        mProcessPluginsToNetwork;  ///< Vector of ProcessPlugin<EM>s</EM>
    QVector< QSharedPointer<ProcessPlugin> >
        mProcessPluginsToMonitor;  ///< Vector of ProcessPlugin<EM>s</EM>
    QVarLengthArray<sample_t*>
        mInProcessBuffer;  ///< Vector of Input buffers/channel for ProcessPlugin
    QVarLengthArray<sample_t*>
        mOutProcessBuffer;  ///< Vector of Output buffers/channel for ProcessPlugin
    WaitFreeFrameBuffer<64>*
        mMonitorQueuePtr;        //< Queue of audio frames from monitor signal
    int8_t* mAudioInputPacket;   ///< Packet containing all the channels to read from the
                                 ///< RingBuffer
    int8_t* mAudioOutputPacket;  ///< Packet containing all the channels to send to the
                                 ///< RingBuffer
    bool mLoopBack;
    bool mProcessWithNetwork;  ///< whether or not to send/receive data via the network
    bool mMonitorStarted;      ///< True if we have started to consume monitor audio
    AudioTester* mAudioTesterP{nullptr};

   protected:
    JackTrip* mJackTrip;          ///< JackTrip Mediator Class pointer
    inputMixModeT mInputMixMode;  ///< Input mixing mode

    void setDevicesWarningMsg(warningMessageT msg);
    void setDevicesErrorMsg(errorMessageT msg);

    bool mProcessingAudio;  ///< Set when processing an audio callback buffer pair
    const uint32_t MAX_AUDIO_BUFFER_SIZE = 8192;

    std::string mWarningMsg;
    std::string mErrorMsg;
    std::string mWarningHelpUrl;
    std::string mErrorHelpUrl;
    bool mHighLatencyFlag;
    AudioErrorCallback mErrorCallback;
};

#endif  // __AUDIOINTERFACE_H__
