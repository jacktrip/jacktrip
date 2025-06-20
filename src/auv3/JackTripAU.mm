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

#import "JackTripAU.h"
#import "JackTripAUViewController.h"
#import <AudioUnit/AudioUnit.h>
#import <AVFoundation/AVFoundation.h>

// Import C++ headers
#include "../AudioBridgeProcessor.h"
#include <algorithm>
#include <cmath>
#include <memory>

// Parameter ranges - matching AUv2
static const float kMinGain = 0.0f;
static const float kMaxGain = 1.0f;
static const float kDefaultSendGain = 1.0f;
static const float kDefaultOutputMix = 0.0f;
static const float kDefaultOutputGain = 1.0f;

// C++ implementation class
class JackTripAUImpl {
public:
    JackTripAUImpl() {
        // Initialize parameters to defaults
        mSendGain = kDefaultSendGain;
        mOutputMix = kDefaultOutputMix;
        mOutputGain = kDefaultOutputGain;
        mConnected = false;
        
        updateVolumeMultipliers();
    }
    
    ~JackTripAUImpl() {
        uninitialize();
    }
    
    bool initialize(double sampleRate, UInt32 maxFramesToRender) {
        mSampleRate = sampleRate;
        mMaxFramesToRender = maxFramesToRender;
        
        // Initialize the audio bridge processor
        mProcessor.initialize(static_cast<unsigned int>(sampleRate), 
                             static_cast<unsigned int>(maxFramesToRender));
        
        mInitialized = true;
        return true;
    }
    
    void uninitialize() {
        if (mInitialized) {
            mProcessor.uninitialize();
            mInitialized = false;
        }
    }
    
    void setParameter(AUParameterAddress address, float value) {
        switch (address) {
            case kJackTripAUParam_SendGain:
                mSendGain = std::clamp(value, kMinGain, kMaxGain);
                break;
            case kJackTripAUParam_OutputMix:
                mOutputMix = std::clamp(value, kMinGain, kMaxGain);
                break;
            case kJackTripAUParam_OutputGain:
                mOutputGain = std::clamp(value, kMinGain, kMaxGain);
                break;
            case kJackTripAUParam_Connected:
                // This is read-only, updated internally
                break;
        }
        updateVolumeMultipliers();
    }
    
    float getParameter(AUParameterAddress address) {
        switch (address) {
            case kJackTripAUParam_SendGain:
                return mSendGain;
            case kJackTripAUParam_OutputMix:
                return mOutputMix;
            case kJackTripAUParam_OutputGain:
                return mOutputGain;
            case kJackTripAUParam_Connected:
                return mConnected ? 1.0f : 0.0f;
            default:
                return 0.0f;
        }
    }
    
    void processAudio(AudioBufferList* inBufferList, 
                     AudioBufferList* outBufferList, 
                     UInt32 frameCount) {
        if (!mInitialized) {
            // Zero output if not initialized
            for (UInt32 i = 0; i < outBufferList->mNumberBuffers; ++i) {
                memset(outBufferList->mBuffers[i].mData, 0, 
                       outBufferList->mBuffers[i].mDataByteSize);
            }
            return;
        }
        
        // Update connection state
        bool currentlyConnected = mProcessor.isConnected();
        if (currentlyConnected != mConnected) {
            mConnected = currentlyConnected;
        }
        
        // Set up input buffers
        bool inputSilenceFlags[AudioSocketNumChannels];
        float* inputBuffers[AudioSocketNumChannels];
        for (UInt32 ch = 0; ch < AudioSocketNumChannels; ch++) {
            if (ch < inBufferList->mNumberBuffers) {
                inputBuffers[ch] = static_cast<float*>(inBufferList->mBuffers[ch].mData);
            } else {
                inputBuffers[ch] = nullptr;
            }
            inputSilenceFlags[ch] = false;
        }
        
        // Set up output buffers
        bool outputSilenceFlags[AudioSocketNumChannels];
        float* outputBuffers[AudioSocketNumChannels];
        for (UInt32 ch = 0; ch < AudioSocketNumChannels; ch++) {
            if (ch < outBufferList->mNumberBuffers) {
                outputBuffers[ch] = static_cast<float*>(outBufferList->mBuffers[ch].mData);
            } else {
                outputBuffers[ch] = nullptr;
            }
        }
        
        // Process through the audio bridge processor
        mProcessor.process(inputBuffers, outputBuffers,
                          inputSilenceFlags, outputSilenceFlags, 
                          static_cast<unsigned int>(frameCount));
        
        // Handle any remaining output channels by zeroing them
        for (UInt32 ch = AudioSocketNumChannels; ch < outBufferList->mNumberBuffers; ch++) {
            memset(outBufferList->mBuffers[ch].mData, 0, 
                   outBufferList->mBuffers[ch].mDataByteSize);
        }
    }
    
    bool isConnected() const {
        return mConnected;
    }
    
private:
    void updateVolumeMultipliers(void) {
        // Calculate volume multipliers from parameters
        float outMul = AudioBridgeProcessor::gainToVol(mOutputGain);
        float sendMul = AudioBridgeProcessor::gainToVol(mSendGain);
        float recvMul = mOutputMix * outMul;
        float passMul = (1.0f - mOutputMix) * outMul;

        // Update processor parameters
        mProcessor.setSendMul(sendMul);
        mProcessor.setRecvMul(recvMul);
        mProcessor.setPassMul(passMul);
    }
    
    // Audio bridge processor
    AudioBridgeProcessor mProcessor;
    
    // Parameters
    float mSendGain;
    float mOutputMix;
    float mOutputGain;
    bool mConnected;
    
    // State
    double mSampleRate = 44100.0;
    UInt32 mMaxFramesToRender = 512;
    bool mInitialized = false;
};

@interface JackTripAU () {
    // C++ implementation
    std::unique_ptr<JackTripAUImpl> mImpl;
}

@property (nonatomic, readwrite) AUAudioUnitBus *inputBus;
@property (nonatomic, readwrite) AUAudioUnitBus *outputBus;
@property (nonatomic, readwrite) AUAudioUnitBusArray *inputBusArray;
@property (nonatomic, readwrite) AUAudioUnitBusArray *outputBusArray;

@end

@implementation JackTripAU

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription
                                     options:(AudioComponentInstantiationOptions)options
                                       error:(NSError **)outError {
    self = [super initWithComponentDescription:componentDescription options:options error:outError];
    if (self == nil) {
        return nil;
    }
    
    // Create C++ implementation
    mImpl = std::make_unique<JackTripAUImpl>();
    
    // Create audio format (stereo 32-bit float)
    AVAudioFormat *defaultFormat = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:44100.0 
                                                                                   channels:2];
    
    // Create input and output buses
    _inputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
    _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
    
    // Create bus arrays
    _inputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self
                                                            busType:AUAudioUnitBusTypeInput
                                                             busses:@[_inputBus]];
    _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self
                                                             busType:AUAudioUnitBusTypeOutput
                                                              busses:@[_outputBus]];
    
    // Create parameters
    [self createParameters];
    
    // Set maximum frames to render
    self.maximumFramesToRender = 512;
    
    return self;
}

- (void)createParameters {
    // Create parameter objects
    AUParameter *sendGainParam = [AUParameterTree createParameterWithIdentifier:@"sendGain"
                                                                            name:@"Send Gain"
                                                                         address:kJackTripAUParam_SendGain
                                                                             min:kMinGain
                                                                             max:kMaxGain
                                                                            unit:kAudioUnitParameterUnit_LinearGain
                                                                        unitName:nil
                                                                           flags:kAudioUnitParameterFlag_IsWritable |
                                                                                 kAudioUnitParameterFlag_IsReadable |
                                                                                 kAudioUnitParameterFlag_IsHighResolution
                                                                    valueStrings:nil
                                                             dependentParameters:nil];
    
    AUParameter *outputMixParam = [AUParameterTree createParameterWithIdentifier:@"outputMix"
                                                                             name:@"Output Mix"
                                                                          address:kJackTripAUParam_OutputMix
                                                                              min:kMinGain
                                                                              max:kMaxGain
                                                                             unit:kAudioUnitParameterUnit_LinearGain
                                                                         unitName:nil
                                                                            flags:kAudioUnitParameterFlag_IsWritable |
                                                                                  kAudioUnitParameterFlag_IsReadable |
                                                                                  kAudioUnitParameterFlag_IsHighResolution
                                                                     valueStrings:nil
                                                              dependentParameters:nil];
    
    AUParameter *outputGainParam = [AUParameterTree createParameterWithIdentifier:@"outputGain"
                                                                              name:@"Output Gain"
                                                                           address:kJackTripAUParam_OutputGain
                                                                               min:kMinGain
                                                                               max:kMaxGain
                                                                              unit:kAudioUnitParameterUnit_LinearGain
                                                                          unitName:nil
                                                                             flags:kAudioUnitParameterFlag_IsWritable |
                                                                                   kAudioUnitParameterFlag_IsReadable |
                                                                                   kAudioUnitParameterFlag_IsHighResolution
                                                                      valueStrings:nil
                                                               dependentParameters:nil];
    
    AUParameter *connectedParam = [AUParameterTree createParameterWithIdentifier:@"connected"
                                                                             name:@"Connected"
                                                                          address:kJackTripAUParam_Connected
                                                                              min:0.0f
                                                                              max:1.0f
                                                                             unit:kAudioUnitParameterUnit_Boolean
                                                                         unitName:nil
                                                                            flags:kAudioUnitParameterFlag_IsReadable
                                                                     valueStrings:nil
                                                              dependentParameters:nil];
    
    // Set default values
    sendGainParam.value = kDefaultSendGain;
    outputMixParam.value = kDefaultOutputMix;
    outputGainParam.value = kDefaultOutputGain;
    connectedParam.value = 0.0f;
    
    // Create parameter tree
    self.parameterTree = [AUParameterTree createTreeWithChildren:@[sendGainParam, outputMixParam, outputGainParam, connectedParam]];
    
    // Set up parameter observers
    __weak JackTripAU *weakSelf = self;
    self.parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
        __strong JackTripAU *strongSelf = weakSelf;
        if (strongSelf && strongSelf->mImpl) {
            strongSelf->mImpl->setParameter(param.address, value);
        }
    };
    
    self.parameterTree.implementorValueProvider = ^AUValue(AUParameter *param) {
        __strong JackTripAU *strongSelf = weakSelf;
        if (strongSelf && strongSelf->mImpl) {
            return strongSelf->mImpl->getParameter(param.address);
        }
        return 0.0f;
    };
}

- (BOOL)allocateRenderResourcesAndReturnError:(NSError **)outError {
    if (![super allocateRenderResourcesAndReturnError:outError]) {
        return NO;
    }
    
    // Initialize C++ implementation
    if (mImpl) {
        mImpl->initialize(self.outputBus.format.sampleRate, self.maximumFramesToRender);
    }
    
    // Create render block
    __weak JackTripAU *weakSelf = self;
    self.internalRenderBlock = ^AUAudioUnitStatus(AudioUnitRenderActionFlags *actionFlags,
                                                  const AudioTimeStamp *timestamp,
                                                  AVAudioFrameCount frameCount,
                                                  NSInteger outputBusNumber,
                                                  AudioBufferList *outputData,
                                                  const AURenderEvent *realtimeEventListHead,
                                                  AURenderPullInputBlock pullInputBlock) {
        #pragma unused(actionFlags, outputBusNumber, realtimeEventListHead)
        
        __strong JackTripAU *strongSelf = weakSelf;
        if (!strongSelf || !strongSelf->mImpl) {
            return kAudioUnitErr_Uninitialized;
        }
        
        // Pull input - create a temporary input buffer
        AudioBufferList *inputData = nullptr;
        UInt32 inputBufferSize = outputData->mBuffers[0].mDataByteSize;
        AudioBufferList inputBufferList;
        Float32 *inputBuffer0 = nullptr;
        Float32 *inputBuffer1 = nullptr;
        
        if (pullInputBlock) {
            // Allocate temporary input buffers
            inputBuffer0 = (Float32 *)malloc(inputBufferSize);
            inputBuffer1 = (Float32 *)malloc(inputBufferSize);
            
            inputBufferList.mNumberBuffers = 2;
            inputBufferList.mBuffers[0].mNumberChannels = 1;
            inputBufferList.mBuffers[0].mDataByteSize = inputBufferSize;
            inputBufferList.mBuffers[0].mData = inputBuffer0;
            inputBufferList.mBuffers[1].mNumberChannels = 1;
            inputBufferList.mBuffers[1].mDataByteSize = inputBufferSize;
            inputBufferList.mBuffers[1].mData = inputBuffer1;
            
            AudioUnitRenderActionFlags inputFlags = 0;
            OSStatus status = pullInputBlock(&inputFlags, timestamp, frameCount, 0, &inputBufferList);
            if (status != noErr) {
                free(inputBuffer0);
                free(inputBuffer1);
                return status;
            }
            inputData = &inputBufferList;
        }
        
        // Process audio through C++ implementation
        strongSelf->mImpl->processAudio(inputData, outputData, frameCount);
        
        // Clean up temporary input buffers
        if (inputBuffer0) free(inputBuffer0);
        if (inputBuffer1) free(inputBuffer1);
        
        return noErr;
    };
    
    return YES;
}

- (void)deallocateRenderResources {
    if (mImpl) {
        mImpl->uninitialize();
    }
    
    [super deallocateRenderResources];
}

- (NSArray<NSString *> *)MIDIOutputNames {
    return @[];
}

- (NSArray<NSString *> *)channelCapabilities {
    return @[];
}

#pragma mark - AUAudioUnitFactory

- (NSIndexSet *)supportedViewConfigurations:(NSArray<AUAudioUnitViewConfiguration *> *)availableViewConfigurations {
    // Return the first available view configuration
    if (availableViewConfigurations.count > 0) {
        return [NSIndexSet indexSetWithIndex:0];
    }
    return [super supportedViewConfigurations:availableViewConfigurations];
}

- (void)requestViewControllerWithCompletionHandler:(void (^)(AUViewControllerBase * _Nullable))completionHandler {
    JackTripAUViewController *viewController = [[JackTripAUViewController alloc] init];
    viewController.audioUnit = self;
    completionHandler(viewController);
}

@end 