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
	defines += '-DNOMINMAX'
endif

subdir('src')
