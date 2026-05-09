//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

Copyright (c) 2008-2026 Juan-Pablo Caceres, Chris Chafe.
SoundWIRE group at CCRMA, Stanford University.
*/
//*****************************************************************

/**
 * \file PipewireAudioInterface.h
 * \author Chris Chafe
 * \date March 2026
 */

#ifndef __PIPEWIRE_AUDIO_INTERFACE_H__
#define __PIPEWIRE_AUDIO_INTERFACE_H__

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

#include <QMutex>
#include <QString>
#include <QVarLengthArray>
#include <QVector>

#include "AudioInterface.h"

class JackTrip;

/** \brief Class that provides an interface with PipeWire
 */
class PipewireAudioInterface : public AudioInterface
{
   public:
    /// \brief The class constructor
    PipewireAudioInterface(
        QVarLengthArray<int> InputChans, QVarLengthArray<int> OutputChans,
#ifdef WAIR
        int NumNetRevChans = 0,
#endif
        AudioInterface::audioBitResolutionT AudioBitResolution =
        AudioInterface::audioBitResolutionT::BIT32,
        bool processWithNetwork = false, JackTrip* jacktrip = nullptr,
        const QString& ClientName = QString("JackTrip"));

            /// \brief The class destructor
    virtual ~PipewireAudioInterface();

    void setup(bool verbose = true) override;
    int startProcess() override;
    int stopProcess() override;
    void connectDefaultPorts() override;
    uint32_t getSampleRate() const override;
    uint32_t getBufferSizeInSamples() const override;
    size_t getSizeInBytesPerChannel() const override;

            /// \brief Set the Client Name to something different that the default (JackTrip)
    virtual void setClientName(const QString& ClientName) { mClientName = ClientName; }

    /// \brief Get the assigned client name (may differ from requested if name collision)
    virtual QString getAssignedClientName() const { return mAssignedClientName; }

            /// \brief Tell PipeWire whether it should start the JACK server or not
            /// \param start true to start JACK server, false otherwise
    virtual void enableAutoStart(bool /*enable*/) {}

   private:
    void setupClient();
    void createStreams();

    // Process callback (for API compatibility)
    int processCallback(uint32_t nframes);

            // Static callback wrappers for PipeWire events - Input
    static void onInputStateChanged(void* data, enum pw_stream_state old,
                                    enum pw_stream_state state, const char* error);
    static void onInputParamChanged(void* data, uint32_t id, const struct spa_pod* param);
    static void onInputProcess(void* data);

            // Static callback wrappers for PipeWire events - Output
    static void onOutputStateChanged(void* data, enum pw_stream_state old,
                                     enum pw_stream_state state, const char* error);
    static void onOutputParamChanged(void* data, uint32_t id, const struct spa_pod* param);
    static void onOutputProcess(void* data);

            // Event structures
    static const struct pw_stream_events sInputEvents;
    static const struct pw_stream_events sOutputEvents;

            // PipeWire objects
    struct pw_thread_loop* mLoop;
    struct pw_context* mContext;
    struct pw_stream* mInputStream;
    struct pw_stream* mOutputStream;

    QString mClientName;
    QString mAssignedClientName;
    uint32_t mSampleRate;
    uint32_t mBufferSize;
    uint32_t mNumFrames;
    bool mStopped;

            // Buffer arrays (same as JACK)
    QVarLengthArray<sample_t*> mInBuffer;
    QVarLengthArray<sample_t*> mOutBuffer;

            // Mutex for thread safety and client numbering
    static QMutex sPWMutex;
    static int sClientNumber;
};

#endif  // __PIPEWIRE_AUDIO_INTERFACE_H__
