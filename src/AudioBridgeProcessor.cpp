//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024-2025 JackTrip Labs, Inc.

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

#include "AudioBridgeProcessor.h"

#include <QDebug>
#include <algorithm>
#include <cmath>

using namespace std;

// uncomment to generate log file, for debugging purposes
// #define AUDIO_BRIDGE_PLUGIN_LOG

#ifdef AUDIO_BRIDGE_PLUGIN_LOG
#if defined(_WIN32)
#define AUDIO_BRIDGE_PLUGIN_LOG_PATH "c:/JackTripTemp"
#define AUDIO_BRIDGE_PLUGIN_LOG_FILE "c:/JackTripTemp/plugin.log"
#else
#define AUDIO_BRIDGE_PLUGIN_LOG_PATH "/tmp/jacktrip"
#define AUDIO_BRIDGE_PLUGIN_LOG_FILE "/tmp/jacktrip/plugin.log"
#endif
#include <filesystem>
#include <fstream>
#include <iostream>

static ofstream kLogFile;

void qtMessageHandler([[maybe_unused]] QtMsgType type,
                      [[maybe_unused]] const QMessageLogContext& context,
                      const QString& msg)
{
    kLogFile << msg.toStdString() << endl;
}
#endif

// any multiplier less than this is considered to be silent
constexpr double kSilentMul = 0.0000001;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// AudioBridgeProcessor
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

AudioBridgeProcessor::AudioBridgeProcessor() {}

AudioBridgeProcessor::~AudioBridgeProcessor()
{
    uninitialize();
}

void AudioBridgeProcessor::initialize(unsigned int sampleRate, unsigned int bufferSize)
{
    // Allocate audio buffers
    if (!mBuffersInitialized) {
        mInputBuffer  = new float*[AudioSocketNumChannels];
        mOutputBuffer = new float*[AudioSocketNumChannels];
        for (int i = 0; i < AudioSocketNumChannels; i++) {
            mInputBuffer[i]  = new float[AudioSocketMaxSamplesPerBlock];
            mOutputBuffer[i] = new float[AudioSocketMaxSamplesPerBlock];
        }
        mBuffersInitialized = true;
    }

#ifdef AUDIO_BRIDGE_PLUGIN_LOG
    // Setup plugin logging
    if (!filesystem::is_directory(AUDIO_BRIDGE_PLUGIN_LOG_PATH)) {
        if (!filesystem::create_directory(AUDIO_BRIDGE_PLUGIN_LOG_PATH)) {
            qDebug() << "Failed to create AU log directory: "
                     << AUDIO_BRIDGE_PLUGIN_LOG_PATH;
        }
    }
    kLogFile.open(AUDIO_BRIDGE_PLUGIN_LOG_FILE, ios::app);
    if (kLogFile.is_open()) {
        kLogFile << "JackTrip audio bridge plugin initialized" << endl;
        kLogFile.flush();
        cout.rdbuf(kLogFile.rdbuf());
        cerr.rdbuf(kLogFile.rdbuf());
    } else {
        qDebug() << "Failed to open plugin log file: " << AUDIO_BRIDGE_PLUGIN_LOG_FILE;
    }
    qInstallMessageHandler(qtMessageHandler);
#endif

    // Initialize AudioSocket
    mSocketPtr = make_unique<AudioSocket>();
    mSocketPtr->setRetryConnection(true);
    mSocketPtr->connect(sampleRate, bufferSize);
}

void AudioBridgeProcessor::uninitialize(void)
{
    // Clean up AudioSocket
    mSocketPtr.reset();

    // Clean up audio buffers
    if (mInputBuffer) {
        for (int i = 0; i < AudioSocketNumChannels; i++) {
            delete[] mInputBuffer[i];
        }
        delete[] mInputBuffer;
        mInputBuffer = nullptr;
    }

    if (mOutputBuffer) {
        for (int i = 0; i < AudioSocketNumChannels; i++) {
            delete[] mOutputBuffer[i];
        }
        delete[] mOutputBuffer;
        mOutputBuffer = nullptr;
    }

    mBuffersInitialized = false;

#ifdef AUDIO_BRIDGE_PLUGIN_LOG
    if (kLogFile.is_open()) {
        kLogFile.close();
    }
#endif
}

void AudioBridgeProcessor::process(float** inputBuffers, float** outputBuffers,
                                   bool* inputSilenceFlags, bool* outputSilenceFlags,
                                   unsigned int bufSize)
{
    // Check if buffers are initialized
    if (!mBuffersInitialized || !mInputBuffer || !mOutputBuffer) {
        // sanity check for OOB memory access
        if (outputBuffers == nullptr) {
            return;
        }
        // internal buffers not initialized - silence output
        for (unsigned int ch = 0; ch < AudioSocketNumChannels; ch++) {
            if (outputBuffers[ch] != nullptr) {
                memset(outputBuffers[ch], 0, BytesPerSample * bufSize);
            }
            if (outputSilenceFlags) {
                outputSilenceFlags[ch] = true;
            }
        }
        return;
    }

    // Limit frames to our max buffer size (sanity check for OOB memory access)
    const unsigned int framesToProcess =
        min(bufSize, static_cast<unsigned int>(AudioSocketMaxSamplesPerBlock));

    // Clear our internal buffers
    for (int ch = 0; ch < AudioSocketNumChannels; ch++) {
        memset(mInputBuffer[ch], 0, framesToProcess * BytesPerSample);
        memset(mOutputBuffer[ch], 0, framesToProcess * BytesPerSample);
        if (outputSilenceFlags) {
            outputSilenceFlags[ch] = true;
        }
    }

    // Copy input to our buffer, applying send gain
    if (inputBuffers) {
        for (int ch = 0; ch < AudioSocketNumChannels; ch++) {
            float* inData = inputBuffers[ch];
            if (inData != nullptr
                && (inputSilenceFlags == nullptr || !inputSilenceFlags[ch])) {
                for (unsigned int i = 0; i < framesToProcess; i++) {
                    mInputBuffer[ch][i] = inData[i] * mSendMul;
                }
            }
        }
    }

    // Process through AudioSocket
    if (mSocketPtr) {
        mSocketPtr->compute(framesToProcess, mInputBuffer, mOutputBuffer);
    }

    // sanity check for OOB memory access
    if (outputBuffers == nullptr) {
        return;
    }

    // Mix output from AudioSocket with passthrough, applying appropriate gains
    for (int ch = 0; ch < AudioSocketNumChannels; ch++) {
        float* inData  = inputBuffers ? inputBuffers[ch] : nullptr;
        float* outData = outputBuffers[ch];
        bool silent    = true;

        if (outData == nullptr)
            continue;

        for (unsigned int i = 0; i < framesToProcess; i++) {
            float output = 0.0f;

            // Add received audio from JackTrip
            if (mRecvMul > kSilentMul) {
                output += mOutputBuffer[ch][i] * mRecvMul;
            }

            // Add passthrough audio
            if (mPassMul > kSilentMul && inData
                && (inputSilenceFlags == nullptr || !inputSilenceFlags[ch])) {
                output += inData[i] * mPassMul;
            }

            outData[i] = output;
            if (output > 0) {
                silent = false;
            }
        }

        if (outputSilenceFlags) {
            outputSilenceFlags[ch] = silent;
        }
    }
}

float AudioBridgeProcessor::gainToVol(double gain)
{
    // handle min and max
    if (gain < kSilentMul)
        return 0;
    if (gain > 0.9999999)
        return 1.0;
    // simple logarithmic conversion
    return exp(log(1000) * gain) / 1000.0;
}
