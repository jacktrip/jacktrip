# Audio Unit v3 sources
auv3_sources = [
    'JackTripAU.mm',
    'JackTripAUFactory.mm',
    'JackTripAUViewController.mm'
]

# Required AudioBridgeProcessor sources
audio_bridge_sources = [
    '../AudioBridgeProcessor.cpp',
    '../AudioSocket.cpp',
    '../SocketClient.cpp', 
    '../ProcessPlugin.cpp',
    '../jacktrip_globals.cpp'
]

# Qt MOC for AudioSocket headers
audio_socket_moc_h = ['../AudioSocket.h', '../SocketClient.h', '../ProcessPlugin.h']
audio_socket_sources = qt.compile_moc(headers: audio_socket_moc_h, extra_args: defines)

# Apple frameworks required for Audio Units
auv3_frameworks = dependency('appleframeworks', modules: [
    'Foundation',
    'AudioToolbox', 
    'AudioUnit',
    'CoreAudioKit',
    'AVFoundation',
    'CoreAudio',
    'Cocoa',
])

# Dependencies
auv3_deps = [qt_core_deps, auv3_frameworks]
if get_option('default_library') == 'static'
    auv3_deps += static_deps
endif

# Add libsamplerate if available
if found_libsamplerate
    auv3_deps += libsamplerate_dep
endif

# Include directories
auv3_incdirs = include_directories('.', '../')

# Build the Audio Unit bundle
jacktrip_auv3 = shared_module('JackTrip',
    auv3_sources, audio_bridge_sources, audio_socket_sources,
    name_prefix: '',
    name_suffix: 'auv3',  # Audio Unit extension
    cpp_args: defines + ['-std=c++17'],
    objcpp_args: defines + ['-fobjc-arc', '-std=c++17'],
    dependencies: auv3_deps,
    include_directories: auv3_incdirs,
    link_args: static_link_args,
    cpp_args: defines
)

message('JackTrip Audio Unit v3 will be built')
