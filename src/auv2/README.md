# JackTrip Audio Unit Plugin (v2)

This directory contains the Audio Unit (AUv2) implementation of the JackTrip Audio Bridge plugin for macOS.

## Overview

The JackTrip Audio Bridge plugin uses the AudioBridgeProcessor class to exchange audio with the JackTrip application.

## Features

- **Audio Processing**: Stereo audio input/output with configurable gain controls
- **Network Audio**: Exchange audio with remote JackTrip instances via local socket
- **Parameter Control**: Send gain, output mix, output gain, bypass, and connection status
- **Real-time Audio**: Low-latency audio processing suitable for live performance
- **Interface**: Native Cocoa UI with the same controls as the VST3 version

## Building

The AU plugin is built automatically when building JackTrip with static Qt on macOS:

```bash
meson setup -Ddefault_library=static -Dnogui=true --buildtype release buildstatic
meson compile -C buildstatic
```

Use the `macos/assemble_app.sh` to build the installation bundles.

## Usage

1. Launch JackTrip in your desired mode
2. Insert the "JackTrip Audio Bridge" AU plugin in your DAW
3. Configure the plugin parameters:
   - **Send Gain**: Controls level of audio sent to remote
   - **Output Mix**: Blends received audio (0%) with input passthrough (100%)
   - **Output Gain**: Master output level
   - **Bypass**: Bypasses all processing
   - **Connected**: Shows connection status (read-only)

The plugin will automatically attempt to connect to the JackTrip application via local socket.

## Requirements

- macOS 12 or later
- Audio Unit compatible host application
- Host application must have network access via entitlements
- JackTrip application running on the same machine

## References

* [Developer Guide for AUv2](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html)
* [Audio Unit v2 C API](https://developer.apple.com/documentation/audiotoolbox/audio-unit-v2-c-api)
* [Building AudioUnits with modern Mac OSX](https://teragonaudio.com/article/Building-AudioUnits-with-modern-Mac-OSX.html)
* [Audio Plugin Dev Notes](https://gist.github.com/olilarkin/8f378d212b0a59944d84f9f47061d70f?utm_source=chatgpt.com)

## License

Copyright (c) 2024-2025 JackTrip Labs, Inc.
Licensed under the MIT License. 