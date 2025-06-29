# JackTrip Audio Unit Plugin (v3)

This directory contains the Audio Unit (AUv3) implementation of the JackTrip Audio Bridge plugin for macOS.

## Overview

The JackTrip Audio Bridge plugin uses the AudioBridgeProcessor class to exchange audio with the JackTrip application via local socket connections. This implementation is based on the working AUv2 plugin but adapted for the modern AUv3 architecture. It was created by asking AI to create a v3 version of the plugin. Although it compiles, it is not being recognized as a valid app extension. Apple doesn't provide good documentation or tools for debugging this.

## Features

- **Audio Processing**: Stereo audio input/output with configurable gain controls
- **Network Audio**: Exchange audio with remote JackTrip instances via local socket
- **Parameter Control**: Send gain, output mix, output gain, and connection status
- **Real-time Audio**: Low-latency audio processing suitable for live performance
- **Native UI**: Cocoa-based user interface with sliders for all parameters
- **C++ Core**: Maximum use of C++ for audio processing logic with minimal Objective-C wrapper

## Architecture

The AUv3 plugin follows a hybrid C++/Objective-C architecture:

- `JackTripAU` - Main AUAudioUnit subclass (Objective-C wrapper)
- `JackTripAUImpl` - C++ implementation class containing all audio processing logic
- `AudioBridgeProcessor` - Shared C++ audio processing engine (same as AUv2/VST3)
- `JackTripAUViewController` - Native Cocoa UI controller
- `JackTripAUFactory` - Audio Unit component registration

## Building

The AUv3 plugin is built automatically when building JackTrip with static Qt on macOS:

```bash
meson setup -Ddefault_library=static -Dnogui=true --buildtype release buildstatic
meson compile -C buildstatic
```

Use the `macos/assemble_app.sh` script to build the installation bundles.

After installation, restart your DAW to detect the new plugin.

## Usage

1. Launch JackTrip in your desired mode
2. Insert the "JackTrip Audio Bridge" AUv3 plugin in your DAW
3. Configure the plugin parameters:
   - **Send Gain**: Controls level of audio sent to remote (0.0 - 1.0)
   - **Output Mix**: Blends received audio (0%) with input passthrough (100%)
   - **Output Gain**: Master output level (0.0 - 1.0)
   - **Connected**: Shows connection status (read-only indicator)

The plugin will automatically attempt to connect to the JackTrip application via local socket.

## Implementation Details

### C++ Core Logic
- All audio processing is handled in C++ via `JackTripAUImpl` class
- Uses the same `AudioBridgeProcessor` as other plugin formats for consistency
- Parameter management and audio buffer handling in C++
- Memory management with RAII and smart pointers

### Objective-C Wrapper
- Minimal Objective-C code for AUv3 framework integration
- Property management and Audio Unit lifecycle
- Block-based render callback with proper memory management
- Parameter tree creation and observer setup

### Audio Processing
- Stereo input/output processing
- Dynamic input buffer allocation in render callback
- Proper cleanup of temporary buffers
- Connection state monitoring and UI updates

## Requirements

- macOS 12 or later
- Audio Unit v3 compatible host application
- JackTrip application running on the same machine

## Differences from AUv2

- Uses modern AUAudioUnit base class instead of AudioUnitSDK
- Block-based render callback instead of C-style callback
- AUParameterTree for parameter management
- Native AUv3 extension architecture
- Integrated UI support via AUViewController

## References

* [Audio Unit v3 Plugins](https://developer.apple.com/documentation/audiotoolbox/audio-unit-v3-plug-ins)
* [AUAudioUnit Class Reference](https://developer.apple.com/documentation/audiotoolbox/auaudiounit)
* [Core Audio Programming Guide](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/Introduction/Introduction.html)

## License

Copyright (c) 2024-2025 JackTrip Labs, Inc.
Licensed under the MIT License. 