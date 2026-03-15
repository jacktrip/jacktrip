## Project Overview

JackTrip is a high-quality audio network performance system for bidirectional, uncompressed audio streaming over the Internet. It's a C++20 desktop application using Qt 6, supporting Linux, macOS and Windows.

## Build Commands

**Build system**: Meson (primary). CMake exists but is legacy/unsupported.

```bash
# Configure (first time)
meson setup builddir

# Build
meson compile -C builddir

# Reconfigure with options
meson configure builddir -Dnogui=true   # CLI-only build
meson configure builddir -Dnovs=true    # without Virtual Studio
meson configure builddir -Dnoclassic=true  # without classic Qt Widgets GUI

# Clean rebuild
meson setup builddir --wipe
```

See `meson_options.txt` for setup options.

## Code Formatting & Linting

- **clang-format** (version 13) enforced in CI on all `src/**/*.{h,cpp}` files
- Style: Google-based, 90-column limit, 4-space indent, Linux brace style
- Run manually: `clang-format -i src/MyFile.cpp`
- **clang-tidy** runs as a separate CI check on PRs
- Pre-commit hook enforces clang-format

## Testing

Tests use Qt Test framework. Test files are in `tests/`:
- `tests/jacktrip_tests.cpp` — threading tests
- `tests/audio_socket_test.cpp` — AudioSocket tests

Test coverage is minimal; the project relies primarily on manual/integration testing.

## Architecture

### Core Design (Mediator Pattern)

**`JackTrip`** (`src/JackTrip.cpp`, ~3000 lines) is the central orchestrator coordinating all subsystems.

**Data flow**:
- Sender: AudioInterface → RingBuffer → PacketHeader → UdpDataProtocol → Network
- Receiver: Network → UdpDataProtocol → JitterBuffer → RingBuffer → Effects → AudioInterface

### Threading Model

Four threads in operation:
1. **Audio thread** — real-time priority, driven by audio backend callbacks
2. **Network sender thread** — `DataProtocol` subclass
3. **Network receiver thread** — `DataProtocol` subclass
4. **GUI thread** — Qt event loop

### Key Components

| Component | Files | Purpose |
|-----------|-------|---------|
| Audio backends | `AudioInterface.cpp`, `JackAudioInterface.cpp`, `RtAudioInterface.cpp` | Abstract audio I/O (JACK or RtAudio) |
| Networking | `DataProtocol.cpp`, `UdpDataProtocol.cpp` | UDP-based audio transport |
| Buffering | `RingBuffer.cpp`, `JitterBuffer.cpp`, `Regulator.cpp` | Lock-free audio buffering and jitter management |
| Effects | `Compressor.cpp`, `Limiter.cpp`, `Reverb.cpp`, `Volume.cpp`, etc. | Audio processing via `ProcessPlugin` base class |
| Hub server | `UdpHubListener.cpp`, `JackTripWorker.cpp` | Multi-client server mode |
| Settings | `Settings.cpp` | Command-line parsing and configuration |
| Entry point | `main.cpp` | GUI/CLI dispatch |

### GUI Modes

- **Virtual Studio** (`src/vs/`) — Modern Qt QML + WebEngine GUI, requires Qt 6.2+. Main class: `VirtualStudio`
- **Classic** (`src/gui/`) — Traditional Qt Widgets GUI. Main class: `QJackTrip`
- **CLI** — Headless mode when built with `-Dnogui=true`

### Extending Audio Effects

Subclass `ProcessPlugin` and implement the `compute` method. Effects are chained in the audio processing pipeline by `JackTrip`.

### Platform-Specific Code

- macOS: `NoNap.mm`, `vsMacPermissions.mm`, CoreAudio integration, DYLD injection protection
- Windows: RtAudio primary backend, DLL management in `win/`
- Linux: Strong JACK support, Flatpak packaging in `linux/`

## Adding Icons to the UI

Prefer sourcing icons from [heroicons.com](https://heroicons.com). Download the SVG and follow the steps below.

### Steps

1. **Add the SVG file** to `src/vs/` (or `src/vs/flags/` for country flags)
2. **Register it in `src/vs/vs.qrc`** — add a `<file>youricon.svg</file>` entry inside the `<qresource prefix="vs">` block
3. **Reference in QML** — use `icon.source: "youricon.svg"` or the `AppIcon` wrapper component for theme-aware rendering:
   ```qml
   AppIcon {
       width: 24 * virtualstudio.uiScale
       height: 24 * virtualstudio.uiScale
       icon.source: "youricon.svg"
       color: textColour
   }
   ```
4. **Rebuild** — Meson recompiles the QRC, embedding the icon into the executable

### QRC files

- `src/vs/vs.qrc` — Virtual Studio icons, QML files, fonts (prefix: `vs`)
- `src/images/images.qrc` — application window icons (prefix: `images`)

### Icon conventions

- **Format**: SVG (all UI icons are SVG; PNG only for app icons and branding)
- **Color**: icons are colored at runtime via `icon.color` in QML, so use a single fill color in the SVG (the `AppIcon` component in `src/vs/AppIcon.qml` handles dark/light theme defaults automatically)
- **Sizing**: controlled by the parent QML element and `virtualstudio.uiScale`, not baked into the SVG

## Code Style Conventions

- **Classes**: PascalCase (`JackTrip`, `AudioInterface`)
- **Methods**: camelCase (`startProcess`, `computeProcessFromNetwork`)
- **Member variables**: `m` prefix (`mAudioInterface`, `mDataProtocol`)
- Heavy use of Qt signals/slots, `QObject`, `Q_PROPERTY`
- Pointers: left-aligned (`int* ptr`, not `int *ptr`)
