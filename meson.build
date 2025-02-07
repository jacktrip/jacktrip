project('jacktrip', ['cpp','c'],
		default_options: ['cpp_std=c++17','warning_level=2','optimization=2'])

if get_option('profile') == 'development'
	application_id = 'org.jacktrip.JackTrip.Devel'
	name_suffix = ' (Development Snapshot)'
else
	application_id = 'org.jacktrip.JackTrip'
	name_suffix = ''
endif

qt_version = get_option('qtversion')
if qt_version == ''
	qt_dep = dependency('qt6', modules: ['Core'], required: false)
	if qt_dep.found()
		qt_version = '6'
	else
		qt_version = '5'
	endif
endif
if qt_version == '5' and get_option('nogui') == false and get_option('novs') == false
	error('JackTrip Virtual Studio requires Qt 6.2 or later.')
endif
qt = import('qt' + qt_version)

cmake = import('cmake')

compiler = meson.get_compiler('cpp')

defines = ['-DWAIRTOHUB']
c_defines = []
incdirs = []

if get_option('debug') == true
	defines += ['-D_DEBUG']
	c_defines += ['-D_DEBUG']
else
	defines += ['-DNDEBUG', '-DQT_NO_DEBUG']
	c_defines += ['-DNDEBUG', '-DQT_NO_DEBUG']
endif

build_info = get_option('buildinfo')
git = find_program('git', required: false)
if build_info == '' and git.found()
	git_tags_cmd = run_command(git, 'describe', '--tags', check: false)
	git_hash_cmd = run_command(git, 'rev-parse', '--short', 'HEAD', check: false)
	if git_tags_cmd.returncode() == 0 and git_hash_cmd.returncode() == 0
		git_tags = git_tags_cmd.stdout().strip()
		git_hash = git_hash_cmd.stdout().strip()
		build_info = git_tags + '-' + git_hash
	endif
endif
if build_info != ''
	message('Build info: ' + build_info)
	defines += ['-DJACKTRIP_BUILD_INFO=' + build_info]
endif

src = [	'src/DataProtocol.cpp',
	'src/JackTrip.cpp',
	'src/ProcessPlugin.cpp',
	'src/AudioSocket.cpp',
	'src/AudioTester.cpp',
	'src/jacktrip_globals.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/RingBuffer.cpp',
	'src/JitterBuffer.cpp',
	'src/Regulator.cpp',
	'src/SampleRateConverter.cpp',
	'src/Settings.cpp',
	'src/SocketClient.cpp',
	'src/SocketServer.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Meter.cpp',
	'src/Monitor.cpp',
	'src/Volume.cpp',
	'src/Tone.cpp',
	'src/StereoToMono.cpp',
	'src/Reverb.cpp',
	'src/main.cpp',
	'src/SslServer.cpp',
	'src/Auth.cpp']

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/ProcessPlugin.h',
	'src/AudioSocket.h',
	'src/Meter.h',
	'src/Monitor.h',
	'src/StereoToMono.h',
	'src/Volume.h',
	'src/Tone.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Regulator.h',
	'src/Settings.h',
	'src/SocketClient.h',
	'src/SocketServer.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/Auth.h',
	'src/SslServer.h']

if get_option('nooscpp') == true
	defines += '-DNO_OSCPP'
	c_defines += '-DNO_OSCPP'
else
	incdirs += include_directories('externals/oscpp', is_system: true)
	incdirs += include_directories('externals/oscpp/include', is_system: true)
	src += ['src/OscServer.cpp']
	moc_h += ['src/OscServer.h']
endif

ui_h = []
qres = []
deps = [dependency('threads')]
link_args = []

subdir('win')
subdir('linux')

jack_dep = dependency('jack', required: get_option('jack'))
if not jack_dep.found()
	if get_option('weakjack') == true
		error('unable to find jack and weakjack requested')
	endif
	defines += '-DNO_JACK'
	c_defines += '-DNO_JACK'
else
	src += 	['src/JackAudioInterface.cpp',
		'src/JMess.cpp',
		'src/Patcher.cpp']
	moc_h += ['src/Patcher.h']
	if get_option('weakjack') == true
		incdirs += include_directories('externals/weakjack', is_system: true)
		src += 'externals/weakjack/weak_libjack.c'
		defines += '-DUSE_WEAK_JACK'
		c_defines += '-DUSE_WEAK_JACK'
		deps += jack_dep.partial_dependency(compile_args: true, includes: true)
		deps += compiler.find_library('dl', required : false)
	else
		deps += jack_dep
	endif
endif

qmake = ''
qt_core_deps = []
if qt_version == '5'
	qmake = find_program('qmake', required: true)
	qt_core_deps = dependency('qt5', modules: ['Core', 'Network'], include_type: 'system')
else
	qmake = find_program('qmake6', required: true)
	qt_core_deps = dependency('qt6', modules: ['Core', 'Network'], include_type: 'system')
endif
deps += qt_core_deps

if get_option('nogui') == true or (get_option('noclassic') == true and get_option('novs') == true)
	# command line only
	defines += '-DNO_GUI'
else
	# include vs and/or classic gui
	if qt_version == '5'
		deps += dependency('qt5', modules: ['Gui', 'Widgets'], include_type: 'system')
	else
		deps += dependency('qt6', modules: ['Gui', 'Widgets'], include_type: 'system')
	endif
	qres = ['src/images/images.qrc']
	src += 'src/UserInterface.cpp'

	if get_option('noclassic') == true
		defines += '-DNO_CLASSIC'
	else
		# support classic mode
		src += [
			'src/gui/qjacktrip.cpp',
			'src/gui/about.cpp',
			'src/gui/messageDialog.cpp',
			'src/gui/textbuf.cpp',
			'src/gui/vuMeter.cpp'
		]
		moc_h += [
			'src/gui/about.h',
			'src/gui/qjacktrip.h',
			'src/gui/messageDialog.h',
			'src/gui/textbuf.h',
			'src/gui/vuMeter.h'
		]
		ui_h += [
			'src/gui/qjacktrip.ui',
			'src/gui/messageDialog.ui',
			'src/gui/about.ui'
		]
	endif

	if get_option('novs') == true
		defines += '-DNO_VS'
	else
		src += [
			'src/vs/virtualstudio.cpp',
			'src/vs/vsAuth.cpp',
			'src/vs/vsApi.cpp',
			'src/vs/vsDeviceCodeFlow.cpp',
			'src/vs/vsDeeplink.cpp',
			'src/vs/vsDevice.cpp',
			'src/vs/vsAudio.cpp',
			'src/vs/vsServerInfo.cpp',
			'src/vs/vsQuickView.cpp',
			'src/vs/vsWebSocket.cpp',
			'src/vs/vsPermissions.cpp',
			'src/vs/vsPinger.cpp',
			'src/vs/vsPing.cpp',
			'src/vs/WebSocketTransport.cpp'
		]
		moc_h += [
			'src/vs/virtualstudio.h',
			'src/vs/vsApi.h',
			'src/vs/vsAuth.h',
			'src/vs/vsDeviceCodeFlow.h',
			'src/vs/vsDeeplink.h',
			'src/vs/vsDevice.h',
			'src/vs/vsAudio.h',
			'src/vs/vsServerInfo.h',
			'src/vs/vsQuickView.h',
			'src/vs/vsWebSocket.h',
			'src/vs/vsPermissions.h',
			'src/vs/vsPinger.h',
			'src/vs/vsPing.h',
			'src/vs/vsQmlClipboard.h',
			'src/vs/JTApplication.h',
			'src/vs/WebSocketTransport.h'
		]

		if host_machine.system() == 'darwin'
			moc_h += ['src/vs/vsMacPermissions.h']
		endif

		if get_option('vsftux') == true or get_option('noclassic') == true
			defines += '-DVS_FTUX'
		endif

		deps += dependency('qt6', modules: ['Core5Compat', 'Quick', 'QuickControls2', 'Qml', 'ShaderTools', 'Svg', 'WebSockets', 'WebEngineCore', 'WebEngineQuick', 'WebChannel'], include_type: 'system')
		qres += ['src/vs/vs.qrc']
	endif

	if get_option('noupdater') == true or host_machine.system() == 'linux'
		defines += '-DNO_UPDATER'
	else
		src += [
			'src/dblsqd/feed.cpp',
			'src/dblsqd/release.cpp',
			'src/dblsqd/semver.cpp',
			'src/dblsqd/update_dialog.cpp'
		]
		moc_h += [
			'src/dblsqd/feed.h',
			'src/dblsqd/update_dialog.h'
		]
		ui_h += ['src/dblsqd/update_dialog.ui']
	endif

	if get_option('nofeedback') == true
		defines += '-DNO_FEEDBACK'
	else
		src += [ 'src/Analyzer.cpp' ]
		moc_h += [ 'src/Analyzer.h' ]
	endif
endif

static_deps = []
static_src = []
static_link_args = []
if get_option('default_library') == 'static'
	# use qmake to get paths for qt libraries and plugins
	# seems like qt module should have a method for this, but it doesn't
	qt_libdir = run_command(qmake, '-query', 'QT_INSTALL_LIBS', check : true).stdout().strip()
	qt_plugindir = run_command(qmake, '-query', 'QT_INSTALL_PLUGINS', check : true).stdout().strip()
	if qt_version == '6'
		# qt6 requires "Bundled*" modules for linking
		static_deps += compiler.find_library('Qt6BundledLibpng', required : true, dirs : [qt_libdir])
		static_deps += compiler.find_library('Qt6BundledPcre2', required : true, dirs : [qt_libdir])
		static_deps += compiler.find_library('Qt6BundledHarfbuzz', required : true, dirs : [qt_libdir])
		zlib_dep = compiler.find_library('Qt6BundledZLIB', required : false, dirs : [qt_libdir])
		if zlib_dep.found()
			static_deps += zlib_dep
		endif
		dbus_dep = compiler.find_library('Qt6DBus', required : false, dirs : [qt_libdir])
		if dbus_dep.found()
			static_deps += dbus_dep
		endif
	else
		static_deps += compiler.find_library('qtpcre2', required : true, dirs : [qt_libdir])
	endif
	if (host_machine.system() == 'linux')
		# linux static
		static_deps += compiler.find_library('ssl', required : true, dirs : [qt_libdir])
		static_deps += compiler.find_library('crypto', required : true, dirs : [qt_libdir])
		static_deps += compiler.find_library('dl', required : true)
		static_deps += compiler.find_library('glib-2.0', required : true)
		if qt_version == '6'
			# we need a Q_IMPORT_LIBRARY for the openssl backend on linux
			static_deps += compiler.find_library('dbus-1', required : true)
			static_deps += compiler.find_library('qopensslbackend', required : true, dirs : [qt_plugindir+'/tls'])
			static_src += ['src/QtStaticPlugins.cpp']
		endif
	else
		if (host_machine.system() == 'windows')
			# windows static
			static_deps += compiler.find_library('bcrypt', required : true)
			static_deps += compiler.find_library('winmm', required : true)
			static_deps += compiler.find_library('Crypt32', required : true)
			if qt_version == '6'
				static_deps += compiler.find_library('Authz', required : true)
			endif
		else
			# mac static
			# this approach fails for universal builds, so we have to just append to link_args
			#static_deps += dependency('CoreServices', required : true)
			static_link_args += ['-framework', 'CoreServices']
			static_link_args += ['-framework', 'CFNetwork']
			static_link_args += ['-framework', 'AppKit']
			static_link_args += ['-framework', 'IOKit']
			static_link_args += ['-framework', 'Security']
			static_link_args += ['-framework', 'GSS']
			static_link_args += ['-framework', 'SystemConfiguration']
			static_link_args += ['-framework', 'UniformTypeIdentifiers']
			static_link_args += '-lresolv'
			static_deps += dependency('zlib', required : true)
		endif
	endif
	deps += static_deps
	src += static_src
	link_args += static_link_args
endif

# QT_OPENSOURCE should only be defined for open source Qt distribution
if get_option('qtedition') != 'commercial'
	defines += '-DQT_OPENSOURCE'
endif

rtaudio_dep = dependency('rtaudio', required: get_option('rtaudio'))
if rtaudio_dep.found() == true
	defines += '-DRT_AUDIO'
	src += 'src/RtAudioInterface.cpp'
	deps += rtaudio_dep
endif

if rtaudio_dep.found() == false and jack_dep.found() == false
	error('''
	JackTrip requires at least one available audio backend. Install the
	appropriate library or enable the appropriate backends using meson
	configure.''')
endif

found_libsamplerate = false
if get_option('libsamplerate').allowed()
	libsamplerate_dep = dependency('samplerate', required: false)
	if libsamplerate_dep.found()
		found_libsamplerate = true
	else
		opt_var = cmake.subproject_options()
		if get_option('buildtype') == 'release'
			opt_var.add_cmake_defines({'CMAKE_BUILD_TYPE': 'Release'})
		else
			opt_var.add_cmake_defines({'CMAKE_BUILD_TYPE': 'Debug'})
		endif
		opt_var.add_cmake_defines({'CMAKE_POSITION_INDEPENDENT_CODE': 'ON'})
		libsamplerate_subproject = cmake.subproject('libsamplerate', options: opt_var)
		libsamplerate_dep = libsamplerate_subproject.dependency('samplerate')
		found_libsamplerate = libsamplerate_dep.found()
		if not found_libsamplerate and not get_option('libsamplerate').auto()
			error('failed to configure libsamplerate')
		endif
	endif
endif
if found_libsamplerate
	defines += '-DHAVE_LIBSAMPLERATE'
	deps += libsamplerate_dep
endif

if host_machine.system() == 'darwin'
	src += ['src/NoNap.mm']
	# Adding CoreAudio here is a workaround and should be removed
	# when https://github.com/thestk/rtaudio/issues/302 is fixed
	# and arrived in all common package managers
	# Check at 2022-07-30
	apple_dep = dependency('appleframeworks', modules : ['foundation','coreaudio'])
	deps += apple_dep
	add_languages('objcpp')
endif

if host_machine.system() == 'darwin' and get_option('novs') == false and get_option('nogui') == false
	src += ['src/vs/vsMacPermissions.mm']
	apple_av_dep = dependency('appleframeworks', modules : ['avfoundation', 'webkit'])
	deps += apple_av_dep
endif

qres_files = []
if qres.length() > 0
	qres_files = qt.compile_resources(sources: qres)
endif
moc_files = qt.compile_moc(headers: moc_h, extra_args: defines)
ui_files = []
if ui_h.length() > 0
	ui_files = qt.compile_ui(sources: ui_h)
endif

jacktrip = executable('jacktrip', src, qres_files, ui_files, moc_files, include_directories: incdirs, dependencies: deps, link_args: link_args, c_args: c_defines, cpp_args: defines, install: true )

vst_sdkdir = get_option('vst-sdkdir')
if vst_sdkdir != ''
	# adapted from https://github.com/centricular/gstreamer-vst3
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

	vst_sources = ['src/vst3/JackTripVSTController.cpp', 'src/vst3/JackTripVSTEntry.cpp', 'src/vst3/JackTripVSTProcessor.cpp']

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
		vst_link_args += ['-framework', 'CoreServices']
		vst_link_args += ['-framework', 'CFNetwork']
		vst_link_args += ['-framework', 'AppKit']
		vst_link_args += ['-framework', 'IOKit']
		vst_link_args += ['-framework', 'Security']
		vst_link_args += ['-framework', 'GSS']
		vst_link_args += ['-framework', 'SystemConfiguration']
		vst_deps += dependency('zlib', required : true)
		if (qt_version == '5')
			vst_link_args += '@0@/libQt5Core.a'.format(vst_libdir)
			vst_link_args += '@0@/libQt5Network.a'.format(vst_libdir)
		else
			vst_link_args += '@0@/libQt6Core.a'.format(vst_libdir)
			vst_link_args += '@0@/libQt6Network.a'.format(vst_libdir)
			vst_link_args += '@0@/libQt6BundledPcre2.a'.format(vst_libdir)
		endif
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

	audio_socket_moc_h = ['src/AudioSocket.h', 'src/SocketClient.h', 'src/ProcessPlugin.h']
	audio_socket_sources = qt.compile_moc(headers: audio_socket_moc_h, extra_args: defines)
	audio_socket_sources += [
		'src/AudioSocket.cpp',
		'src/SocketClient.cpp',
		'src/ProcessPlugin.cpp',
		'src/jacktrip_globals.cpp'
	]
	audio_socket_test = executable('audio_socket_tests',
		['tests/audio_socket_test.cpp'] + audio_socket_sources,
		cpp_args : defines,
		dependencies : vst_deps,
		include_directories: vst_incdirs + include_directories('src/'),
		link_args: vst_link_args
	)

	vst3 = shared_module('JackTrip',
		vst_sources, audio_socket_sources,
		name_prefix: '',
		name_suffix: 'vst3',
		cpp_args : defines,
		dependencies : vst_deps,
		include_directories: vst_incdirs,
		link_args: vst_link_args,
		cpp_args: defines
	)
endif

help2man = find_program('help2man', required: false)
if (host_machine.system() == 'linux')
	if help2man.found()
		gzip = find_program('gzip', required: false)
		help2man_opts = [
			'--name="high-quality system for audio network performances"',
			'--no-info',
			'--section=1']
		manfile = custom_target('jacktrip.1',
			output: 'jacktrip.1',
			command: [help2man, help2man_opts, '--output=@OUTPUT@', jacktrip],
			install: not gzip.found(),
			install_dir: get_option('mandir') / 'man1')
		if gzip.found()
			custom_target('jacktrip.1.gz',
				input: manfile,
				output: 'jacktrip.1.gz',
				command: [gzip, '-k', '-f', '@INPUT@'],
				install: true,
				install_dir: get_option('mandir') / 'man1')
		endif
	endif
endif

summary({'JACK': jack_dep.found(),
	'Weak JACK Linking': get_option('weakjack'),
	'RtAudio': rtaudio_dep.found()}, bool_yn: true, section: 'Audio Backends')

summary({'Application ID': application_id,
	'GUI': not get_option('nogui'),
	'WAIR': get_option('wair'),
	'Sample rate conversions': found_libsamplerate,
	'Manpage': help2man.found()}, bool_yn: true, section: 'Configuration')
