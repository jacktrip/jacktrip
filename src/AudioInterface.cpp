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
 * \file AudioInterface.cpp
 * \author Juan-Pablo Caceres
 * \date July 2009
 */

#include "AudioInterface.h"

#include <cassert>
#include <cmath>
#include <iostream>

#include "JackTrip.h"

using std::cout;
using std::endl;

//*******************************************************************************
AudioInterface::AudioInterface(QVarLengthArray<int> InputChans,
                               QVarLengthArray<int> OutputChans,
                               inputMixModeT InputMixMode,
#ifdef WAIR  // wair
                               int NumNetRevChans,
#endif  // endwhere
                               audioBitResolutionT AudioBitResolution,
                               bool processWithNetwork, JackTrip* jacktrip)
    : mInputChans(InputChans)
    , mOutputChans(OutputChans)
    ,
#ifdef WAIR  // WAIR
    mNumNetRevChans(NumNetRevChans)
    ,
#endif  // endwhere
    mAudioBitResolution(AudioBitResolution * 8)
    , mBitResolutionMode(AudioBitResolution)
    , mSampleRate(gDefaultSampleRate)
    , mBufferSizeInSamples(gDefaultBufferSizeInSamples)
    , mAudioInputPacket(NULL)
    , mAudioOutputPacket(NULL)
    , mLoopBack(false)
    , mProcessWithNetwork(processWithNetwork)
    , mJackTrip(jacktrip)
    , mInputMixMode(InputMixMode)
    , mProcessingAudio(false)
{
#ifndef WAIR
    // cc
    // Initialize and assign memory for ProcessPlugins Buffers
    int monitorChans = std::min(mInputChans.size(), mOutputChans.size());
    mInProcessBuffer.resize(mInputChans.size());
    mOutProcessBuffer.resize(mOutputChans.size());
    mMonProcessBuffer.resize(monitorChans);
    // Set pointer to NULL
    for (int i = 0; i < mInProcessBuffer.size(); i++) {
        mInProcessBuffer[i] = NULL;
    }
    for (int i = 0; i < mOutProcessBuffer.size(); i++) {
        mOutProcessBuffer[i] = NULL;
    }
    for (int i = 0; i < monitorChans; i++) {
        mMonProcessBuffer[i] = NULL;
    }
#else   // WAIR
    int iCnt =
        (mInputChans.size() > mNumNetRevChans) ? mInputChans.size() : mNumNetRevChans;
    int oCnt =
        (mOutputChans.size() > mNumNetRevChans) ? mOutputChans.size() : mNumNetRevChans;
    int aCnt = (mNumNetRevChans) ? mInputChans.size() : 0;
    int mCnt = std::min(iCnt, oCnt);
    for (int i = 0; i < iCnt; i++) {
        mInProcessBuffer[i] = NULL;
    }
    for (int i = 0; i < oCnt; i++) {
        mOutProcessBuffer[i] = NULL;
    }
    for (int i = 0; i < mCnt; i++) {
        mMonProcessBuffer[i] = NULL;
    }
    for (int i = 0; i < aCnt; i++) {
        mAPInBuffer[i] = NULL;
    }
#endif  // endwhere

    mInBufCopy.resize(mInputChans.size());
    for (int i = 0; i < mInputChans.size(); i++) {
        mInBufCopy[i] =
            new sample_t[MAX_AUDIO_BUFFER_SIZE];  // required for processing audio input
    }

    // Not used in this class but may be needed by subclasses
    mNumInChans  = mInputChans.size();
    mNumOutChans = mOutputChans.size();
}

//*******************************************************************************
AudioInterface::~AudioInterface()
{
    delete[] mAudioInputPacket;
    delete[] mAudioOutputPacket;
#ifndef WAIR  // NOT WAIR:
    for (int i = 0; i < mInProcessBuffer.size(); i++) {
        delete[] mInProcessBuffer[i];
    }

    for (int i = 0; i < mOutProcessBuffer.size(); i++) {
        delete[] mOutProcessBuffer[i];
    }
    for (int i = 0; i < mMonProcessBuffer.size(); i++) {
        delete[] mMonProcessBuffer[i];
    }
#else   // WAIR
    for (int i = 0; i < mInProcessBuffer.size(); i++) {
        delete[] mInProcessBuffer[i];
    }
    for (int i = 0; i < mOutProcessBuffer.size(); i++) {
        delete[] mOutProcessBuffer[i];
    }
    for (int i = 0; i < mMonProcessBuffer.size(); i++) {
        delete[] mMonProcessBuffer[i];
    }
    for (int i = 0; i < mAPInBuffer.size(); i++) {
        delete[] mAPInBuffer[i];
    }
#endif  // endwhere
    for (auto* i : qAsConst(mProcessPluginsFromNetwork)) {
        delete i;
    }
    for (auto* i : qAsConst(mProcessPluginsToNetwork)) {
        delete i;
    }
    for (auto* i : qAsConst(mProcessPluginsToMonitor)) {
        delete i;
    }
    for (int i = 0; i < mInBufCopy.size(); i++) {
        delete[] mInBufCopy[i];
    }
}

//*******************************************************************************
void AudioInterface::setup(bool /*verbose*/)
{
    int nChansIn  = mInputChans.size();
    int nChansOut = mOutputChans.size();
    int nChansMon =
        std::min(nChansIn, nChansOut);  // Note: Should be 2 when mixing stereo-to-mono
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }
    if (inputMixMode == MONO) {
        nChansMon = nChansOut;
    }
    // Allocate buffer memory to read and write
    mSizeInBytesPerChannel = getSizeInBytesPerChannel();

    int size_audio_input  = mSizeInBytesPerChannel * nChansIn;
    int size_audio_output = mSizeInBytesPerChannel * nChansOut;
#ifdef WAIR               // WAIR
    if (mNumNetRevChans)  // else don't change sizes
    {
        size_audio_input  = mSizeInBytesPerChannel * mNumNetRevChans;
        size_audio_output = mSizeInBytesPerChannel * mNumNetRevChans;
    }
#endif  // endwhere
    mAudioInputPacket  = new int8_t[size_audio_input];
    mAudioOutputPacket = new int8_t[size_audio_output];

    // Initialize and assign memory for ProcessPlugins Buffers
#ifdef WAIR  // WAIR
    if (mNumNetRevChans) {
        mInProcessBuffer.resize(mNumNetRevChans);
        mOutProcessBuffer.resize(mNumNetRevChans);
        mMonProcessBuffer.resize(mNumNetRevChans);
        mAPInBuffer.resize(nChansIn);
        mNetInBuffer.resize(mNumNetRevChans);
    } else  // don't change sizes
#endif      // endwhere
    {
        mInProcessBuffer.resize(nChansIn);
        mOutProcessBuffer.resize(nChansOut);
        mMonProcessBuffer.resize(nChansMon);
    }

    int nframes = getBufferSizeInSamples();

#ifndef WAIR  // NOT WAIR:
    for (int i = 0; i < nChansIn; i++) {
        mInProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mInProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < nChansOut; i++) {
        mOutProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mOutProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < nChansMon; i++) {
        mMonProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mMonProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    }
#else   // WAIR
    for (int i = 0; i < ((mNumNetRevChans) ? mNumNetRevChans : nChansIn); i++) {
        mInProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mInProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < ((mNumNetRevChans) ? mNumNetRevChans : nChansOut); i++) {
        mOutProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mOutProcessBuffer[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < ((mNumNetRevChans) ? mNumNetRevChans : nChansMon); i++) {
        mMonProcessBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mMonitorProcess[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < ((mNumNetRevChans) ? nChansIn : 0); i++) {
        mAPInBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mAPInBuffer[i], 0, sizeof(sample_t) * nframes);
    }
    for (int i = 0; i < mNumNetRevChans; i++) {
        mNetInBuffer[i] = new sample_t[nframes];
        // set memory to 0
        std::memset(mNetInBuffer[i], 0, sizeof(sample_t) * nframes);
    }
#endif  // endwhere
}

//*******************************************************************************
size_t AudioInterface::getSizeInBytesPerChannel() const
{
    return (getBufferSizeInSamples() * getAudioBitResolution() / 8);
}

//*******************************************************************************
void AudioInterface::callback(QVarLengthArray<sample_t*>& in_buffer,
                              QVarLengthArray<sample_t*>& out_buffer,
                              unsigned int n_frames)
{
    int nChansIn  = mInputChans.size();
    int nChansOut = mOutputChans.size();
    int nChansMon =
        std::min(nChansIn, nChansOut);  // Note: Should be 2 when mixing stereo-to-mono
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }
    if (inputMixMode == MONO) {
        nChansMon = nChansOut;
    }
    // Allocate the Process Callback
    //-------------------------------------------------------------------
    // 1) First, process incoming packets
    // ----------------------------------

#ifdef WAIR  // WAIR
    //    qDebug() << "--" << mProcessPluginsFromNetwork.size();
    bool client = (mProcessPluginsFromNetwork.size() == 2);
#define COMBDSP 1  // client
#define APDSP   0  // client
#define DCBDSP  0  // server
    for (int i = 0; i < mNumNetRevChans; i++) {
        std::memset(mNetInBuffer[i], 0, sizeof(sample_t) * n_frames);
    }
#endif  // endwhere

    // ==== RECEIVE AUDIO CHANNELS FROM NETWORK ====
    if (mProcessWithNetwork) {
        computeProcessFromNetwork(out_buffer, n_frames);
    }
    // =============================================

    // out_buffer is from the network and goes "out" to local audio
    // hardware via JACK:

    // mAudioTesterP will be nullptr for hub server's JackTripWorker instances
    if (mAudioTesterP && mAudioTesterP->getEnabled()) {
        mAudioTesterP->lookForReturnPulse(out_buffer, n_frames);
    }

#ifdef WAIR  // WAIR
    // nib16 result now in mNetInBuffer
#endif  // endwhere

    // 2) Dynamically allocate ProcessPlugin processes
    // -----------------------------------------------
    // The processing will be done in order of allocation
    /// \todo Implement for more than one process plugin, now it just works propertely
    /// with one. do it chaining outputs to inputs in the buffers. May need a tempo buffer

#ifndef WAIR  // NOT WAIR:
    for (auto* p : qAsConst(mProcessPluginsFromNetwork)) {
        if (p->getInited()) {
            p->compute(n_frames, out_buffer.data(), out_buffer.data());
        }
    }
#else   // WAIR:
    for (int i = 0; i < ((mNumNetRevChans) ? mNumNetRevChans : nChansOut); i++) {
        std::memset(mOutProcessBuffer[i], 0, sizeof(sample_t) * n_frames);
    }
    for (int i = 0; i < ((mNumNetRevChans) ? mNumNetRevChans : nChansIn); i++) {
        std::memset(mInProcessBuffer[i], 0, sizeof(sample_t) * n_frames);
        if (mNumNetRevChans) {
            if (client)
                std::memcpy(mInProcessBuffer[i], mNetInBuffer[i],
                            sizeof(sample_t) * n_frames);
            else
                std::memcpy(mOutProcessBuffer[i], mNetInBuffer[i],
                            sizeof(sample_t) * n_frames);
        }
    }
    // nib16 to cib16

    if (mNumNetRevChans && client) {
        mProcessPluginsFromNetwork[COMBDSP]->compute(n_frames, mInProcessBuffer.data(),
                                                     mOutProcessBuffer.data());
    }
    // compute cob16
#endif  // endwhere

    // 3) Send packets to network:
    // mAudioTesterP will be nullptr for hub server's JackTripWorker instances:
    bool audioTesting = (mAudioTesterP && mAudioTesterP->getEnabled());
    int nop = mProcessPluginsToNetwork.size();  // number of OUTGOING processing modules
    if (nop > 0 || audioTesting
        || mProcessPluginsToMonitor.size()
               > 0) {  // cannot modify in_buffer, so make a copy
        // in_buffer is "in" from local audio hardware via JACK
        if (mInBufCopy.size() < nChansIn) {  // created in constructor above
            std::cerr << "*** AudioInterface.cpp: Number of Input Channels changed - "
                         "insufficient room reserved\n";
            exit(1);
        }
        if (MAX_AUDIO_BUFFER_SIZE < n_frames) {  // allocated in constructor above
            std::cerr << "*** AudioInterface.cpp: n_frames = " << n_frames
                      << " larger than expected max = " << MAX_AUDIO_BUFFER_SIZE << "\n";
            exit(1);
        }
        for (int i = 0; i < nChansIn; i++) {
            std::memcpy(mInBufCopy[i], in_buffer[i], sizeof(sample_t) * n_frames);
        }
        for (int i = 0; i < nop; i++) {
            // process all outgoing channels with ProcessPlugins:
            ProcessPlugin* p = mProcessPluginsToNetwork[i];
            if (p->getInited()) {
                p->compute(n_frames, mInBufCopy.data(), mInBufCopy.data());
            }
        }

        for (int i = 0; i < nChansMon; i++) {
            if ((mInputChans.size() == 2 && mInputMixMode == AudioInterface::MIXTOMONO)
                || (mInputMixMode == AudioInterface::MONO)) {
                // if using mix-to-mono, in_buffer[0] should already contain the mixed
                // audio, so copy it to the monitor buffer. See RtAudioInterface.cpp

                // likewise if using mono, we simply copy the input to every monitor
                // channel
                std::memcpy(mMonProcessBuffer[i], in_buffer[0],
                            sizeof(sample_t) * n_frames);
            } else {
                // otherwise, copy each channel individually
                std::memcpy(mMonProcessBuffer[i], in_buffer[i],
                            sizeof(sample_t) * n_frames);
            }
        }
        for (int i = 0; i < mProcessPluginsToMonitor.size(); i++) {
            ProcessPlugin* p = mProcessPluginsToMonitor[i];
            if (p->getInited()) {
                // note: for monitor plugins, the output is out_buffer (to the speakers)
                p->compute(n_frames, mMonProcessBuffer.data(), out_buffer.data());
            }
        }

        if (audioTesting) {
            mAudioTesterP->writeImpulse(
                mInBufCopy,
                n_frames);  // writes last channel of mInBufCopy with test impulse
        }
        if (mProcessWithNetwork) {
            computeProcessToNetwork(mInBufCopy, n_frames);
        }
    } else {  // copy saved if no plugins and no audio testing in progress:
        if (mProcessWithNetwork) {
            computeProcessToNetwork(
                in_buffer, n_frames);  // send processed input audio to network - OUTGOING
        }
    }

#ifdef WAIR  // WAIR
    // aib2 + cob16 to nob16
#endif  // endwhere

#ifdef WAIR               // WAIR
    if (mNumNetRevChans)  // else not wair, so skip all this
    {
#define AP
#ifndef AP
        // straight to audio out
        for (int i = 0; i < nChansOut; i++) {
            std::memset(out_buffer[i], 0, sizeof(sample_t) * n_frames);
        }
        for (int i = 0; i < mNumNetRevChans; i++) {
            sample_t* mix_sample = out_buffer[i % nChansOut];
            sample_t* tmp_sample = mNetInBuffer[i];  // mNetInBuffer
            for (int j = 0; j < (int)n_frames; j++) {
                mix_sample[j] += tmp_sample[j];
            }
        }  // nib6 to aob2
#else      // AP

        // output through all-pass cascade
        // AP2 is 2 channel, mixes inputs to mono, then splits to two parallel AP chains
        // AP8 is 2 channel, two parallel AP chains
        for (int i = 0; i < nChansIn; i++) {
            std::memset(mAPInBuffer[i], 0, sizeof(sample_t) * n_frames);
        }
        for (int i = 0; i < mNumNetRevChans; i++) {
            sample_t* mix_sample = mAPInBuffer[i % nChansOut];
            sample_t* tmp_sample = mNetInBuffer[i];
            for (int j = 0; j < n_frames; j++) {
                mix_sample[j] += tmp_sample[j];
            }
        }  // nib16 to apib2
        for (int i = 0; i < nChansOut; i++) {
            std::memset(out_buffer[i], 0, sizeof(sample_t) * n_frames);
        }
        mProcessPluginsFromNetwork[APDSP]->compute(n_frames, mAPInBuffer.data(),
                                                   out_buffer.data());
        // compute ap2 into aob2

        //#define ADD_DIRECT
#ifdef ADD_DIRECT
        for (int i = 0; i < nChansIn; i++) {
            sample_t* mix_sample = out_buffer[i];
            sample_t* tmp_sample = in_buffer[i];
            for (int j = 0; j < n_frames; j++) {
                mix_sample[j] += tmp_sample[j];
            }
        }
        // add aib2 to aob2
#endif  // ADD_DIRECT
#endif  // AP
    }
#endif  // endwhere

    ///************PROTOTYPE FOR CELT**************************
    ///********************************************************
    /*
  CELTMode* mode;
  int* error;
  mode = celt_mode_create(48000, 2, 64, error);
  */
    // celt_mode_create(48000, 2, 64, NULL);
    // unsigned char* compressed;
    // CELTEncoder* celtEncoder;
    // celt_encode_float(celtEncoder, mInBuffer, NULL, compressed, );

    ///********************************************************
    ///********************************************************
}

//*******************************************************************************
void AudioInterface::broadcastCallback(QVarLengthArray<sample_t*>& mon_buffer,
                                       unsigned int n_frames)
{
    int nChansOut = mOutputChans.size();

    /// \todo cast *mInBuffer[i] to the bit resolution
    // Output Process (from NETWORK to JACK)
    // ----------------------------------------------------------------
    // Read Audio buffer from RingBuffer (read from incoming packets)
    mJackTrip->receiveBroadcastPacket(mAudioOutputPacket);
    // Extract separate channels to send to Jack
    for (int i = 0; i < nChansOut; i++) {
        sample_t* tmp_sample = mon_buffer[i];  // sample buffer for channel i
        for (unsigned int j = 0; j < n_frames; j++) {
            // Change the bit resolution on each sample
            fromBitToSampleConversion(
                // use interleaved channel layout
                //&mOutputPacket[(i*mSizeInBytesPerChannel) + (j*mBitResolutionMode)],
                &mAudioOutputPacket[(j * mBitResolutionMode * nChansOut)
                                    + (i * mBitResolutionMode)],
                &tmp_sample[j], mBitResolutionMode);
        }
    }
}

//*******************************************************************************
// Before sending and reading to Jack, we have to round to the sample resolution
// that the program is using. Jack uses 32 bits (gJackBitResolution in globals.h)
// by default
void AudioInterface::computeProcessFromNetwork(QVarLengthArray<sample_t*>& out_buffer,
                                               unsigned int n_frames)
{
    int nChansOut = mOutputChans.size();

    /// \todo cast *mInBuffer[i] to the bit resolution
    // Output Process (from NETWORK to JACK)
    // ----------------------------------------------------------------
    // Read Audio buffer from RingBuffer (read from incoming packets)
    mJackTrip->receiveNetworkPacket(mAudioOutputPacket);

#ifdef WAIR  // WAIR
    if (mNumNetRevChans)
        // Extract separate channels
        for (int i = 0; i < mNumNetRevChans; i++) {
            sample_t* tmp_sample = mNetInBuffer[i];  // sample buffer for channel i
            for (unsigned int j = 0; j < n_frames; j++) {
                // Change the bit resolution on each sample
                fromBitToSampleConversion(
                    // use interleaved channel layout
                    //&mOutputPacket[(i*mSizeInBytesPerChannel) + (j*mBitResolutionMode)],
                    &mOutputPacket[(j * mBitResolutionMode * nChansOut)
                                   + (i * mBitResolutionMode)],
                    &tmp_sample[j], mBitResolutionMode);
            }
        }
    else  // not wair
#endif    // endwhere

        // Extract separate channels to send to Jack
        for (int i = 0; i < nChansOut; i++) {
            //--------
            // This should be faster for 32 bits
            // std::memcpy(mOutBuffer[i], &mOutputPacket[i*mSizeInBytesPerChannel],
            //         mSizeInBytesPerChannel);
            //--------
            sample_t* tmp_sample = out_buffer[i];  // sample buffer for channel i
            for (unsigned int j = 0; j < n_frames; j++) {
                // Change the bit resolution on each sample
                fromBitToSampleConversion(
                    // use interleaved channel layout
                    //&mOutputPacket[(i*mSizeInBytesPerChannel) + (j*mBitResolutionMode)],
                    &mAudioOutputPacket[(j * mBitResolutionMode * nChansOut)
                                        + (i * mBitResolutionMode)],
                    &tmp_sample[j], mBitResolutionMode);
            }
        }
}

//*******************************************************************************
void AudioInterface::computeProcessToNetwork(QVarLengthArray<sample_t*>& in_buffer,
                                             unsigned int n_frames)
{
    int nChansIn               = mInputChans.size();
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }
    // Input Process (from JACK to NETWORK)
    // ----------------------------------------------------------------
    // Concatenate  all the channels from jack to form packet

#ifdef WAIR  // WAIR
    if (mNumNetRevChans)
        for (int i = 0; i < mNumNetRevChans; i++) {
            sample_t* tmp_sample =
                in_buffer[i % nChansIn];  // sample buffer for channel i
            sample_t* tmp_process_sample =
                mInProcessBuffer[i];  // sample buffer from the output process
            sample_t tmp_result;
            for (unsigned int j = 0; j < n_frames; j++) {
                // Change the bit resolution on each sample
                // Add the input jack buffer to the buffer resulting from the output
                // process
#define INGAIN \
    (0.9999)  // 0.9999 because 1.0 can saturate the fixed pt rounding on output
#define COMBGAIN (1.0)
                tmp_result = INGAIN * tmp_sample[j] + COMBGAIN * tmp_process_sample[j];
                fromSampleToBitConversion(
                    &tmp_result,
                    // use interleaved channel layout
                    //&mInputPacket[(i*mSizeInBytesPerChannel) + (j*mBitResolutionMode)],
                    &mInputPacket[(j * mBitResolutionMode * nChansOut)
                                  + (i * mBitResolutionMode)],
                    mBitResolutionMode);
            }
        }
    else  // not wair
#endif    // endwhere

        for (int i = 0; i < nChansIn; i++) {
            //--------
            // This should be faster for 32 bits
            // std::memcpy(&mInputPacket[i*mSizeInBytesPerChannel], mInBuffer[i],
            //         mSizeInBytesPerChannel);
            //--------
            sample_t* tmp_sample = in_buffer[i];  // sample buffer for channel i
            sample_t* tmp_process_sample =
                mInProcessBuffer[i];  // sample buffer from the output process
            sample_t tmp_result;
            for (unsigned int j = 0; j < n_frames; j++) {
                // Change the bit resolution on each sample
                // Add the input jack buffer to the buffer resulting from the output
                // process
                tmp_result = tmp_sample[j] + tmp_process_sample[j];
                fromSampleToBitConversion(
                    &tmp_result,
                    // use interleaved channel layout
                    //&mInputPacket[(i*mSizeInBytesPerChannel) + (j*mBitResolutionMode)],
                    &mAudioInputPacket[(j * mBitResolutionMode * nChansIn)
                                       + (i * mBitResolutionMode)],
                    mBitResolutionMode);
            }
        }
    // Send Audio buffer to Network
    mJackTrip->sendNetworkPacket(mAudioInputPacket);
}  // /computeProcessToNetwork

//*******************************************************************************
// This function quantize from 32 bit to a lower bit resolution
// 24 bit is not working yet
void AudioInterface::fromSampleToBitConversion(
    const sample_t* const input, int8_t* output,
    const AudioInterface::audioBitResolutionT targetBitResolution)
{
    int8_t tmp_8;
    uint8_t tmp_u8;  // unsigned to quantize the remainder in 24bits
    int16_t tmp_16;
    double tmp_sample;
    sample_t tmp_sample16;
    sample_t tmp_sample8;
    switch (targetBitResolution) {
    case BIT8:
        // 8bit integer between -128 to 127
        tmp_sample =
            std::max(-127.0, std::min(127.0, std::round((*input) * 127.0)));  // 2^7 = 128
        tmp_8 = static_cast<int8_t>(tmp_sample);
        std::memcpy(output, &tmp_8, 1);  // 8bits = 1 bytes
        break;
    case BIT16:
        // 16bit integer between -32768 to 32767
        // original scaling: tmp_sample = floor( (*input) * 32768.0 ); // 2^15 = 32768.0
        tmp_sample = std::max(
            -32767.0, std::min(32767.0, std::round((*input) * 32767.0)));  // 2^15 = 32768
        tmp_16 = static_cast<int16_t>(tmp_sample);
        std::memcpy(
            output, &tmp_16,
            2);  // 2 bytes output in Little Endian order (LSB -> smallest address)
        break;
    case BIT24:
        // To convert to 24 bits, we first quantize the number to 16bit
        tmp_sample   = (*input) * 32768.0;  // 2^15 = 32768.0
        tmp_sample16 = floor(tmp_sample);
        tmp_16       = static_cast<int16_t>(tmp_sample16);

        // Then we compute the remainder error, and quantize that part into an 8bit number
        // Note that this remainder is always positive, so we use an unsigned integer
        tmp_sample8 = floor(
            (tmp_sample - tmp_sample16)  // this is a positive number, between 0.0-1.0
            * 256.0);
        tmp_u8 = static_cast<uint8_t>(tmp_sample8);

        // Finally, we copy the 16bit number in the first 2 bytes,
        // and the 8bit number in the third bite
        std::memcpy(output, &tmp_16, 2);      // 16bits = 2 bytes
        std::memcpy(output + 2, &tmp_u8, 1);  // 8bits = 1 bytes
        break;
    case BIT32:
        tmp_sample = *input;
        // not necessary yet:
        // tmp_sample = std::max(-1.0, std::min(1.0, tmp_sample));
        std::memcpy(output, &tmp_sample, 4);  // 32bit = 4 bytes
        break;
    }
}

//*******************************************************************************
void AudioInterface::fromBitToSampleConversion(
    const int8_t* const input, sample_t* output,
    const AudioInterface::audioBitResolutionT sourceBitResolution)
{
    int8_t tmp_8;
    uint8_t tmp_u8;
    int16_t tmp_16;
    sample_t tmp_sample;
    sample_t tmp_sample16;
    sample_t tmp_sample8;
    switch (sourceBitResolution) {
    case BIT8:
        tmp_8      = *input;
        tmp_sample = static_cast<sample_t>(tmp_8) / 128.0;
        std::memcpy(output, &tmp_sample, 4);  // 4 bytes
        break;
    case BIT16:
        tmp_16     = *(reinterpret_cast<const int16_t*>(input));  // *((int16_t*) input);
        tmp_sample = static_cast<sample_t>(tmp_16) / 32768.0;
        std::memcpy(output, &tmp_sample, 4);  // 4 bytes
        break;
    case BIT24:
        // We first extract the 16bit and 8bit number from the 3 bytes
        tmp_16 = *(reinterpret_cast<const int16_t*>(input));
        tmp_u8 = *(reinterpret_cast<const uint8_t*>(input + 2));

        // Then we recover the number
        tmp_sample16 = static_cast<sample_t>(tmp_16);
        tmp_sample8  = static_cast<sample_t>(tmp_u8) / 256.0;
        tmp_sample   = (tmp_sample16 + tmp_sample8) / 32768.0;
        std::memcpy(output, &tmp_sample, 4);  // 4 bytes
        break;
    case BIT32:
        std::memcpy(output, input, 4);  // 4 bytes
        break;
    }
}

//*******************************************************************************
void AudioInterface::appendProcessPluginToNetwork(ProcessPlugin* plugin)
{
    if (not plugin) {
        return;
    }

    int nChansIn               = mInputChans.size();
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }

    int nTestChans   = (mAudioTesterP && mAudioTesterP->getEnabled()) ? 1 : 0;
    int nPluginChans = nChansIn - nTestChans;
    assert(nTestChans == 0 || (mAudioTesterP->getSendChannel() == nChansIn - 1));
    if (plugin->getNumInputs() < nPluginChans) {
        std::cerr
            << "*** AudioInterface.cpp: appendProcessPluginToNetwork: ProcessPlugin "
            << typeid(plugin).name() << " REJECTED due to having "
            << plugin->getNumInputs() << " inputs, while the audio to JACK needs "
            << nPluginChans << " inputs\n";
        return;
    }
    mProcessPluginsToNetwork.append(plugin);
}

void AudioInterface::appendProcessPluginFromNetwork(ProcessPlugin* plugin)
{
    if (not plugin) {
        return;
    }

    int nChansOut = mOutputChans.size();

    int nTestChans   = (mAudioTesterP && mAudioTesterP->getEnabled()) ? 1 : 0;
    int nPluginChans = nChansOut - nTestChans;
    assert(nTestChans == 0 || (mAudioTesterP->getSendChannel() == nChansOut - 1));
    if (plugin->getNumOutputs() > nPluginChans) {
        std::cerr
            << "*** AudioInterface.cpp: appendProcessPluginFromNetwork: ProcessPlugin "
            << typeid(plugin).name() << " REJECTED due to having "
            << plugin->getNumOutputs() << " inputs, while the JACK audio output requires "
            << nPluginChans << " outputs\n";
        return;
    }
    mProcessPluginsFromNetwork.append(plugin);
}

void AudioInterface::appendProcessPluginToMonitor(ProcessPlugin* plugin)
{
    if (not plugin) {
        return;
    }
    int nChansIn  = mInputChans.size();
    int nChansOut = mOutputChans.size();
    int nChansMon =
        std::min(nChansIn, nChansOut);  // Note: Should be 2 when mixing stereo-to-mono
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }
    if (inputMixMode == MONO) {
        nChansMon = nChansOut;
    }
    if (plugin->getNumInputs() > nChansMon) {
        std::cerr
            << "*** AudioInterface.cpp: appendProcessPluginToMonitor: ProcessPlugin "
            << typeid(plugin).name() << " REJECTED due to having "
            << plugin->getNumInputs()
            << " inputs, while the monitor audio input requires " << nChansMon
            << " outputs\n";
        return;
    }

    if (plugin->getNumOutputs() > nChansMon) {
        std::cerr
            << "*** AudioInterface.cpp: appendProcessPluginToMonitor: ProcessPlugin "
            << typeid(plugin).name() << " REJECTED due to having "
            << plugin->getNumOutputs()
            << " inputs, while the monitor audio output requires " << nChansMon
            << " outputs\n";
        return;
    }

    mProcessPluginsToMonitor.append(plugin);
}

void AudioInterface::initPlugins(bool verbose)
{
    int nChansIn  = mInputChans.size();
    int nChansOut = mOutputChans.size();
    int nChansMon =
        std::min(nChansIn, nChansOut);  // Note: Should be 2 when mixing stereo-to-mono
    inputMixModeT inputMixMode = mInputMixMode;
    if (inputMixMode == MIXTOMONO) {
        nChansIn = 1;
    }
    if (inputMixMode == MONO) {
        nChansMon = nChansOut;
    }
    int nPlugins = mProcessPluginsFromNetwork.size() + mProcessPluginsToNetwork.size()
                   + mProcessPluginsToMonitor.size();
    if (nPlugins > 0) {
        if (verbose) {
            std::cout << "Initializing Faust plugins (have " << nPlugins
                      << ") at sampling rate " << mSampleRate << "\n";
        }

        for (ProcessPlugin* plugin : qAsConst(mProcessPluginsFromNetwork)) {
            plugin->setOutgoingToNetwork(false);
            plugin->updateNumChannels(nChansIn, nChansOut);
            plugin->init(mSampleRate);
        }
        for (ProcessPlugin* plugin : qAsConst(mProcessPluginsToNetwork)) {
            plugin->setOutgoingToNetwork(true);
            plugin->updateNumChannels(nChansIn, nChansOut);
            plugin->init(mSampleRate);
        }
        for (ProcessPlugin* plugin : qAsConst(mProcessPluginsToMonitor)) {
            plugin->setOutgoingToNetwork(false);
            plugin->updateNumChannels(nChansMon, nChansMon);
            plugin->init(mSampleRate);
        }
    }
}

//*******************************************************************************
AudioInterface::samplingRateT AudioInterface::getSampleRateType() const
{
    int32_t rate = getSampleRate();

    if (100 > qAbs(rate - 22050)) {
        return AudioInterface::SR22;
    } else if (100 > qAbs(rate - 32000)) {
        return AudioInterface::SR32;
    } else if (100 > qAbs(rate - 44100)) {
        return AudioInterface::SR44;
    } else if (100 > qAbs(rate - 48000)) {
        return AudioInterface::SR48;
    } else if (100 > qAbs(rate - 88200)) {
        return AudioInterface::SR88;
    } else if (100 > qAbs(rate - 96000)) {
        return AudioInterface::SR96;
    } else if (100 > qAbs(rate - 19200)) {
        return AudioInterface::SR192;
    }

    return AudioInterface::UNDEF;
}

//*******************************************************************************
int AudioInterface::getSampleRateFromType(samplingRateT rate_type)
{
    int sample_rate = 0;
    switch (rate_type) {
    case SR22:
        sample_rate = 22050;
        return sample_rate;
        break;
    case SR32:
        sample_rate = 32000;
        return sample_rate;
        break;
    case SR44:
        sample_rate = 44100;
        return sample_rate;
        break;
    case SR48:
        sample_rate = 48000;
        return sample_rate;
        break;
    case SR88:
        sample_rate = 88200;
        return sample_rate;
        break;
    case SR96:
        sample_rate = 96000;
        return sample_rate;
        break;
    case SR192:
        sample_rate = 192000;
        return sample_rate;
        break;
    default:
        return sample_rate;
        break;
    }

    return sample_rate;
}

//*******************************************************************************
void AudioInterface::setDevicesWarningMsg(warningMessageT msg)
{
    switch (msg) {
    case DEVICE_WARN_LATENCY:
        mWarningMsg =
            "The selected Input and Output devices are Non-ASIO and may cause high "
            "latency or audio delay. Installing ASIO drivers and using ASIO Input and "
            "Output devices will lower audio delays for a 'same room experience'.";
#ifdef _WIN32
        mWarningHelpUrl = "https://help.jacktrip.org/hc/en-us/articles/4409919243155";
#else
        mWarningHelpUrl = "";
#endif
        break;
    default:
        mWarningMsg     = "";
        mWarningHelpUrl = "";
        break;
    }

    return;
}

//*******************************************************************************
void AudioInterface::setDevicesErrorMsg(errorMessageT msg)
{
    mErrorMsg = msg;
    switch (msg) {
    case DEVICE_ERR_INCOMPATIBLE:
        mErrorMsg =
            "The two devices you have selected are not compatible. Please select a "
            "different pair of devices.";
#ifdef _WIN32
        mErrorHelpUrl = "https://help.jacktrip.org/hc/en-us/articles/4409919243155";
#else
        mErrorHelpUrl   = "";
#endif
        break;
    case DEVICE_ERR_NO_INPUTS:
        mErrorMsg     = "JackTrip couldn't find any input devices!";
        mErrorHelpUrl = "";
        break;
    case DEVICE_ERR_NO_OUTPUTS:
        mErrorMsg     = "JackTrip couldn't find any output devices!";
        mErrorHelpUrl = "";
        break;
    case DEVICE_ERR_NO_DEVICES:
        mErrorMsg     = "JackTrip couldn't find any audio devices!";
        mErrorHelpUrl = "";
        break;
#ifdef _WIN32
    case DEVICE_ERR_SAME_ASIO:
        mErrorMsg =
            "When using ASIO, please select the same device for your input and output.";
        mErrorHelpUrl = "https://help.jacktrip.org/hc/en-us/articles/4409919243155";
        break;
#endif
    default:
        mErrorMsg     = "";
        mErrorHelpUrl = "";
        break;
    }
    return;
}

//*******************************************************************************
std::string AudioInterface::getDevicesWarningMsg()
{
    return mWarningMsg;
}

//*******************************************************************************
std::string AudioInterface::getDevicesErrorMsg()
{
    return mErrorMsg;
}

//*******************************************************************************
std::string AudioInterface::getDevicesWarningHelpUrl()
{
    return mWarningHelpUrl;
}

//*******************************************************************************
std::string AudioInterface::getDevicesErrorHelpUrl()
{
    return mErrorHelpUrl;
}
