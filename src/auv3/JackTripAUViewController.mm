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

#import "JackTripAUViewController.h"
#import "JackTripAU.h"
#import <CoreAudioKit/CoreAudioKit.h>

@interface JackTripAUViewController () {
    // UI Controls
    NSSlider *sendGainSlider;
    NSSlider *outputMixSlider;
    NSSlider *outputGainSlider;
    NSTextField *connectedLabel;
    
    // Parameter observers
    AUParameterObserverToken parameterObserver;
}

@end

@implementation JackTripAUViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Set up the view
    self.view = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 400, 300)];
    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = [[NSColor controlBackgroundColor] CGColor];
    
    [self createUI];
    [self connectParametersToControls];
}

- (void)createUI {
    // Title label
    NSTextField *titleLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 250, 360, 30)];
    titleLabel.stringValue = @"JackTrip Audio Bridge";
    titleLabel.font = [NSFont boldSystemFontOfSize:18];
    titleLabel.textColor = [NSColor labelColor];
    titleLabel.backgroundColor = [NSColor clearColor];
    titleLabel.bordered = NO;
    titleLabel.editable = NO;
    titleLabel.alignment = NSTextAlignmentCenter;
    [self.view addSubview:titleLabel];
    
    // Send Gain control
    NSTextField *sendGainLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 200, 100, 20)];
    sendGainLabel.stringValue = @"Send Gain:";
    sendGainLabel.textColor = [NSColor labelColor];
    sendGainLabel.backgroundColor = [NSColor clearColor];
    sendGainLabel.bordered = NO;
    sendGainLabel.editable = NO;
    [self.view addSubview:sendGainLabel];
    
    sendGainSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(130, 200, 200, 20)];
    sendGainSlider.minValue = 0.0;
    sendGainSlider.maxValue = 1.0;
    sendGainSlider.doubleValue = 1.0;
    sendGainSlider.continuous = YES;
    sendGainSlider.target = self;
    sendGainSlider.action = @selector(sendGainChanged:);
    [self.view addSubview:sendGainSlider];
    
    // Output Mix control
    NSTextField *outputMixLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 160, 100, 20)];
    outputMixLabel.stringValue = @"Output Mix:";
    outputMixLabel.textColor = [NSColor labelColor];
    outputMixLabel.backgroundColor = [NSColor clearColor];
    outputMixLabel.bordered = NO;
    outputMixLabel.editable = NO;
    [self.view addSubview:outputMixLabel];
    
    outputMixSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(130, 160, 200, 20)];
    outputMixSlider.minValue = 0.0;
    outputMixSlider.maxValue = 1.0;
    outputMixSlider.doubleValue = 0.0;
    outputMixSlider.continuous = YES;
    outputMixSlider.target = self;
    outputMixSlider.action = @selector(outputMixChanged:);
    [self.view addSubview:outputMixSlider];
    
    // Output Gain control
    NSTextField *outputGainLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 120, 100, 20)];
    outputGainLabel.stringValue = @"Output Gain:";
    outputGainLabel.textColor = [NSColor labelColor];
    outputGainLabel.backgroundColor = [NSColor clearColor];
    outputGainLabel.bordered = NO;
    outputGainLabel.editable = NO;
    [self.view addSubview:outputGainLabel];
    
    outputGainSlider = [[NSSlider alloc] initWithFrame:NSMakeRect(130, 120, 200, 20)];
    outputGainSlider.minValue = 0.0;
    outputGainSlider.maxValue = 1.0;
    outputGainSlider.doubleValue = 1.0;
    outputGainSlider.continuous = YES;
    outputGainSlider.target = self;
    outputGainSlider.action = @selector(outputGainChanged:);
    [self.view addSubview:outputGainSlider];
    
    // Connection status
    NSTextField *connectionLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 80, 100, 20)];
    connectionLabel.stringValue = @"Status:";
    connectionLabel.textColor = [NSColor labelColor];
    connectionLabel.backgroundColor = [NSColor clearColor];
    connectionLabel.bordered = NO;
    connectionLabel.editable = NO;
    [self.view addSubview:connectionLabel];
    
    connectedLabel = [[NSTextField alloc] initWithFrame:NSMakeRect(130, 80, 200, 20)];
    connectedLabel.stringValue = @"Disconnected";
    connectedLabel.textColor = [NSColor systemRedColor];
    connectedLabel.backgroundColor = [NSColor clearColor];
    connectedLabel.bordered = NO;
    connectedLabel.editable = NO;
    [self.view addSubview:connectedLabel];
}

- (void)connectParametersToControls {
    if (!self.audioUnit || !self.audioUnit.parameterTree) {
        return;
    }
    
    AUParameterTree *parameterTree = self.audioUnit.parameterTree;
    
    // Set up parameter observer
    __weak JackTripAUViewController *weakSelf = self;
    parameterObserver = [parameterTree tokenByAddingParameterObserver:^(AUParameterAddress address, AUValue value) {
        JackTripAUViewController *strongSelf = weakSelf;
        if (!strongSelf) return;
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [strongSelf updateControlForParameter:address value:value];
        });
    }];
    
    // Initialize control values from parameters
    AUParameter *sendGainParam = [parameterTree parameterWithAddress:kJackTripAUParam_SendGain];
    if (sendGainParam) {
        sendGainSlider.doubleValue = sendGainParam.value;
    }
    
    AUParameter *outputMixParam = [parameterTree parameterWithAddress:kJackTripAUParam_OutputMix];
    if (outputMixParam) {
        outputMixSlider.doubleValue = outputMixParam.value;
    }
    
    AUParameter *outputGainParam = [parameterTree parameterWithAddress:kJackTripAUParam_OutputGain];
    if (outputGainParam) {
        outputGainSlider.doubleValue = outputGainParam.value;
    }
    
    AUParameter *connectedParam = [parameterTree parameterWithAddress:kJackTripAUParam_Connected];
    if (connectedParam) {
        [self updateConnectionStatus:connectedParam.value > 0.5];
    }
}

- (void)updateControlForParameter:(AUParameterAddress)address value:(AUValue)value {
    switch (address) {
        case kJackTripAUParam_SendGain:
            sendGainSlider.doubleValue = value;
            break;
        case kJackTripAUParam_OutputMix:
            outputMixSlider.doubleValue = value;
            break;
        case kJackTripAUParam_OutputGain:
            outputGainSlider.doubleValue = value;
            break;
        case kJackTripAUParam_Connected:
            [self updateConnectionStatus:value > 0.5];
            break;
    }
}

- (void)updateConnectionStatus:(BOOL)connected {
    if (connected) {
        connectedLabel.stringValue = @"Connected";
        connectedLabel.textColor = [NSColor systemGreenColor];
    } else {
        connectedLabel.stringValue = @"Disconnected";
        connectedLabel.textColor = [NSColor systemRedColor];
    }
}

// Control actions
- (void)sendGainChanged:(NSSlider *)sender {
    AUParameter *param = [self.audioUnit.parameterTree parameterWithAddress:kJackTripAUParam_SendGain];
    if (param) {
        param.value = sender.doubleValue;
    }
}

- (void)outputMixChanged:(NSSlider *)sender {
    AUParameter *param = [self.audioUnit.parameterTree parameterWithAddress:kJackTripAUParam_OutputMix];
    if (param) {
        param.value = sender.doubleValue;
    }
}

- (void)outputGainChanged:(NSSlider *)sender {
    AUParameter *param = [self.audioUnit.parameterTree parameterWithAddress:kJackTripAUParam_OutputGain];
    if (param) {
        param.value = sender.doubleValue;
    }
}

- (void)dealloc {
    if (parameterObserver) {
        [self.audioUnit.parameterTree removeParameterObserver:parameterObserver];
        parameterObserver = nil;
    }
}

@end 