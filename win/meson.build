if host_machine.system() == 'windows'
	windows = import('windows')

	src += windows.compile_resources('qjacktrip.rc',
					depend_files: 'jacktrip.ico',
					include_directories: '.')

	defines += '-D_WIN32_WINNT=0x0A00'
	defines += '-DWINVER=0x0A00'
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
		link_args += 'userenv.lib'
		link_args += 'Synchronization.lib'
		link_args += 'Netapi32.lib'
		link_args += 'Version.lib'
		link_args += 'Dwrite.lib'
		link_args += 'Iphlpapi.lib'
		link_args += 'Secur32.lib'
		link_args += 'Winhttp.lib'
		link_args += 'Dnsapi.lib'
		link_args += 'Iphlpapi.lib'
	endif

	uninstall_old_jacktrip = executable('uninstall_old_jacktrip', 'uninstall_old_jacktrip.cpp', include_directories: incdirs, dependencies: deps, link_args: link_args, c_args: c_defines, cpp_args: defines, install: false )

endif
