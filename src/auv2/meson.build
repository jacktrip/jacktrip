# JackTrip Audio Unit Plugin Build Configuration

# Audio Unit sources
auv2_sources = [
    'JackTripAU.cpp',
    'JackTripAUView.mm',
    'JackTripAUViewFactory.mm',
    'JackTripAUCocoaUI.mm'
]

# Required AudioSocket sources (shared with VST3)
audio_socket_moc_h = ['../AudioSocket.h', '../SocketClient.h', '../ProcessPlugin.h']
audio_socket_sources = qt.compile_moc(headers: audio_socket_moc_h, extra_args: defines)
audio_socket_sources += [
    '../AudioSocket.cpp',
    '../AudioBridgeProcessor.cpp',
    '../SocketClient.cpp', 
    '../ProcessPlugin.cpp',
    '../jacktrip_globals.cpp'
]

# Apple frameworks required for Audio Units
auv2_frameworks = dependency('appleframeworks', modules: [
    'Foundation',
    'AudioToolbox', 
    'AudioUnit',
    'CoreServices',
    'Cocoa'
])

# Dependencies
auv2_deps = [qt_core_deps, static_deps, auv2_frameworks, audiounitsdk_dep]

# Add libsamplerate if available
if found_libsamplerate
    auv2_deps += libsamplerate_dep
endif

# Include directories
auv2_incdirs = include_directories('.', '../')

# Build the Audio Unit bundle
jacktrip_auv2 = shared_module('JackTrip',
    auv2_sources, audio_socket_sources,
    name_prefix: '',
    name_suffix: 'auv2',
    dependencies: auv2_deps,
    include_directories: auv2_incdirs,
    link_args: static_link_args
)

message('JackTrip Audio Unit v2 will be built')
