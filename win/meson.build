if host_machine.system() == 'windows'
	windows = import('windows')

	src += windows.compile_resources('qjacktrip.rc',
					depend_files: 'jacktrip.ico',
					include_directories: '.')

	defines += '-D_WIN32_WINNT=0x0600'
	defines += '-DWIN32_LEAN_AND_MEAN'
	defines += '-DNOMINMAX'

	deps += compiler.find_library('ws2_32', required: true)

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

endif
