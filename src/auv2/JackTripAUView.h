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

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioUnit/AUCocoaUIView.h>
#import <AudioToolbox/AudioUnitUtilities.h>

#ifndef __JackTripAUView_h__
#define __JackTripAUView_h__

@class JackTripKnobView;
@class JackTripLEDView;

@interface JackTripAUView : NSView {
    AudioUnit mAU;
    AUEventListenerRef mParameterListener;
    NSTimer* mUpdateTimer;
    
    // UI Elements
    NSImageView* mBackgroundView;
    JackTripKnobView* mSendKnob;
    JackTripKnobView* mOutputMixKnob;
    JackTripKnobView* mOutputGainKnob;
    JackTripLEDView* mConnectionLED;
    
    // Labels
    NSTextField* mSendLabel;
    NSTextField* mOutputMixLabel;
    NSTextField* mOutputGainLabel;
    NSTextField* mToJackTripLabel;
    NSTextField* mFromJackTripLabel;
    NSTextField* mPassThroughLabel;
    
    // Resources
    NSImage* mBackgroundImage;
    NSImage* mKnobImage;
    NSImage* mLEDImage;
}

- (id)initWithFrame:(NSRect)frameRect audioUnit:(AudioUnit)inAU;
- (void)dealloc;
- (void)setAudioUnit:(AudioUnit)inAU;
- (void)parameterChanged:(AudioUnitParameterID)parameterID;

@end

// Custom knob control
@interface JackTripKnobView : NSView {
    AudioUnit mAU;
    AudioUnitParameterID mParameterID;
    NSImage* mKnobImage;
    float mValue;
    float mMinValue;
    float mMaxValue;
    int mFrameCount;
    NSPoint mLastMousePoint;
    BOOL mIsTracking;
}

- (id)initWithFrame:(NSRect)frameRect 
          audioUnit:(AudioUnit)inAU 
        parameterID:(AudioUnitParameterID)parameterID
           knobImage:(NSImage*)knobImage
          frameCount:(int)frameCount;
- (void)setValue:(float)value;
- (float)getValue;
- (void)setParameterRange:(float)minValue maxValue:(float)maxValue;
- (void)setAudioUnit:(AudioUnit)inAU;

@end

// Custom LED indicator
@interface JackTripLEDView : NSView {
    NSImage* mLEDImage;
    BOOL mIsConnected;
}

- (id)initWithFrame:(NSRect)frameRect LEDImage:(NSImage*)ledImage;
- (void)setConnected:(BOOL)connected;

@end

#endif 