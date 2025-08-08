//*****************************************************************
/*
  JackTrip: A System for High-Quality Audio Network Performance
  over the Internet

  Copyright (c) 2024 Juan-Pablo Caceres, Chris Chafe.
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
 * \file RegulatorTest.h
 * \author Chris Chafe
 * \date December 2024
 */

#ifndef __REGULATOR_TEST_H__
#define __REGULATOR_TEST_H__

#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <QThread>
#include <QCoreApplication>

#include "Regulator.h"
#include "RingBuffer.h"

class RegulatorTest : public ::testing::Test
{
public:
    RegulatorTest();
    virtual ~RegulatorTest();

    // Test fixture setup and teardown
    void SetUp() override;
    void TearDown() override;

    // Test data generation methods
    std::vector<int8_t> createSilentPacket(int seq_num = 0);
    std::vector<int8_t> createSinePacket(int seq_num, double frequency = 440.0, double amplitude = 0.5);
    std::vector<int8_t> createNoisePacket(int seq_num, double amplitude = 0.1);
    std::vector<int8_t> createSweepPacket(int seq_num, double start_freq, double end_freq, int total_packets);
    std::vector<int8_t> createHarmonicPacket(int seq_num, double fundamental = 220.0, int harmonics = 4);

    // Network simulation methods
    void simulatePacketLoss(double loss_rate);
    void simulateJitter(double jitter_ms);
    void simulateLatency(double latency_ms);
    void simulateNetworkConditions(double loss_rate, double jitter_ms, double latency_ms);

    // Test validation methods
    bool validateAudioContinuity(const std::vector<int8_t>& buffer);
    double calculateRMS(const std::vector<int8_t>& buffer);
    double calculateSNR(const std::vector<int8_t>& signal, const std::vector<int8_t>& noise);
    bool detectSilence(const std::vector<int8_t>& buffer, double threshold = 0.001);
    bool detectClipping(const std::vector<int8_t>& buffer);

    // Performance measurement methods
    struct PerformanceMetrics {
        double total_test_time_ms;
        int packets_sent;
        int packets_received;
        int underruns;
        int overruns;
        double max_latency_ms;
        double avg_latency_ms;
        double cpu_usage_percent;
        bool worker_activated;
        int prediction_activations;
    };

    PerformanceMetrics measurePerformance(int duration_ms);
    void printPerformanceMetrics(const PerformanceMetrics& metrics);
    void saveMetricsToFile(const PerformanceMetrics& metrics, const std::string& filename);

    // Real-time simulation methods
    void runRealtimeSimulation(int duration_ms, double packet_interval_ms = 13.3);
    void sendPacketBurst(int count, int start_seq);
    void createPacketGap(int duration_packets);

protected:
    // Test configuration
    int mChannels;
    int mBitRes;
    int mLocalFPP;
    int mSampleRate;
    int mPeerFPP;
    int mPacketBytes;
    double mPacketDurationMs;

    // Test objects
    std::unique_ptr<Regulator> mRegulator;
    std::vector<int8_t> mOutputBuffer;
    
    // Network simulation state
    std::mt19937 mRandomGenerator;
    std::uniform_real_distribution<double> mUniformDist;
    std::normal_distribution<double> mNormalDist;
    
    // Timing and synchronization
    std::chrono::high_resolution_clock::time_point mTestStartTime;
    std::vector<std::chrono::high_resolution_clock::time_point> mPacketTimes;
    
    // Test statistics
    int mPacketsSent;
    int mPacketsReceived;
    int mUnderruns;
    int mOverruns;
    bool mTestRunning;

    void printDetailedStatistics(const std::string& testName);

    // Realistic timing methods
    void realtimeDelay(double targetMs);
    void simulateAudioCallback();
    void simulatePacketArrival(int seq_num, const std::vector<int8_t>& packet);
    void runRealtimeTest(int durationMs, std::function<std::vector<int8_t>(int)> packetGenerator, double jitterPercent = 0.0);
    double mAudioCallbackIntervalMs;  // Audio callback timing (should be 1.33ms)

    // Add these declarations to the protected section of RegulatorTest.h:

    // WAV recording methods
    void enableWavRecording(bool enable = true);
    void recordInputPacket(const std::vector<int8_t>& packet);
    void recordOutputBuffer();
    void recordSilentInput();
    void writeWavFile(const std::string& filename);

    // Packet loss simulation
    void simulatePacketLoss();

    // Add these to the private member variables:
   private:

private:
    // WAV recording
    bool mRecordingEnabled;
    std::vector<double> mInputSamples;
    std::vector<double> mOutputSamples;

    // Internal helper methods
    void initializeTestDefaults();
    double sampleToFloat(int8_t* sample_ptr, int bit_res);
    void floatToSample(double value, int8_t* sample_ptr, int bit_res);
    int16_t bytesToInt16(const int8_t* bytes);
    void int16ToBytes(int16_t value, int8_t* bytes);
    
    // Qt application for worker thread support
    static QCoreApplication* sQtApp;
    static int sAppArgc;
    static char* sAppArgv[];
};

#endif  // __REGULATOR_TEST_H__

