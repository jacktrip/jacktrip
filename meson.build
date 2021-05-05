project('jacktrip', 'cpp', version: '1.3.0',
		default_options: ['cpp_std=c++17'])
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

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/JackTripWorker.h',
	'src/PacketHeader.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h',
	'src/about.h',
	'src/qjacktrip.h',
	'src/messageDialog.h',
	'src/Auth.h',
	'src/Patcher.h',
	'src/SslServer.h']
	
ui_h = ['src/qjacktrip.ui',
	'src/messageDialog.ui',
	'src/about.ui']

moc_files = qt5.preprocess(moc_headers : moc_h, ui_files : ui_h)

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
	'src/about.cpp',
	'src/main.cpp',
	'src/qjacktrip.cpp',
	'src/messageDialog.cpp',
	'src/SslServer.cpp',
	'src/Auth.cpp']

qt5_dep = dependency('qt5', modules: ['Core', 'Network', 'Gui', 'Widgets'])
jack_dep = dependency('jack', required: get_option('jack'))
if not jack_dep.found()
	defines += '-D__NO_JACK__'
endif

deps = [qt5_dep, jack_dep]

if not get_option('rtaudio').disabled()
	defines += '-D__RT_AUDIO__'
	rtaudio_dep = dependency('rtaudio', required: false)
	deps += rtaudio_dep
	if not rtaudio_dep.found()
		opt_var = cmake.subproject_options()
		opt_var.add_cmake_defines({'RTAUDIO_API_JACK': 'OFF'})
		rtaudio = cmake.subproject('rtaudio', options: opt_var)
		rtaudio_dep = rtaudio.dependency('rtaudio')
		deps += rtaudio_dep
	endif
	src += 'src/RtAudioInterface.cpp'
endif

if host_machine.system() == 'windows'
	wingetopt = cmake.subproject('wingetopt')
	deps += wingetopt.dependency('wingetopt')
	deps += compiler.find_library('ws2_32', required: true)
endif

if host_machine.system() == 'darwin'
	src += ['src/NoNap.mm']
	apple_dep = dependency('appleframeworks', modules : 'foundation')
	deps += apple_dep
	add_languages('objcpp')
	compiler = meson.get_compiler('objcpp')
endif

deps += dependency('threads')
if jack_dep.found()
src += 	['src/JackAudioInterface.cpp',
	'src/JMess.cpp',
	'src/Patcher.cpp']
endif


executable('qjacktrip', src, moc_files, dependencies: deps, cpp_args: defines, install: true )
