project('jacktrip', ['cpp','c'],
		default_options: ['cpp_std=c++17','warning_level=2','optimization=2'])

if get_option('profile') == 'development'
  application_id = 'org.jacktrip.JackTrip.Devel'
  name_suffix = ' (Development Snapshot)'
else
  application_id = 'org.jacktrip.JackTrip'
  name_suffix = ''
endif

if get_option('qtversion') == '5'
  qt = import('qt5')
else
  qt = import('qt6')
endif

cmake = import('cmake')

compiler = meson.get_compiler('cpp')

defines = ['-DWAIRTOHUB']
c_defines = []
incdirs = []

src = [	'src/DataProtocol.cpp',
	'src/JackTrip.cpp',
	'src/ProcessPlugin.cpp',
	'src/AudioTester.cpp',
	'src/jacktrip_globals.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/RingBuffer.cpp',
	'src/JitterBuffer.cpp',
	'src/Regulator.cpp',
	'src/Settings.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Meter.cpp',
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
	'src/Meter.h',
	'src/StereoToMono.h',
	'src/Volume.h',
	'src/Tone.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Regulator.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/Auth.h',
	'src/SslServer.h']

ui_h = []
qres = []
deps = [dependency('threads')]

subdir('win')
subdir('linux')

jack_dep = dependency('jack', required: get_option('jack'))
if not jack_dep.found()
	defines += '-DNO_JACK'
else
	src += 	['src/JackAudioInterface.cpp',
		'src/JMess.cpp',
		'src/Patcher.cpp']
	moc_h += ['src/Patcher.h']
	if get_option('weakjack') == true
		incdirs += include_directories('externals/weakjack')
		src += 'externals/weakjack/weak_libjack.c'
		defines += '-DUSE_WEAK_JACK'
		c_defines += '-DUSE_WEAK_JACK'
		deps += jack_dep.partial_dependency(includes: true)
		deps += compiler.find_library('dl', required : false)
	else
		deps += jack_dep
	endif
endif

if get_option('nogui') == true
	defines += '-DNO_GUI'
	if get_option('qtversion') == '5'
		qt_dep = dependency('qt5', modules: ['Core', 'Network'], include_type: 'system')
	else
		qt_dep = dependency('qt6', modules: ['Core', 'Network'], include_type: 'system')
	endif
else
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

	if get_option('novs') == true
		defines += '-DNO_VS'
		if get_option('qtversion') == '5'
			qt_dep = dependency('qt5', modules: ['Core', 'Gui', 'Network', 'Widgets'], include_type: 'system')
		else
			qt_dep = dependency('qt6', modules: ['Core', 'Gui', 'Network', 'Widgets'], include_type: 'system')
		endif
		qres = ['src/gui/qjacktrip_novs.qrc']
	else
		src += [
			'src/gui/virtualstudio.cpp',
			'src/gui/vsInit.cpp',
			'src/gui/vsDevice.cpp',
			'src/gui/vsAudioInterface.cpp',
			'src/gui/vsServerInfo.cpp',
			'src/gui/vsQuickView.cpp',
			'src/gui/vsWebSocket.cpp',
			'src/gui/vsUrlHandler.cpp',
			'src/gui/vsPermissions.cpp',
			'src/gui/vsPinger.cpp',
			'src/gui/vsPing.cpp'
		]
		moc_h += [
			'src/gui/virtualstudio.h',
			'src/gui/vsInit.h',
			'src/gui/vsDevice.h',
			'src/gui/vsAudioInterface.h',
			'src/gui/vsServerInfo.h',
			'src/gui/vsQuickView.h',
			'src/gui/vsWebSocket.h',
			'src/gui/vsPermissions.h',
			'src/gui/vsPinger.h',
			'src/gui/vsPing.h',
			'src/gui/vsUrlHandler.h',
			'src/gui/vsQmlClipboard.h',
			'src/JTApplication.h'
		]

		if host_machine.system() == 'darwin'
			moc_h += ['src/gui/vsMacPermissions.h']
		endif

		if get_option('qtversion') == '5'
			qt_dep = dependency('qt5', modules: ['Core', 'Gui', 'Network', 'Widgets', 'Quick', 'QuickControls2', 'Qml', 'Svg', 'NetworkAuth', 'WebSockets'], include_type: 'system')
		else
			qt_dep = dependency('qt6', modules: ['Core', 'Gui', 'Network', 'Widgets', 'Quick', 'QuickControls2', 'Qml', 'Svg', 'NetworkAuth', 'WebSockets'], include_type: 'system')
		endif
		qres = ['src/gui/qjacktrip.qrc']
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
			'src/dblsqd/release.h',
			'src/dblsqd/semver.h',
			'src/dblsqd/update_dialog.h'
		]
		ui_h += ['src/dblsqd/update_dialog.ui']
	endif
endif
deps += qt_dep

# TODO: QT_OPENSOURCE should only be defined for open source Qt distribution
# in QMake this can be checked with QT_EDITION == 'OpenSource'
defines += '-DQT_OPENSOURCE'

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

if host_machine.system() == 'darwin'
	src += ['src/gui/NoNap.mm']
	# Adding CoreAudio here is a workaround and should be removed
	# when https://github.com/thestk/rtaudio/issues/302 is fixed
	# and arrived in all common package managers
	# Check at 2022-07-30
	apple_dep = dependency('appleframeworks', modules : ['foundation','coreaudio'])
	deps += apple_dep
	add_languages('objcpp')
endif

if host_machine.system() == 'darwin' and get_option('novs') == false
	src += ['src/gui/vsMacPermissions.mm']
	apple_av_dep = dependency('appleframeworks', modules : ['avfoundation'])
	deps += apple_av_dep
endif

qres_files = qt.compile_resources(sources: qres)
moc_files = qt.compile_moc(headers: moc_h, extra_args: defines)
ui_files = qt.compile_ui(sources: ui_h)

jacktrip = executable('jacktrip', src, qres_files, ui_files, moc_files, include_directories: incdirs, dependencies: deps, c_args: c_defines, cpp_args: defines, install: true )

help2man = find_program('help2man', required: false)
if not (host_machine.system() == 'windows')
	if help2man.found()
		help2man_opts = [
			'--no-info',
			'--section=1']
		custom_target('jacktrip.1',
			output: 'jacktrip.1',
			command: [help2man, help2man_opts, '--output=@OUTPUT@', jacktrip],
			install: true,
			install_dir: get_option('mandir') / 'man1')
	endif
endif

summary({'JACK': jack_dep.found(),
	'Weak JACK Linking': get_option('weakjack'),
	'RtAudio': rtaudio_dep.found()}, bool_yn: true, section: 'Audio Backends')

summary({'Application ID': application_id,
	'GUI': not get_option('nogui'),
	'WAIR': get_option('wair'),
	'Manpage': help2man.found()}, bool_yn: true, section: 'Configuration')

