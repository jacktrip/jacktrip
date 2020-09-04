project('jacktrip', 'cpp', version: '1.2.1',
		default_options: ['cpp_std=c++11'])

qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Network'])
jack_dep = dependency('jack')
thread_dep = dependency('threads')

libjacktrip_dep = dependency('jacktrip', fallback: ['libjacktrip','libjacktrip_dep'])


defines = []
if host_machine.system() == 'linux'
	defines += '-D__LINUX__'
elif host_machine.system() == 'darwin'
	defines += '-D__MAC_OSX__'
elif host_machine.system() == 'windows'
	defines += '-D__WIN_32__'
endif

moc_h = ['src/JackTripWorkerMessages.h',
	'src/NetKS.h',
	'src/Settings.h',]
moc_files = qt5.preprocess(moc_headers : moc_h)

src = ['src/jacktrip_main.cpp',
	'src/Settings.cpp',
	'src/Compressor.cpp',
	'src/Limiter.cpp',
	'src/Reverb.cpp',]

executable('jacktrip', src, moc_files, dependencies: [libjacktrip_dep, qt5_dep, jack_dep, thread_dep], cpp_args: defines, install: true )
