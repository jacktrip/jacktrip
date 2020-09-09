project('libjacktrip', 'cpp', version: '1.2.1')
qt5 = import('qt5')
dependencies = [dependency('qt5', modules: ['Core', 'Network']),
	dependency('jack'),
	dependency('threads')]

inc = include_directories('include')

defines = []
if host_machine.system() == 'linux'
	defines += '-D__LINUX__'
elif host_machine.system() == 'darwin'
	defines += '-D__MAC_OSX__'
elif host_machine.system() == 'windows'
	defines += '-D__WIN_32__'
endif

subdir('include')
subdir('src')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : libjacktrip,
                 version : '1.2.1',
                 name : 'jacktrip',
                 filebase : 'jacktrip',
                 description : 'A Library to barnicate your foos.')

libjacktrip_dep = declare_dependency(include_directories : inc,
  link_with : libjacktrip)
