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
#include <fstream>
#include <iomanip>

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
    , mRecordingEnabled(false)
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
            // qLen=1 for 1ms tolerance, bqLen=0 for no broadcast
    mRegulator = std::make_unique<Regulator>(mChannels, mBitRes, mLocalFPP,
                                             1, 0, mSampleRate);

            // Calculate packet parameters
    mPacketBytes = mPeerFPP * mChannels * mBitRes;
    mPacketDurationMs = (double)mPeerFPP / mSampleRate * 1000.0;  // Should be 1.33ms
    mAudioCallbackIntervalMs = mPacketDurationMs;  // 1.33ms for 64 samples at 48kHz

            // Initialize output buffer
    mOutputBuffer.resize(mLocalFPP * mChannels * mBitRes);

            // Reset statistics
    mPacketsSent = 0;
    mPacketsReceived = 0;
    mUnderruns = 0;
    mOverruns = 0;
    mTestRunning = false;
    mRecordingEnabled = false;

            // Clear recording buffers
    mInputSamples.clear();
    mOutputSamples.clear();

    std::cout << "Test setup - Packet duration: " << mPacketDurationMs << "ms, Audio callback: " << mAudioCallbackIntervalMs << "ms" << std::endl;
}

void RegulatorTest::TearDown()
{
    mTestRunning = false;
    mRecordingEnabled = false;
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

void RegulatorTest::enableWavRecording(bool enable)
{
    mRecordingEnabled = enable;
    if (enable) {
        mInputSamples.clear();
        mOutputSamples.clear();
        std::cout << "WAV recording enabled" << std::endl;
    }
}

void RegulatorTest::recordInputPacket(const std::vector<int8_t>& packet)
{
    if (!mRecordingEnabled) return;

            // Convert packet to mono float samples (mix channels if stereo)
    for (int frame = 0; frame < mPeerFPP; frame++) {
        double sample_sum = 0.0;
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16_t sample = bytesToInt16(&packet[offset]);
            sample_sum += sample / 32767.0;
        }
        mInputSamples.push_back(sample_sum / mChannels);  // Average channels
    }
}

void RegulatorTest::recordOutputBuffer()
{
    if (!mRecordingEnabled) return;

            // Convert output buffer to mono float samples (mix channels if stereo)
    for (int frame = 0; frame < mLocalFPP; frame++) {
        double sample_sum = 0.0;
        for (int ch = 0; ch < mChannels; ch++) {
            int offset = (frame * mChannels + ch) * mBitRes;
            int16_t sample = bytesToInt16(&mOutputBuffer[offset]);
            sample_sum += sample / 32767.0;
        }
        mOutputSamples.push_back(sample_sum / mChannels);  // Average channels
    }
}

void RegulatorTest::recordSilentInput()
{
    if (!mRecordingEnabled) return;

            // Record silence for missed input packets
    for (int frame = 0; frame < mPeerFPP; frame++) {
        mInputSamples.push_back(0.0);
    }
}

void RegulatorTest::writeWavFile(const std::string& filename)
{
    if (!mRecordingEnabled || mInputSamples.empty() || mOutputSamples.empty()) {
        std::cout << "No recorded data to write" << std::endl;
        return;
    }

            // Make both channels the same length (pad with silence if needed)
    size_t maxLength = std::max(mInputSamples.size(), mOutputSamples.size());

    while (mInputSamples.size() < maxLength) {
        mInputSamples.push_back(0.0);
    }
    while (mOutputSamples.size() < maxLength) {
        mOutputSamples.push_back(0.0);
    }

            // Write WAV file
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to open " << filename << " for writing" << std::endl;
        return;
    }

            // WAV header
    uint32_t sampleRate = mSampleRate;
    uint16_t numChannels = 2;  // Stereo: left=input, right=output
    uint16_t bitsPerSample = 16;
    uint32_t numSamples = static_cast<uint32_t>(maxLength);
    uint32_t dataSize = numSamples * numChannels * (bitsPerSample / 8);
    uint32_t fileSize = dataSize + 36;

            // RIFF header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);

            // Format chunk
    file.write("fmt ", 4);
    uint32_t fmtSize = 16;
    file.write(reinterpret_cast<const char*>(&fmtSize), 4);
    uint16_t audioFormat = 1;  // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

            // Data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

            // Interleaved audio data
    for (size_t i = 0; i < maxLength; i++) {
        // Left channel: input packets
        int16_t leftSample = static_cast<int16_t>(std::clamp(mInputSamples[i] * 32767.0, -32767.0, 32767.0));
        file.write(reinterpret_cast<const char*>(&leftSample), 2);

                // Right channel: output to audio
        int16_t rightSample = static_cast<int16_t>(std::clamp(mOutputSamples[i] * 32767.0, -32767.0, 32767.0));
        file.write(reinterpret_cast<const char*>(&rightSample), 2);
    }

    file.close();

    double durationSec = static_cast<double>(maxLength) / mSampleRate;
    std::cout << "Wrote WAV file: " << filename << std::endl;
    std::cout << "  Duration: " << std::fixed << std::setprecision(2) << durationSec << " seconds" << std::endl;
    std::cout << "  Samples: " << maxLength << " per channel" << std::endl;
    std::cout << "  Input samples recorded: " << mInputSamples.size() << std::endl;
    std::cout << "  Output samples recorded: " << mOutputSamples.size() << std::endl;
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

void RegulatorTest::realtimeDelay(double targetMs)
{
    auto delayMicros = static_cast<int>(targetMs * 1000.0);
    std::this_thread::sleep_for(std::chrono::microseconds(delayMicros));
}

void RegulatorTest::simulateAudioCallback()
{
    mRegulator->readSlotNonBlocking(mOutputBuffer.data());
    mPacketsReceived++;

            // Record output for WAV file
    recordOutputBuffer();
}

void RegulatorTest::simulatePacketArrival(int seq_num, const std::vector<int8_t>& packet)
{
    mRegulator->insertSlotNonBlocking(packet.data(), packet.size(), 0, seq_num);
    mPacketsSent++;

            // Record input for WAV file
    recordInputPacket(packet);
}

void RegulatorTest::simulatePacketLoss()
{
    // Simulate missing packet by not calling insertSlotNonBlocking
    // but still record silence in the input channel
    recordSilentInput();
}

void RegulatorTest::runRealtimeTest(int durationMs, std::function<std::vector<int8_t>(int)> packetGenerator, double jitterPercent)
{
    std::cout << "Starting realtime test - Duration: " << durationMs << "ms, Jitter: " << jitterPercent*100 << "%" << std::endl;

    auto testStart = std::chrono::high_resolution_clock::now();
    auto testEnd = testStart + std::chrono::milliseconds(durationMs);

    int seq_num = 0;
    auto nextPacketTime = testStart;
    auto nextCallbackTime = testStart;

    mTestRunning = true;

    while (std::chrono::high_resolution_clock::now() < testEnd && mTestRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();

                // Handle packet arrivals
        if (currentTime >= nextPacketTime) {
            auto packet = packetGenerator(seq_num);
            simulatePacketArrival(seq_num, packet);
            seq_num++;

                    // Calculate next packet time with optional jitter
            double baseInterval = mPacketDurationMs;
            if (jitterPercent > 0) {
                double jitter = (mUniformDist(mRandomGenerator) - 0.5) * 2.0 * jitterPercent * baseInterval;
                baseInterval += jitter;
            }

            nextPacketTime += std::chrono::microseconds(static_cast<int>(baseInterval * 1000));
        }

                // Handle audio callbacks
        if (currentTime >= nextCallbackTime) {
            simulateAudioCallback();
            nextCallbackTime += std::chrono::microseconds(static_cast<int>(mAudioCallbackIntervalMs * 1000));
        }

                // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    mTestRunning = false;

    auto actualDuration = std::chrono::duration<double, std::milli>(
                              std::chrono::high_resolution_clock::now() - testStart).count();

    std::cout << "Realtime test completed - Actual duration: " << actualDuration << "ms" << std::endl;
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
        int16_t sample = bytesToInt16(&buffer[i * mBitRes]);
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

void RegulatorTest::printDetailedStatistics(const std::string& testName)
{
    std::cout << "\n=== " << testName << " Detailed Statistics ===" << std::endl;

            // Regulator statistics
    RingBuffer::IOStat stats;
    if (mRegulator->getStats(&stats, false)) {
        std::cout << "Regulator Internal Statistics:" << std::endl;
        std::cout << "  Underruns: " << stats.underruns << std::endl;
        std::cout << "  Overflows: " << stats.overflows << std::endl;
        std::cout << "  Current Skew: " << stats.skew / 1000.0 << " ms" << std::endl;
        std::cout << "  Raw Skew: " << stats.skew_raw / 1000.0 << " ms" << std::endl;
        std::cout << "  Current Level: " << stats.level / 1000.0 << " ms" << std::endl;
        std::cout << "  Tolerance (Auto Queue): " << stats.autoq_corr / 1000.0 << " ms" << std::endl;
        std::cout << "  Push Stats StdDev: " << stats.buf_dec_overflows / 1000.0 << std::endl;
        std::cout << "  Push Stats Mean: " << stats.buf_inc_underrun / 1000.0 << " ms" << std::endl;
        std::cout << "  Push Stats Min: " << stats.buf_inc_compensate / 1000.0 << " ms" << std::endl;
        std::cout << "  Push Stats Max: " << stats.broadcast_skew / 1000.0 << " ms" << std::endl;
        std::cout << "  Pull Stats StdDev: " << stats.broadcast_delta / 1000.0 << std::endl;
        std::cout << "  Max PLC DSP Time: " << stats.autoq_rate / 1000.0 << " ms" << std::endl;
    }

            // System configuration
    std::cout << "\nSystem Configuration:" << std::endl;
    std::cout << "  Sample Rate: " << mSampleRate << " Hz" << std::endl;
    std::cout << "  Channels: " << mChannels << std::endl;
    std::cout << "  Bit Resolution: " << mBitRes * 8 << " bits" << std::endl;
    std::cout << "  Local FPP: " << mLocalFPP << " samples" << std::endl;
    std::cout << "  Peer FPP: " << mPeerFPP << " samples" << std::endl;
    std::cout << "  Packet Duration: " << mPacketDurationMs << " ms" << std::endl;
    std::cout << "  Audio Callback Interval: " << mAudioCallbackIntervalMs << " ms" << std::endl;
    std::cout << "  Packet Size: " << mPacketBytes << " bytes" << std::endl;

            // Runtime information
    std::cout << "\nRuntime Information:" << std::endl;
    std::cout << "  Worker Thread Enabled: " << (mRegulator->isWorkerEnabled() ? "Yes" : "No") << std::endl;
    std::cout << "  Current Max Latency: " << mRegulator->getLatency() << " ms" << std::endl;

            // Test progress
    std::cout << "\nTest Progress:" << std::endl;
    std::cout << "  Total Packets Sent: " << mPacketsSent << std::endl;
    std::cout << "  Total Packets Received: " << mPacketsReceived << std::endl;
    if (mPacketsReceived > 0) {
        std::cout << "  Packet/Callback Ratio: " << (double)mPacketsSent / mPacketsReceived << std::endl;
    }

            // WAV recording info
    if (mRecordingEnabled) {
        std::cout << "\nWAV Recording:" << std::endl;
        std::cout << "  Input samples recorded: " << mInputSamples.size() << std::endl;
        std::cout << "  Output samples recorded: " << mOutputSamples.size() << std::endl;
    }

    std::cout << "===================================================\n" << std::endl;
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
// ACTUAL UNIT TESTS WITH REALISTIC TIMING AND WAV OUTPUT
// ========================================================================

TEST_F(RegulatorTest, BasicInitialization)
{
    std::cout << "\n=== BasicInitialization Test ===" << std::endl;

    EXPECT_EQ(mRegulator->getSampleRate(), mSampleRate);
    EXPECT_EQ(mRegulator->getBufferSizeInSamples(), mLocalFPP);
    EXPECT_FALSE(mRegulator->isWorkerEnabled());

            // Should not be fully initialized until first packet
    RingBuffer::IOStat stats;
    EXPECT_FALSE(mRegulator->getStats(&stats, false));

    std::cout << "Basic initialization test passed" << std::endl;
}

TEST_F(RegulatorTest, FirstPacketInitialization)
{
    std::cout << "\n=== FirstPacketInitialization Test ===" << std::endl;

    auto packet = createSinePacket(0);
    simulatePacketArrival(0, packet);

            // Wait for initialization to settle
    realtimeDelay(mPacketDurationMs);

            // Now should be initialized
    RingBuffer::IOStat stats;
    EXPECT_TRUE(mRegulator->getStats(&stats, false));
    EXPECT_GT(mRegulator->getPacketSize(), 0);

    printDetailedStatistics("RegulatorTest.FirstPacketInitialization");
}

TEST_F(RegulatorTest, NormalPacketFlow)
{
    std::cout << "\n=== NormalPacketFlow Test ===" << std::endl;

            // Use realtime test framework
    auto sineGenerator = [this](int seq) { return createSinePacket(seq, 440.0, 0.5); };
    runRealtimeTest(100, sineGenerator, 0.0);  // 100ms test, no jitter

            // Validate final output
    EXPECT_FALSE(detectSilence(mOutputBuffer));
    EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    EXPECT_FALSE(detectClipping(mOutputBuffer));

            // Should have reasonable packet/callback ratio
    if (mPacketsReceived > 0) {
        double ratio = (double)mPacketsSent / mPacketsReceived;
        EXPECT_NEAR(ratio, 1.0, 0.2);  // Within 20% of 1:1 ratio
    }

    printDetailedStatistics("RegulatorTest.NormalPacketFlow");
}

TEST_F(RegulatorTest, PacketLossConcealment)
{
    std::cout << "\n=== PacketLossConcealment Test with WAV Output ===" << std::endl;

            // Enable WAV recording for this test
    enableWavRecording(true);

            // Start with normal flow to initialize and prime the system
    std::cout << "Initializing with normal packets..." << std::endl;
    for (int i = 0; i < 10; i++) {
        auto packet = createSinePacket(i, 440.0, 0.6);  // 440Hz sine
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    }

    std::cout << "Initial packets sent successfully" << std::endl;

            // Simulate packet loss - skip packets 10, 11, 12 but keep audio callbacks
    std::cout << "Simulating packet loss (packets 10, 11, 12)..." << std::endl;
    for (int i = 0; i < 3; i++) {
        simulatePacketLoss();  // Record silence in input
        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();

                // During packet loss, might get prediction or silence
        bool silent = detectSilence(mOutputBuffer);
        double rms = calculateRMS(mOutputBuffer);
        std::cout << "  Loss period " << i << " - Silent: " << (silent ? "Yes" : "No")
                  << ", RMS: " << rms << std::endl;
    }

            // Resume with normal packets
    std::cout << "Resuming normal transmission..." << std::endl;
    for (int i = 13; i < 30; i++) {
        auto packet = createSinePacket(i, 440.0, 0.6);
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    }

            // Write WAV file for analysis
    writeWavFile("packet_loss_concealment_test.wav");

    printDetailedStatistics("RegulatorTest.PacketLossConcealment");
}

TEST_F(RegulatorTest, SilenceHandling)
{
    std::cout << "\n=== SilenceHandling Test ===" << std::endl;

            // Test with silent packets using realistic timing
    for (int i = 0; i < 8; i++) {
        auto packet = createSilentPacket(i);
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();

        EXPECT_TRUE(detectSilence(mOutputBuffer));
    }

    printDetailedStatistics("RegulatorTest.SilenceHandling");
}

TEST_F(RegulatorTest, HarmonicContentPLC)
{
    std::cout << "\n=== HarmonicContentPLC Test with WAV Output ===" << std::endl;

            // Enable WAV recording
    enableWavRecording(true);

            // Test PLC with harmonic content (should predict better)
    std::cout << "Sending harmonic content..." << std::endl;
    for (int i = 0; i < 12; i++) {
        auto packet = createHarmonicPacket(i, 220.0, 6);  // Rich harmonic content
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
    }

            // Create packet loss
    std::cout << "Creating packet loss..." << std::endl;
    for (int i = 0; i < 4; i++) {
        simulatePacketLoss();
        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();

        double rms = calculateRMS(mOutputBuffer);
        std::cout << "  PLC period " << i << " - RMS: " << rms << std::endl;
    }

            // Resume
    std::cout << "Resuming harmonic content..." << std::endl;
    for (int i = 16; i < 25; i++) {
        auto packet = createHarmonicPacket(i, 220.0, 6);
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
    }

    writeWavFile("harmonic_plc_test.wav");
    printDetailedStatistics("RegulatorTest.HarmonicContentPLC");
}

TEST_F(RegulatorTest, JitterTolerance)
{
    std::cout << "\n=== JitterTolerance Test ===" << std::endl;

            // Test with 20% jitter auto sineGenerator = [this](int seq) { return createSinePacket(seq, 880.0, 0.4); };
    auto sineGenerator = [this](int seq) { return createSinePacket(seq, 440.0, 0.5); };
    runRealtimeTest(150, sineGenerator, 0.2);  // 150ms test, 20% jitter

    EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    EXPECT_FALSE(detectClipping(mOutputBuffer));

    std::cout << "Jitter test completed with " << mPacketsSent << " packets sent, "
              << mPacketsReceived << " callbacks processed" << std::endl;

    printDetailedStatistics("RegulatorTest.JitterTolerance");
}

TEST_F(RegulatorTest, BroadbandNoisePLC)
{
    std::cout << "\n=== BroadbandNoisePLC Test with WAV Output ===" << std::endl;

            // Enable WAV recording
    enableWavRecording(true);

            // Test PLC with broadband noise (should be challenging to predict)
    std::cout << "Sending broadband noise..." << std::endl;
    for (int i = 0; i < 10; i++) {
        auto packet = createNoisePacket(i, 0.3);
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
    }

            // Create packet loss
    std::cout << "Creating packet loss..." << std::endl;
    for (int i = 0; i < 3; i++) {
        simulatePacketLoss();
        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();

        double rms = calculateRMS(mOutputBuffer);
        std::cout << "  Noise PLC period " << i << " - RMS: " << rms << std::endl;
    }

            // Resume
    std::cout << "Resuming noise..." << std::endl;
    for (int i = 13; i < 20; i++) {
        auto packet = createNoisePacket(i, 0.3);
        simulatePacketArrival(i, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
    }

    writeWavFile("broadband_noise_plc_test.wav");
    printDetailedStatistics("RegulatorTest.BroadbandNoisePLC");
}

TEST_F(RegulatorTest, ExtendedOperation)
{
    std::cout << "\n=== ExtendedOperation Test ===" << std::endl;

            // Longer test to potentially trigger worker thread and adaptive behavior
    auto harmonicGenerator = [this](int seq) { return createHarmonicPacket(seq, 220.0, 4); };
    runRealtimeTest(500, harmonicGenerator, 0.1);  // 500ms test, 10% jitter

    EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));

    std::cout << "Extended operation completed - Worker enabled: "
              << (mRegulator->isWorkerEnabled() ? "Yes" : "No") << std::endl;

    printDetailedStatistics("RegulatorTest.ExtendedOperation");
}

TEST_F(RegulatorTest, BurstAndGapPattern)
{
    std::cout << "\n=== BurstAndGapPattern Test ===" << std::endl;

    int seq_num = 0;

            // Send burst of 5 packets quickly
    std::cout << "Sending packet burst..." << std::endl;
    for (int i = 0; i < 5; i++) {
        auto packet = createSinePacket(seq_num++);
        simulatePacketArrival(seq_num - 1, packet);
        realtimeDelay(0.2);  // Very short delay - burst
    }

            // Continue with audio callbacks during burst processing
    for (int i = 0; i < 8; i++) {
        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    }

            // Create gap (no packets for several callback periods)
    std::cout << "Creating packet gap..." << std::endl;
    for (int i = 0; i < 5; i++) {
        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();
        // May get silence or prediction during gap
    }

            // Resume normal flow
    std::cout << "Resuming normal flow..." << std::endl;
    for (int i = 0; i < 5; i++) {
        auto packet = createSinePacket(seq_num++);
        simulatePacketArrival(seq_num - 1, packet);
        realtimeDelay(mPacketDurationMs);
        simulateAudioCallback();
        EXPECT_TRUE(validateAudioContinuity(mOutputBuffer));
    }

    printDetailedStatistics("RegulatorTest.BurstAndGapPattern");
}

TEST_F(RegulatorTest, StatisticsAccuracy)
{
    std::cout << "\n=== StatisticsAccuracy Test ===" << std::endl;

            // Run realistic test with some packet loss
    int seq_num = 0;
    for (int i = 0; i < 30; i++) {
        // Skip every 7th packet to create some loss
        if (i % 7 != 0) {
            auto packet = createSinePacket(seq_num);
            simulatePacketArrival(seq_num, packet);
        }
        seq_num++;

        realtimeDelay(mAudioCallbackIntervalMs);
        simulateAudioCallback();
    }

    RingBuffer::IOStat stats;
    EXPECT_TRUE(mRegulator->getStats(&stats, false));

    std::cout << "Statistics test completed with some packet loss patterns" << std::endl;
    printDetailedStatistics("RegulatorTest.StatisticsAccuracy");
}
