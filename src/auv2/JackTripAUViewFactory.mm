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

#import <AudioUnit/AudioUnit.h>
#import <AudioUnit/AUCocoaUIView.h>
#import "JackTripAUView.h"

@interface JackTripAUViewFactory : NSObject <AUCocoaUIBase>
{
}

@end

// Export the class for external access
extern "C" {
    void* JackTripAUViewFactoryClass() {
        return [JackTripAUViewFactory class];
    }
}

// Make sure the class is available at runtime
__attribute__((constructor))
static void registerViewFactory() {
    // This ensures the class is loaded and available
    [JackTripAUViewFactory class];
}

@implementation JackTripAUViewFactory

- (unsigned)interfaceVersion {
    return 0;
}

- (NSString *)description {
    return @"JackTrip Audio Unit Custom View";
}

- (NSView *)uiViewForAudioUnit:(AudioUnit)inAudioUnit withSize:(NSSize)inPreferredSize {
    // Create our custom view with the preferred size
    NSRect frame = NSMakeRect(0, 0, 400, 200); // Fixed size matching VST3 UI
    
    JackTripAUView* view = [[JackTripAUView alloc] initWithFrame:frame audioUnit:inAudioUnit];
    
    return [view autorelease];
}

@end 