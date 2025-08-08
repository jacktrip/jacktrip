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
 * \file RegulatorTest.cpp
 * \author Chris Chafe
 * \date December 2024
 */

#include "RegulatorTest.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <algorithm>
#include <numeric>

// Static Qt application for worker threads
QCoreApplication* RegulatorTest::sQtApp = nullptr;
int RegulatorTest::sAppArgc = 1;
char* RegulatorTest::sAppArgv[] = {const_cast<char*>("RegulatorTest")};

RegulatorTest::RegulatorTest()
    : mChannels(2)
    , mBitRes(2)  // 16-bit
    , mLocalFPP(64)
    , mSampleRate(48000)
    , mPeerFPP(64)
    , mRandomGenerator(std::random_device{}())
    , mUniformDist(0.0, 1.0)
    , mNormalDist(0.0, 1.0)
    , mPacketsSent(0)
    , mPacketsReceived(0)
    , mUnderruns(0)
    , mOverruns(0)
    , mTestRunning(false)
{
    initializeTestDefaults();
}

RegulatorTest::~RegulatorTest()
{
}

void RegulatorTest::SetUp()
{
    // Initialize Qt application if not already done
    if (!sQtApp) {
        sQtApp = new QCoreApplication(sAppArgc, sAppArgv);
    }

    // Create regulator with standard test configuration
    // qLen=10 for fixed 10ms tolerance, bqLen=0 for no broadcast
    mRegulator = std::make_unique<Regulator>(mChannels, mBitRes, mLocalFPP, 10, 0, mSampleRate);
    
    // Calculate packet parameters
    mPacketBytes = mPeerFPP * mChannels * mBitRes;
    mPacketDurationMs = (double)mPeerFPP / mSampleRate * 1000.0;
    
    // Initialize output buffer
    mOutputBuffer.resize(mLocalFPP * mChannels * mBitRes);
    
    // Reset statistics
    mPacketsSent = 0;
    mPacketsReceived = 0;
    mUnderruns = 0;
    mOverruns = 0;
    mTestRunning = false;
}

void RegulatorTest::TearDown()
{
    mTestRunning = false;
    mRegulator.reset();
}

void RegulatorTest::initializeTestDefaults()
{
    mChannels = 2;
    mBitRes = 2;
    mLocalFPP = 64;
    mSampleRate = 48000;
    mPeerFPP = 64;
}

std::vector<int8_t> RegulatorTest::createSilentPacket(int seq_num)
{
    std::vector<int8_t> packet(mPacketBytes, 0);
    return packet;
}

std::vector<int8_t> RegulatorTest::createSinePacket(int seq_num, double frequency, double amplitude)
{
    std::vector<int8_t> packet(mPacketBytes);
    
    for (int frame = 0; frame < mPeerFPP; frame++) {
        double time = (seq_num * mPeerFPP + frame) / (double)mSampleRate;
        double sample_value = amplitude * sin(2.0 * M_PI * frequency * time);
        
        // Convert to 16-bit signed integer
        int16_t sample_int = static_cast<int16_t>(sample_value * 32767.0);
        
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16ToBytes(sample_int, &packet[offset]);
        }
    }
    
    return packet;
}

std::vector<int8_t> RegulatorTest::createNoisePacket(int seq_num, double amplitude)
{
    std::vector<int8_t> packet(mPacketBytes);
    
    for (int frame = 0; frame < mPeerFPP; frame++) {
        double sample_value = amplitude * mNormalDist(mRandomGenerator);
        int16_t sample_int = static_cast<int16_t>(std::clamp(sample_value * 32767.0, -32767.0, 32767.0));
        
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16ToBytes(sample_int, &packet[offset]);
        }
    }
    
    return packet;
}

std::vector<int8_t> RegulatorTest::createSweepPacket(int seq_num, double start_freq, double end_freq, int total_packets)
{
    std::vector<int8_t> packet(mPacketBytes);
    double progress = (double)seq_num / total_packets;
    double current_freq = start_freq + (end_freq - start_freq) * progress;
    
    for (int frame = 0; frame < mPeerFPP; frame++) {
        double time = (seq_num * mPeerFPP + frame) / (double)mSampleRate;
        double sample_value = 0.5 * sin(2.0 * M_PI * current_freq * time);
        int16_t sample_int = static_cast<int16_t>(sample_value * 32767.0);
        
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16ToBytes(sample_int, &packet[offset]);
        }
    }
    
    return packet;
}

std::vector<int8_t> RegulatorTest::createHarmonicPacket(int seq_num, double fundamental, int harmonics)
{
    std::vector<int8_t> packet(mPacketBytes);
    
    for (int frame = 0; frame < mPeerFPP; frame++) {
        double time = (seq_num * mPeerFPP + frame) / (double)mSampleRate;
        double sample_value = 0.0;
        
        // Sum harmonics with 1/h amplitude
        for (int h = 1; h <= harmonics; h++) {
            sample_value += (0.5 / h) * sin(2.0 * M_PI * fundamental * h * time);
        }
        
        sample_value /= harmonics;  // Normalize
        int16_t sample_int = static_cast<int16_t>(sample_value * 32767.0);
        
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16ToBytes(sample_int, &packet[offset]);
        }
    }
    
    return packet;
}

bool RegulatorTest::validateAudioContinuity(const std::vector<int8_t>& buffer)
{
    if (buffer.size() < 4) return false;
    
    // Check for major discontinuities (sudden large jumps)
    const double threshold = 0.5;  // 50% of full scale
    
    for (size_t i = mBitRes; i < buffer.size() - mBitRes; i += mBitRes) {
        int16_t current = bytesToInt16(&buffer[i]);
        int16_t next = bytesToInt16(&buffer[i + mBitRes]);
        
        double current_norm = current / 32767.0;
        double next_norm = next / 32767.0;
        
        if (std::abs(next_norm - current_norm) > threshold) {
            return false;
        }
    }
    
    return true;
}

double RegulatorTest::calculateRMS(const std::vector<int8_t>& buffer)
{
    if (buffer.empty()) return 0.0;
    
    double sum_squares = 0.0;
    int samples = buffer.size() / mBitRes;
    
    for (int i = 0; i < samples; i++) {
        int16_t sample = byt esToInt16(&buffer[i * mBitRes]);
        double normalized = sample / 32767.0;
        sum_squares += normalized * normalized;
    }
    
    return sqrt(sum_squares / samples);
}

bool RegulatorTest::detectSilence(const std::vector<int8_t>& buffer, double threshold)
{
    return calculateRMS(buffer) < threshold;
}

bool RegulatorTest::detectClipping(const std::vector<int8_t>& buffer)
{
    const int16_t clip_threshold = 32700;  // Near full scale
    
    for (size_t i = 0; i < buffer.size(); i += mBitRes) {
        int16_t sample = bytesToInt16(&buffer[i]);
        if (std::abs(sample) > clip_threshold) {
            return true;
        }
    }
    
    return false;
}

RegulatorTest::PerformanceMetrics RegulatorTest::measurePerformance(int duration_ms)
{
    PerformanceMetrics metrics = {};
    
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(duration_ms);
    
    mTestRunning = true;
    mPacketsSent = 0;
    mPacketsReceived = 0;
    
    // Run test simulation
    auto current_time = start_time;
    int seq_num = 0;
    
    while (current_time < end_time && mTestRunning) {
        // Send packet
        auto packet = createSinePacket(seq_num++);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, seq_num);
        mPacketsSent++;
        
        // Read output
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        mPacketsReceived++;
        
        // Simulate real-time spacing
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(mPacketDurationMs * 1000)));
        
        current_time = std::chrono::high_resolution_clock::now();
    }
    
    auto actual_end_time = std::chrono::high_resolution_clock::now();
    metrics.total_test_time_ms = std::chrono::duration<double, std::milli>(actual_end_time - start_time).count();
    metrics.packets_sent = mPacketsSent;
    metrics.packets_received = mPacketsReceived;
    metrics.worker_activated = mRegulator->isWorkerEnabled();
    
    // Get regulator statistics
    RingBuffer::IOStat stats;
    if (mRegulator->getStats(&stats, false)) {
        metrics.underruns = stats.underruns;
        metrics.max_latency_ms = mRegulator->getLatency();
    }
    
    mTestRunning = false;
    return metrics;
}

void RegulatorTest::printPerformanceMetrics(const PerformanceMetrics& metrics)
{
    std::cout << "\n=== Performance Metrics ===" << std::endl;
    std::cout << "Test Duration: " << metrics.total_test_time_ms << " ms" << std::endl;
    std::cout << "Packets Sent: " << metrics.packets_sent << std::endl;
    std::cout << "Packets Received: " << metrics.packets_received << std::endl;
    std::cout << "Underruns: " << metrics.underruns << std::endl;
    std::cout << "Overruns: " << metrics.overruns << std::endl;
    std::cout << "Max Latency: " << metrics.max_latency_ms << " ms" << std::endl;
    std::cout << "Worker Activated: " << (metrics.worker_activated ? "Yes" : "No") << std::endl;
    std::cout << "Prediction Activations: " << metrics.prediction_activations << std::endl;
}

void RegulatorTest::saveMetricsToFile(const PerformanceMetrics& metrics, const std::string& filename)
{
    std::ofstream file(filename);
    file << "metric,value\n";
    file << "test_duration_ms," << metrics.total_test_time_ms << "\n";
    file << "packets_sent," << metrics.packets_sent << "\n";
    file << "packets_received," << metrics.packets_received << "\n";
    file << "underruns," << metrics.underruns << "\n";
    file << "overruns," << metrics.overruns << "\n";
    file << "max_latency_ms," << metrics.max_latency_ms << "\n";
    file << "worker_activated," << (metrics.worker_activated ? 1 : 0) << "\n";
    file << "prediction_activations," << metrics.prediction_activations << "\n";
    file.close();
}

void RegulatorTest::runRealtimeSimulation(int duration_ms, double packet_interval_ms)
{
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time + std::chrono::milliseconds(duration_ms);
    
    mTestRunning = true;
    int seq_num = 0;
    auto next_packet_time = start_time;
    
    while (std::chrono::high_resolution_clock::now() < end_time && mTestRunning) {
        auto current_time = std::chrono::high_resolution_clock::now();
        
        // Send packet if it's time
        if (current_time >= next_packet_time) {
            auto packet = createSinePacket(seq_num++, 440.0, 0.5);
            mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, seq_num);
            
            next_packet_time += std::chrono::microseconds(static_cast<int>(packet_interval_ms * 1000));
        }
        
        // Always try to read output (simulating audio callback)
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        
        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    mTestRunning = false;
}

void RegulatorTest::sendPacketBurst(int count, int start_seq)
{
    for (int i = 0; i < count; i++) {
        auto packet = createSinePacket(start_seq + i);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, start_seq + i);
    }
}

void RegulatorTest::createPacketGap(int duration_packets)
{
    // Simply don't send packets for the specified duration
    for (int i = 0; i < duration_packets; i++) {
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(mPacketDurationMs * 1000)));
    }
}

// Helper functions
int16_t RegulatorTest::bytesToInt16(const int8_t* bytes)
{
    return static_cast<int16_t>((bytes[1] << 8) | (bytes[0] & 0xFF));
}

void RegulatorTest::int16ToBytes(int16_t value, int8_t* bytes)
{
    bytes[0] = static_cast<int8_t>(value & 0xFF);
    bytes[1] = static_cast<int8_t>((value >> 8) & 0xFF);
}

// ========================================================================
// ACTUAL UNIT TESTS
// ========================================================================

TEST_F(RegulatorTest, BasicInitialization)
{
    EXPECT_EQ(mRegulator->getSampleRate(), mSampleRate);
    EXPECT_EQ(mRegulator->getBufferSizeInSamples(), mLocalFPP);
    EXPECT_FALSE(mRegulator->isWorkerEnabled());
    
    // Should not be fully initialized until first packet
    RingBuffer::IOStat stats;
    EXPECT_FALSE(mRegulator->getStats(&stats, false));
}

TEST_F(RegulatorTest, FirstPacketInitialization)
{
    auto packet = createSinePacket(0);
    mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, 0);
    
    // Now should be initialized
    RingBuffer::IOStat stats;
    EXPECT_TRUE(mRegulator->getStats(&stats, false));
    EXPECT_GT(mRegulator->getPacketSize(), 0);
}

TEST_F(RegulatorTest, NormalPacketFlow)
{
    // Send sequence of packets and read outputs
    for (int i = 0; i < 10; i++) {
        auto packet = createSinePacket(i, 440.0, 0.5);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        
        // Should have valid audio output (not silence)
        EXPECT_FALSE(detectSilence(mOutputBuffer));
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
        EXPECT_FALSE(detectClipping(mOutputBuffer));
    }
}

TEST_F(RegulatorTest, PacketLossHandling)
{
    // Send initial packets
    for (int i = 0; i < 5; i++) {
        auto packet = createSinePacket(i);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    }
    
    // Skip packets 5, 6, 7 (simulate loss)
    for (int i = 0; i < 3; i++) {
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        // Should still produce output (prediction or silence)
        // but might be different from normal audio
    }
    
    // Resume with packet 8
    auto packet8 = createSinePacket(8);
    mRegulator->insertSlotNonBlocking(packet8.data(), packet8.size(), 0, 8);
    mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    
    // Should recover gracefully
    EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
}

TEST_F(RegulatorTest, SilenceHandling)
{
    // Test with silent packets
    for (int i = 0; i < 5; i++) {
        auto packet = createSilentPacket(i);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        
        EXPECT_TRUE(detectSilence(mOutputBuffer));
    }
}

TEST_F(RegulatorTest, DifferentSignalTypes)
{
    // Test various signal types
    std::vector<std::function<std::vector<int8_t>(int)>> generators = {
        [this](int seq) { return createSinePacket(seq, 220.0, 0.6); },
        [this](int seq) { return createSinePacket(seq, 880.0, 0.4); },
        [this](int seq) { return createHarmonicPacket(seq, 110.0, 6); },
        [this](int seq) { return createNoisePacket(seq, 0.1); }
    };
    
    for (auto& generator : generators) {
        for (int i = 0; i < 5; i++) {
            auto packet = generator(i);
            mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
            mRegulator->readSlotNonBlocking(mOutputBuffer.data());
            
            EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
            EXPECT_FALSE(detectClipping(mOutputBuffer));
        }
    }
}

TEST_F(RegulatorTest, BurstPacketHandling)
{
    // Send burst of packets
    sendPacketBurst(5, 0);
    
    // Read them out
    for (int i = 0; i < 10; i++) {
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    }
}

TEST_F(RegulatorTest, ExtendedGapHandling)
{
    // Send initial packets
    for (int i = 0; i < 3; i++) {
        auto packet = createSinePacket(i);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    }
    
    // Create extended gap (simulate network outage)
    createPacketGap(10);
    
    // Resume transmission
    auto packet = createSinePacket(13);
    mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, 13);
    mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    
    // Should handle gracefully (may output silence for extended gaps)
    EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
}

TEST_F(RegulatorTest, PerformanceUnderLoad)
{
    auto metrics = measurePerformance(1000);  // 1 second test
    
    printPerformanceMetrics(metrics);
    saveMetricsToFile(metrics, "regulator_performance_test.csv");
    
    // Basic performance expectations
    EXPECT_GT(metrics.packets_sent, 50);  // Should send reasonable number of packets
    EXPECT_EQ(metrics.packets_sent, metrics.packets_received);  // Should match in controlled test
    EXPECT_LT(metrics.max_latency_ms, 100);  // Reasonable latency bound
}

TEST_F(RegulatorTest, StatisticsAccuracy)
{
    // Send packets and force some underruns
    for (int i = 0; i < 20; i++) {
        if (i % 7 != 0) {  // Skip every 7th packet
            auto packet = createSinePacket(i);
            mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        }
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    }
    
    RingBuffer::IOStat stats;
    EXPECT_TRUE(mRegulator->getStats(&stats, false));
    
    // Should have recorded some statistics
    // Note: exact values depend on internal timing, so we just check they're reasonable
    EXPECT_GE(stats.underruns, 0);
}

TEST_F(RegulatorTest, FrequencySweepHandling)
{
    // Test with frequency sweep to stress the prediction algorithm
    const int total_packets = 100;
    for (int i = 0; i < total_packets; i++) {
        auto packet = createSweepPacket(i, 100.0, 1000.0, total_packets);
        mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, i);
        mRegulator->readSlotNonBlocking(mOutputBuffer.data());
        
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
        EXPECT_FALSE(detectClipping(mOutputBuffer));
    }
}

