# JackTrip VST3 Plugin

This directory contains the VST3 implementation of the JackTrip audio bridge plugin for cross-platform use in digital audio workstations.

## Overview

The JackTrip VST3 plugin provides a bridge between your DAW and remote JackTrip instances, enabling high-quality, low-latency audio exchange over the internet. It uses the AudioBridgeProcessor class to communicate with the JackTrip application running on the same machine.

## Features

- **Audio Processing**: Stereo audio input/output with configurable gain controls
- **Network Audio**: Exchange audio with remote JackTrip instances via local socket
- **Parameter Control**: Send gain, output mix, output gain, bypass, and connection status
- **Real-time Audio**: Low-latency audio processing suitable for live performance
- **Cross-platform**: Works on Windows, macOS, and Linux
- **Interface**: Custom VST3 GUI with visual controls and status indicators

## Files

- `JackTripPlugin.h/.cpp` - Main VST3 plugin implementation
- `JackTripController.h/.cpp` - Parameter handling and UI communication
- `JackTripProcessor.h/.cpp` - Audio processing engine
- `version.h` - Plugin version information
- `resources/JackTripEditor.uidesc` - VSTGUI interface definition
- `../images/` - Interface graphics (shared with Audio Unit)
- `meson.build` - Build configuration

## Building

The VST3 plugin is built automatically when building JackTrip with static Qt:

```bash
meson setup -Ddefault_library=static -Dnogui=true --buildtype release buildstatic
meson compile -C buildstatic
```

## Usage

1. Launch JackTrip in your desired mode
2. Insert the "JackTrip Audio Bridge" VST3 plugin in your DAW
3. Configure the plugin parameters:
   - **Send Gain**: Controls level of audio sent to remote (-60 to +6 dB)
   - **Output Mix**: Blends received audio (0%) with input passthrough (100%)
   - **Output Gain**: Master output level (-60 to +6 dB)
   - **Bypass**: Bypasses all processing
   - **Connected**: Shows connection status (read-only)

The plugin will automatically attempt to connect to the JackTrip application via local socket.

## Architecture

The VST3 plugin follows the Steinberg VST3 SDK architecture:

- `JackTripPlugin` - Main plugin class implementing `IComponent` and `IAudioProcessor`
- `JackTripController` - Parameter controller implementing `IEditController`
- `JackTripProcessor` - Audio processing core with AudioSocket integration
- Uses VSTGUI for cross-platform user interface
- Reuses AudioBridgeProcessor implementation from main application

## Automation

All parameters except "Connected" support DAW automation. The plugin reports parameter changes back to the host for automation recording.

## Requirements

- **VST3 compatible host** (most modern DAWs)
- **JackTrip application** running on the same machine
- **Operating System**: Windows 10+, macOS 12+, or Linux

## License

Copyright (c) 2024-2025 JackTrip Labs, Inc.
Licensed under the MIT License.
