project('jacktrip', 'cpp', version: '1.2',
	default_options: ['cpp_std=c++11'])
qt5 = import('qt5')

dependencies = [
	dependency('qt5', modules: ['Core', 'Network']),
	dependency('jack'),
	dependency('threads')]

defines = []
if host_machine.system() == 'linux'
	defines += '-D__LINUX__'
elif host_machine.system() == 'darwin'
	defines += '-D__MAC_OSX__'
elif host_machine.system() == 'windows'
	defines += '-D__WIN_32__'
endif

moc_h = ['src/DataProtocol.h',
	'src/JackTrip.h',
	'src/JackTripWorker.h',
	'src/JackTripWorkerMessages.h',
	'src/NetKS.h',
	'src/PacketHeader.h',
	'src/Settings.h',
	'src/UdpDataProtocol.h',
	'src/UdpHubListener.h']
moc_files = qt5.preprocess(moc_headers : moc_h)

src = ['src/DataProtocol.cpp',
	'src/JMess.cpp',
	'src/JackTrip.cpp',
	'src/jacktrip_globals.cpp',
	'src/jacktrip_main.cpp',
	'src/JackTripThread.cpp',
	'src/JackTripWorker.cpp',
	'src/LoopBack.cpp',
	'src/PacketHeader.cpp',
	'src/ProcessPlugin.cpp',
	'src/RingBuffer.cpp',
	'src/Settings.cpp',
	'src/UdpDataProtocol.cpp',
	'src/UdpHubListener.cpp',
	'src/AudioInterface.cpp',
	'src/JackAudioInterface.cpp']

executable('jacktrip', src, moc_files, dependencies: dependencies, cpp_args: defines, install: true)
