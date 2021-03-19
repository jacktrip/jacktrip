moc_h = ['DataProtocol.h',
	'JackTrip.h',
	'JackTripWorker.h',
	'JackTripWorkerMessages.h',
	'NetKS.h',
	'PacketHeader.h',
	'Settings.h',
	'UdpDataProtocol.h',
	'UdpHubListener.h']
moc_files = qt5.preprocess(moc_headers : moc_h)

src = [	'DataProtocol.cpp',
	'JackTrip.cpp',
	'AudioTester.cpp',
	'jacktrip_globals.cpp',
	'jacktrip_globals.cpp',
	'jacktrip_main.cpp',
	'JackTripThread.cpp',
	'JackTripWorker.cpp',
	'LoopBack.cpp',
	'PacketHeader.cpp',
	'ProcessPlugin.cpp',
	'RingBuffer.cpp',
	'JitterBuffer.cpp',
	'Settings.cpp',
	'UdpDataProtocol.cpp',
	'UdpHubListener.cpp',
	'AudioInterface.cpp',
	'Compressor.cpp',
	'Limiter.cpp',
	'Reverb.cpp']

qt5_dep = dependency('qt5', modules: ['Core', 'Network'])
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
	src += 'RtAudioInterface.cpp'
endif

if host_machine.system() == 'windows'
	wingetopt = cmake.subproject('wingetopt')
	deps += wingetopt.dependency('wingetopt')
	deps += compiler.find_library('ws2_32', required: true)
endif

deps += dependency('threads')
if jack_dep.found()
src += 	['JackAudioInterface.cpp',
	'JMess.cpp']
endif


executable('jacktrip', src, moc_files, dependencies: deps, cpp_args: defines, install: true )
