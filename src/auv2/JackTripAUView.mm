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

#import "JackTripAUView.h"
#include "JackTripAU.h"

// Parameter listener callback
static void ParameterListenerDispatcher(void* inRefCon, void* /* inObject */,
                                      const AudioUnitParameter* inParameter,
                                      Float32 /* inValue */) {
    JackTripAUView* view = (__bridge JackTripAUView*)inRefCon;
    [view parameterChanged:inParameter->mParameterID];
}

@implementation JackTripAUView

- (id)initWithFrame:(NSRect)frameRect audioUnit:(AudioUnit)inAU {
    self = [super initWithFrame:frameRect];
    if (self) {
        mAU = inAU;
        mParameterListener = nullptr;
        
        // Load images from the component's bundle
        NSBundle* componentBundle = [NSBundle bundleForClass:[self class]];
        
        // Try to load the high-resolution background first, fall back to regular version
        NSString* backgroundPath = [componentBundle pathForResource:@"background_2x" ofType:@"png"];
        if (!backgroundPath) {
            backgroundPath = [componentBundle pathForResource:@"background" ofType:@"png"];
        }
        mBackgroundImage = backgroundPath ? [[NSImage alloc] initWithContentsOfFile:backgroundPath] : nil;
        
        NSString* knobPath = [componentBundle pathForResource:@"Sercan_Moog_Knob" ofType:@"png"];
        mKnobImage = knobPath ? [[NSImage alloc] initWithContentsOfFile:knobPath] : nil;
        
        NSString* ledPath = [componentBundle pathForResource:@"Dual_LED" ofType:@"png"];
        mLEDImage = ledPath ? [[NSImage alloc] initWithContentsOfFile:ledPath] : nil;
        
        // Debug logging to help troubleshoot image loading
        NSLog(@"JackTrip AUv2: Bundle path = %@", [componentBundle bundlePath]);
        NSLog(@"JackTrip AUv2: Background image %@ (path: %@)", mBackgroundImage ? @"loaded" : @"FAILED", backgroundPath);
        NSLog(@"JackTrip AUv2: Knob image %@ (path: %@)", mKnobImage ? @"loaded" : @"FAILED", knobPath);
        NSLog(@"JackTrip AUv2: LED image %@ (path: %@)", mLEDImage ? @"loaded" : @"FAILED", ledPath);
        
        [self setupUI];
        [self setupParameterListener];
        
        // Initialize knob values from current parameter values
        [self updateAllParameterValues];
        
        // Set up a timer to periodically check connection state
        mUpdateTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                                                        target:self
                                                      selector:@selector(updateConnectionStatus:)
                                                      userInfo:nil
                                                       repeats:YES];
    }
    return self;
}

- (void)dealloc {
    if (mParameterListener) {
        AUListenerDispose(mParameterListener);
    }
    
    // Stop and release timer
    if (mUpdateTimer) {
        [mUpdateTimer invalidate];
        mUpdateTimer = nil;
    }
    
    // Release images
    [mBackgroundImage release];
    [mKnobImage release]; 
    [mLEDImage release];
    
    [super dealloc];
}

- (void)setupUI {
    // Set up background
    mBackgroundView = [[NSImageView alloc] initWithFrame:self.bounds];
    [mBackgroundView setImage:mBackgroundImage];
    [mBackgroundView setImageScaling:NSImageScaleProportionallyUpOrDown];
    [self addSubview:mBackgroundView];
    
    // Connection LED (top right)
    NSRect ledFrame = NSMakeRect(335, 200-70, 62, 62); // Adjust Y for Cocoa coordinates
    mConnectionLED = [[JackTripLEDView alloc] initWithFrame:ledFrame LEDImage:mLEDImage];
    [self addSubview:mConnectionLED];
    
    // Send knob and labels
    NSRect sendKnobFrame = NSMakeRect(70, 200-172, 72, 72); // Adjust Y for Cocoa coordinates
    mSendKnob = [[JackTripKnobView alloc] initWithFrame:sendKnobFrame
                                              audioUnit:mAU
                                            parameterID:kJackTripAUParam_SendGain
                                              knobImage:mKnobImage
                                             frameCount:120];
    [mSendKnob setParameterRange:0.0 maxValue:1.0];
    [self addSubview:mSendKnob];
    
    // Send labels
    mSendLabel = [self createLabel:NSMakeRect(75, 200-100, 60, 20) text:@"Send"];
    [self addSubview:mSendLabel];
    
    mToJackTripLabel = [self createSmallLabel:NSMakeRect(50, 200-190, 110, 15) text:@"To JackTrip"];
    [self addSubview:mToJackTripLabel];
    
    // Output Mix knob and labels
    NSRect mixKnobFrame = NSMakeRect(250, 200-172, 72, 72); // Adjust Y for Cocoa coordinates
    mOutputMixKnob = [[JackTripKnobView alloc] initWithFrame:mixKnobFrame
                                                   audioUnit:mAU
                                                 parameterID:kJackTripAUParam_OutputMix
                                                   knobImage:mKnobImage
                                                  frameCount:120];
    [mOutputMixKnob setParameterRange:0.0 maxValue:1.0];
    [self addSubview:mOutputMixKnob];
    
    // Output Mix labels
    mOutputMixLabel = [self createLabel:NSMakeRect(235, 200-100, 100, 20) text:@"Output Mix"];
    [self addSubview:mOutputMixLabel];
    
    mPassThroughLabel = [self createSmallLabel:NSMakeRect(195, 200-190, 90, 15) text:@"Pass-Through"];
    [self addSubview:mPassThroughLabel];
    
    mFromJackTripLabel = [self createSmallLabel:NSMakeRect(285, 200-190, 90, 15) text:@"From JackTrip"];
    [self addSubview:mFromJackTripLabel];
    
    // For now, we'll skip the Output Gain knob since it's not in the original AUv2 parameters
    // but we can add it later if needed
}

- (NSTextField*)createLabel:(NSRect)frame text:(NSString*)text {
    NSTextField* label = [[NSTextField alloc] initWithFrame:frame];
    [label setStringValue:text];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setTextColor:[NSColor whiteColor]];
    [label setAlignment:NSTextAlignmentCenter];
    [label setFont:[NSFont systemFontOfSize:14 weight:NSFontWeightMedium]];
    [[label cell] setLineBreakMode:NSLineBreakByClipping];
    return label;
}

- (NSTextField*)createSmallLabel:(NSRect)frame text:(NSString*)text {
    NSTextField* label = [[NSTextField alloc] initWithFrame:frame];
    [label setStringValue:text];
    [label setBezeled:NO];
    [label setDrawsBackground:NO];
    [label setEditable:NO];
    [label setSelectable:NO];
    [label setTextColor:[NSColor whiteColor]];
    [label setAlignment:NSTextAlignmentCenter];
    [label setFont:[NSFont systemFontOfSize:10]];
    [[label cell] setLineBreakMode:NSLineBreakByClipping];
    return label;
}

- (void)setupParameterListener {
    OSStatus result = AUListenerCreate(ParameterListenerDispatcher, (__bridge void*)self,
                                     CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 
                                     0.05, &mParameterListener);
    if (result == noErr) {
        AudioUnitParameter parameter;
        parameter.mAudioUnit = mAU;
        parameter.mScope = kAudioUnitScope_Global;
        parameter.mElement = 0;
        
        // Listen to parameter changes
        parameter.mParameterID = kJackTripAUParam_SendGain;
        AUListenerAddParameter(mParameterListener, nullptr, &parameter);
        
        parameter.mParameterID = kJackTripAUParam_OutputMix;
        AUListenerAddParameter(mParameterListener, nullptr, &parameter);
        
        parameter.mParameterID = kJackTripAUParam_OutputGain;
        AUListenerAddParameter(mParameterListener, nullptr, &parameter);
        
        parameter.mParameterID = kJackTripAUParam_Connected;
        AUListenerAddParameter(mParameterListener, nullptr, &parameter);
    }
}

- (void)setAudioUnit:(AudioUnit)inAU {
    mAU = inAU;
    [mSendKnob setAudioUnit:inAU];
    [mOutputMixKnob setAudioUnit:inAU];
    
    // Update initial values
    [self parameterChanged:kJackTripAUParam_SendGain];
    [self parameterChanged:kJackTripAUParam_OutputMix];
    [self parameterChanged:kJackTripAUParam_Connected];
}

- (void)updateAllParameterValues {
    if (!mAU) return;
    
    [self parameterChanged:kJackTripAUParam_SendGain];
    [self parameterChanged:kJackTripAUParam_OutputMix];
    [self parameterChanged:kJackTripAUParam_Connected];
}

- (void)updateConnectionStatus:(NSTimer*)timer {
    // Periodically check connection status in case parameter listener misses updates
    [self parameterChanged:kJackTripAUParam_Connected];
}

- (void)parameterChanged:(AudioUnitParameterID)parameterID {
    if (!mAU) return;
    
    Float32 value;
    OSStatus result = AudioUnitGetParameter(mAU, parameterID, kAudioUnitScope_Global, 0, &value);
    if (result != noErr) {
        NSLog(@"JackTrip AUv2: Failed to get parameter %d, error %d", (int)parameterID, (int)result);
        return;
    }
    
    switch (parameterID) {
        case kJackTripAUParam_SendGain:
            [mSendKnob setValue:value];
            NSLog(@"JackTrip AUv2: Send gain updated to %f", value);
            break;
        case kJackTripAUParam_OutputMix:
            [mOutputMixKnob setValue:value];
            NSLog(@"JackTrip AUv2: Output mix updated to %f", value);
            break;
        case kJackTripAUParam_Connected:
            [mConnectionLED setConnected:(value > 0.5)];
            NSLog(@"JackTrip AUv2: Connected state updated to %s", (value > 0.5) ? "true" : "false");
            break;
        default:
            break;
    }
}

@end

#pragma mark - JackTripKnobView Implementation

@implementation JackTripKnobView

- (id)initWithFrame:(NSRect)frameRect 
          audioUnit:(AudioUnit)inAU 
        parameterID:(AudioUnitParameterID)parameterID
           knobImage:(NSImage*)knobImage
          frameCount:(int)frameCount {
    self = [super initWithFrame:frameRect];
    if (self) {
        mAU = inAU;
        mParameterID = parameterID;
        mKnobImage = knobImage;
        mFrameCount = frameCount;
        mMinValue = 0.0;
        mMaxValue = 1.0;
        mIsTracking = NO;
        
        // Get the current parameter value from the AudioUnit
        if (mAU) {
            Float32 currentValue;
            OSStatus result = AudioUnitGetParameter(mAU, mParameterID, kAudioUnitScope_Global, 0, &currentValue);
            mValue = (result == noErr) ? currentValue : 0.5;
        } else {
            mValue = 0.5; // fallback
        }
    }
    return self;
}

- (void)setAudioUnit:(AudioUnit)inAU {
    mAU = inAU;
}

- (void)setValue:(float)value {
    mValue = fmax(mMinValue, fmin(mMaxValue, value));
    [self setNeedsDisplay:YES];
}

- (float)getValue {
    return mValue;
}

- (void)setParameterRange:(float)minValue maxValue:(float)maxValue {
    mMinValue = minValue;
    mMaxValue = maxValue;
}

- (void)drawRect:(NSRect)dirtyRect {
    if (!mKnobImage) return;
    
    NSRect bounds = self.bounds;
    
    // Calculate which frame to show based on the current value
    float normalizedValue = (mValue - mMinValue) / (mMaxValue - mMinValue);
    // Invert the frame index so higher values show later frames (clockwise rotation)
    int frameIndex = (int)((1.0f - normalizedValue) * (mFrameCount - 1));
    frameIndex = fmax(0, fmin(mFrameCount - 1, frameIndex));
    
    // Debug output (remove after testing)
    static int debugCount = 0;
    if (debugCount < 5) { // Only log first few draws to avoid spam
        NSLog(@"JackTrip Knob %d: value=%f, normalized=%f, frame=%d/%d", 
              (int)mParameterID, mValue, normalizedValue, frameIndex, mFrameCount-1);
        debugCount++;
    }
    
    // Calculate the source rect for the current frame
    // The knob image contains 120 frames arranged vertically
    NSSize imageSize = [mKnobImage size];
    float frameHeight = imageSize.height / mFrameCount;
    NSRect sourceRect = NSMakeRect(0, frameIndex * frameHeight, imageSize.width, frameHeight);
    
    // Draw the appropriate frame
    [mKnobImage drawInRect:bounds fromRect:sourceRect operation:NSCompositingOperationSourceOver fraction:1.0];
}

- (void)mouseDown:(NSEvent*)event {
    mIsTracking = YES;
    mLastMousePoint = [self convertPoint:[event locationInWindow] fromView:nil];
}

- (void)mouseDragged:(NSEvent*)event {
    if (!mIsTracking) return;
    
    NSPoint currentPoint = [self convertPoint:[event locationInWindow] fromView:nil];
    float deltaY = currentPoint.y - mLastMousePoint.y;
    
    // Adjust sensitivity
    float sensitivity = 0.005;
    float newValue = mValue + (deltaY * sensitivity);
    
    [self setValue:newValue];
    
    // Update AudioUnit parameter
    if (mAU) {
        AudioUnitSetParameter(mAU, mParameterID, kAudioUnitScope_Global, 0, mValue, 0);
    }
    
    mLastMousePoint = currentPoint;
}

- (void)mouseUp:(NSEvent*)event {
    mIsTracking = NO;
}

@end

#pragma mark - JackTripLEDView Implementation

@implementation JackTripLEDView

- (id)initWithFrame:(NSRect)frameRect LEDImage:(NSImage*)ledImage {
    self = [super initWithFrame:frameRect];
    if (self) {
        mLEDImage = ledImage;
        mIsConnected = NO;
    }
    return self;
}

- (void)setConnected:(BOOL)connected {
    if (mIsConnected != connected) {
        mIsConnected = connected;
        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    if (!mLEDImage) return;
    
    NSRect bounds = self.bounds;
    
    // The LED image contains both red and green states
    // Use the appropriate half based on connection state
    NSSize imageSize = [mLEDImage size];
    float halfHeight = imageSize.height / 2;
    NSRect sourceRect;
    
    if (mIsConnected) {
        // Use bottom half for green (connected)
        sourceRect = NSMakeRect(0, 0, imageSize.width, halfHeight);
    } else {
        // Use top half for red (disconnected)
        sourceRect = NSMakeRect(0, halfHeight, imageSize.width, halfHeight);
    }
    
    [mLEDImage drawInRect:bounds fromRect:sourceRect operation:NSCompositingOperationSourceOver fraction:1.0];
}

@end 