project('jacktrip', ['cpp','c'],
		default_options: ['cpp_std=c++17','warning_level=2'])

if get_option('profile') == 'development'
  application_id = 'org.jacktrip.JackTrip.Devel'
  name_suffix = ' (Development Snapshot)'
else
  application_id = 'org.jacktrip.JackTrip'
  name_suffix = ''
endif

qt5 = import('qt5')
cmake = import('cmake')

compiler = meson.get_compiler('cpp')

defines = ['-DWAIRTOHUB']
c_defines = []
if host_machine.system() == 'windows'
	defines += '-D_WIN32_WINNT=0x0600'
	defines += '-DWIN32_LEAN_AND_MEAN'
	defines += '-DNOMINMAX'
endif

incdirs = []

src = [	'src/DataProtocol.cpp',
	'src/JackTrip.cpp',
	'src/AudioTester.cpp',
	'src/jacktrip_globals.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/RingBuffer.cpp',
	'src/JitterBuffer.cpp',
	'src/PoolBuffer.cpp',
	'src/Settings.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Reverb.cpp',
	'src/main.cpp',
	'src/SslServer.cpp',
	'src/Auth.cpp']

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/Auth.h',
	'src/SslServer.h']

ui_h = []
qres = []

deps = [dependency('threads')]
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
	qt5_dep = dependency('qt5', modules: ['Core', 'Network'], include_type: 'system')
else
	qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Network', 'Widgets'], include_type: 'system')
	src += ['src/gui/qjacktrip.cpp',
		'src/gui/about.cpp',
		'src/gui/messageDialog.cpp',
		'src/gui/textbuf.cpp']
	ui_h += ['src/gui/qjacktrip.ui',
		'src/gui/messageDialog.ui',
		'src/gui/about.ui']
	moc_h += ['src/gui/about.h',
		'src/gui/qjacktrip.h',
		'src/gui/messageDialog.h',
		'src/gui/textbuf.h']
	qres = ['src/gui/qjacktrip.qrc']
endif
deps += qt5_dep

prepro_files = qt5.preprocess(moc_headers : moc_h, ui_files : ui_h, qresources : qres)

# TODO: QT_OPENSOURCE should only be defined for open source Qt distribution
# in QMake this can be checked with QT_EDITION == 'OpenSource'
defines += '-DQT_OPENSOURCE'

rtaudio_dep = dependency('rtaudio', required: get_option('rtaudio'))
if rtaudio_dep.found() == true
	defines += '-DRT_AUDIO'
	src += 'src/RtAudioInterface.cpp'
	deps += rtaudio_dep
endif

if host_machine.system() == 'windows'
	deps += compiler.find_library('ws2_32', required: true)
endif

if compiler.get_id() == 'msvc'
	opt_var = cmake.subproject_options()
	if get_option('buildtype') == 'release'
		opt_var.add_cmake_defines({'CMAKE_BUILD_TYPE': 'Release'})
	else
		opt_var.add_cmake_defines({'CMAKE_BUILD_TYPE': 'Debug'})
	endif
	wingetopt = cmake.subproject('wingetopt', options: opt_var)
	deps += wingetopt.dependency('wingetopt')
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

subdir('linux')

jacktrip = executable('jacktrip', src, prepro_files, include_directories: incdirs, dependencies: deps, c_args: c_defines, cpp_args: defines, install: true )

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

