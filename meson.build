project('jacktrip', 'cpp', version: '1.4.0',
		default_options: ['cpp_std=c++17','warning_level=2'])
qt5 = import('qt5')
cmake = import('cmake')

compiler = meson.get_compiler('cpp')

defines = ['-DWAIRTOHUB']
if host_machine.system() == 'linux'
	defines += '-D__LINUX__'
elif host_machine.system() == 'darwin'
	defines += '-D__MAC_OSX__'
elif host_machine.system() == 'windows'
	defines += '-D__WIN_32__'
	defines += '-D_WIN32_WINNT=0x0600'
	defines += '-DWIN32_LEAN_AND_MEAN'
	defines += '-DNOMINMAX'
endif

if get_option('nogui') == true
	defines += '-D__NO_GUI__'
	qt5_dep = dependency('qt5', modules: ['Core', 'Network'])
else
	qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Network', 'Widgets'])
endif

jack_dep = dependency('jack', required: get_option('jack'))
if not jack_dep.found()
	defines += '-D__NO_JACK__'
endif
deps = [qt5_dep, jack_dep]

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/gui/about.h',
	'src/gui/qjacktrip.h',
	'src/gui/messageDialog.h',
	'src/Auth.h',
	'src/SslServer.h']
	
if jack_dep.found()
	moc_h += ['src/Patcher.h']
endif
	
ui_h = ['src/gui/qjacktrip.ui',
	'src/gui/messageDialog.ui',
	'src/gui/about.ui']

qres = ['src/gui/qjacktrip.qrc']

moc_files = qt5.preprocess(moc_headers : moc_h, ui_files : ui_h, qresources : qres)

src = [	'src/DataProtocol.cpp',
	'src/JackTrip.cpp',
	'src/AudioTester.cpp',
	'src/jacktrip_globals.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/RingBuffer.cpp',
	'src/JitterBuffer.cpp',
	'src/Settings.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Reverb.cpp',
	'src/gui/about.cpp',
	'src/main.cpp',
	'src/gui/qjacktrip.cpp',
	'src/gui/messageDialog.cpp',
	'src/SslServer.cpp',
	'src/Auth.cpp']

rtaudio_dep = dependency('rtaudio', required: get_option('rtaudio'))
if rtaudio_dep.found() == true
	defines += '-D__RT_AUDIO__'
	src += 'src/RtAudioInterface.cpp'
	deps += rtaudio_dep
endif

if host_machine.system() == 'windows'
	deps += compiler.find_library('ws2_32', required: true)
endif

if compiler.get_id() == 'msvc'
	wingetopt = cmake.subproject('wingetopt')
	deps += wingetopt.dependency('wingetopt')
endif

if host_machine.system() == 'darwin'
	src += ['src/gui/NoNap.mm']
	apple_dep = dependency('appleframeworks', modules : 'foundation')
	deps += apple_dep
	add_languages('objcpp')
endif

if host_machine.system() == 'linux'
	install_data('linux/jacktrip.desktop', install_dir: get_option('datadir') / 'applications')
	install_data('linux/jacktrip.svg', install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps')
	install_data('linux/jacktrip_48x48.png', rename: 'jacktrip.png', install_dir: get_option('datadir') / 'icons' / 'hicolor' / '48x48' / 'apps')
endif

deps += dependency('threads')
if jack_dep.found()
src += 	['src/JackAudioInterface.cpp',
	'src/JMess.cpp',
	'src/Patcher.cpp']
endif

jacktrip = executable('jacktrip', src, moc_files, dependencies: deps, cpp_args: defines, install: true )

help2man = find_program('help2man', required: false)
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
