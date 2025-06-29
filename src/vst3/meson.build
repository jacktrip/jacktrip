# JackTrip VST3 Plugin Build Configuration

# adapted from https://github.com/centricular/gstreamer-vst3
vst_sdkdir = get_option('vst-sdkdir')
vst_includedir = '@0@/public.sdk/source'.format(vst_sdkdir)
vst_pluginterfaces_includedir = '@0@'.format(vst_sdkdir)
vst_incdirs = []
vst_incdirs += include_directories('@0@'.format(vst_includedir), is_system: true)
vst_incdirs += include_directories('@0@'.format(vst_pluginterfaces_includedir), is_system: true)
vst_incdirs += include_directories('@0@/vstgui4'.format(vst_pluginterfaces_includedir), is_system: true)

vst_libdir = get_option('vst-libdir')
if vst_libdir == ''
	vst_libdir = vst_sdkdir + '/lib'
endif
libbase_dep = compiler.find_library('base', required : true, dirs : [vst_libdir])
libsdk_dep = compiler.find_library('sdk', required : true, dirs : [vst_libdir])
libsdk_common_dep = compiler.find_library('sdk_common', required : true, dirs : [vst_libdir])
libvstgui_dep = compiler.find_library('vstgui', required : true, dirs : [vst_libdir])
libvstgui_support_dep = compiler.find_library('vstgui_support', required : true, dirs : [vst_libdir])
libvstgui_uidescription_dep = compiler.find_library('vstgui_uidescription', required : true, dirs : [vst_libdir])
libpluginterfaces_dep = compiler.find_library('pluginterfaces', required : true, dirs : [vst_libdir])
vst_deps = [libbase_dep, libsdk_dep, libsdk_common_dep, libvstgui_dep, libvstgui_uidescription_dep, libvstgui_support_dep, libpluginterfaces_dep]
vst_deps += qt_core_deps

vst_sources = ['JackTripVSTController.cpp', 'JackTripVSTEntry.cpp', 'JackTripVSTProcessor.cpp']

# uncomment for live editor
# vst_sources += ['@0@/vstgui4/vstgui/vstgui_uidescription.cpp'.format(vst_sdkdir), '@0@/vstgui4/vstgui/plugin-bindings/vst3editor.cpp'.format(vst_sdkdir)]
# defines += ['-DVSTGUI_LIVE_EDITING=1']

vst_link_args = []
if (host_machine.system() == 'linux')
	vst_sources += '@0@/main/linuxmain.cpp'.format(vst_includedir)
	vst_deps += static_deps
	vst_sources += static_src
	vst_link_args += static_link_args
	vst_deps += compiler.find_library('xcb-util', required : true)
	vst_deps += compiler.find_library('xcb-cursor', required : true)
	vst_deps += compiler.find_library('xkbcommon-x11', required : true)
	vst_deps += compiler.find_library('xml2', required : true)
	vst_deps += compiler.find_library('cairo', required : true)
	vst_deps += compiler.find_library('pango-1.0', required : true)
	vst_deps += compiler.find_library('pangocairo-1.0', required : true)
	vst_deps += compiler.find_library('expat', required : true)
	vst_deps += compiler.find_library('fontconfig', required : true)
elif (host_machine.system() == 'darwin')
	vst_sources += '@0@/main/macmain.cpp'.format(vst_includedir)
	vst_deps += static_deps
	vst_sources += static_src
	vst_link_args += static_link_args
elif (host_machine.system() == 'windows')
	vst_sources += '@0@/main/dllmain.cpp'.format(vst_includedir)
	vst_deps += static_deps
	vst_sources += static_src
	vst_link_args += static_link_args
	vst_deps += compiler.find_library('bcrypt', required : true)
	vst_deps += compiler.find_library('winmm', required : true)
	vst_deps += compiler.find_library('Crypt32', required : true)
	vst_deps += compiler.find_library('ws2_32', required: true)
	vst_link_args += 'userenv.lib'
	vst_link_args += 'Synchronization.lib'
	vst_link_args += 'Netapi32.lib'
	vst_link_args += 'Version.lib'
	vst_link_args += 'Dwrite.lib'
	vst_link_args += 'Iphlpapi.lib'
	vst_link_args += 'Secur32.lib'
	vst_link_args += 'Winhttp.lib'
	vst_link_args += 'Dnsapi.lib'
	vst_link_args += 'Iphlpapi.lib'
else
	error('Unsupported platform: ' + host_machine.system())
endif

if found_libsamplerate
	vst_deps += libsamplerate_dep
endif

audio_socket_moc_h = ['../AudioSocket.h', '../SocketClient.h', '../ProcessPlugin.h']
audio_socket_sources = qt.compile_moc(headers: audio_socket_moc_h, extra_args: defines)
audio_socket_sources += [
	'../AudioBridgeProcessor.cpp',
	'../AudioSocket.cpp',
	'../SocketClient.cpp',
	'../ProcessPlugin.cpp',
	'../jacktrip_globals.cpp'
]

# Build test executable
audio_socket_test = executable('audio_socket_tests',
	['../../tests/audio_socket_test.cpp'] + audio_socket_sources,
	cpp_args : defines,
	dependencies : vst_deps,
	include_directories: vst_incdirs + include_directories('../'),
	link_args: vst_link_args
)

# Build VST3 plugin
vst3 = shared_module('JackTrip',
	vst_sources, audio_socket_sources,
	name_prefix: '',
	name_suffix: 'vst3',
	cpp_args : defines,
	dependencies : vst_deps,
	include_directories: vst_incdirs,
	link_args: vst_link_args
)

message('JackTrip VST3 plugin will be built') 