# Native Meson build for msquic
# This overlay builds msquic using CMake directly and exposes the dependency

project('msquic', 'c',
  version: '2.5.6',
  license: 'MIT'
)

# Support Linux, macOS, and Windows
host_system = host_machine.system()
supported_systems = ['linux', 'darwin', 'windows']
assert(host_system in supported_systems, 'msquic meson overlay only supports Linux, macOS, and Windows')

cc = meson.get_compiler('c')

# Build configuration
buildtype = get_option('buildtype')
if buildtype == 'release' or buildtype == 'plain'
  cmake_build_type = 'Release'
else
  cmake_build_type = 'Debug'
endif

# Paths
msquic_src = meson.current_source_dir()
msquic_build = meson.current_build_dir() / 'cmake_build'
msquic_inc = msquic_src / 'src' / 'inc'

# Extract macOS deployment target and architecture flags from compiler
cmake_extra_flags = []
if host_system == 'darwin'
  # Extract deployment target and architectures from build flags
  deployment_target = ''
  archs = []

  # Combine c_args and c_link_args to search for flags
  all_flags = get_option('c_args') + get_option('c_link_args')

  # Track if we just saw -arch flag
  next_is_arch = false
  foreach flag : all_flags
    if next_is_arch
      # This is the architecture value after -arch
      if flag not in archs
        archs += [flag]
      endif
      next_is_arch = false
    elif flag.startswith('-mmacosx-version-min=')
      deployment_target = flag.split('=')[1]
    elif flag == '-arch'
      # Next flag will be the architecture
      next_is_arch = true
    endif
  endforeach

  # Set deployment target for CMake
  if deployment_target != ''
    cmake_extra_flags += ['-DCMAKE_OSX_DEPLOYMENT_TARGET=' + deployment_target]
    message('Using macOS deployment target: ' + deployment_target)
  endif

  # Set architectures for CMake (for universal builds)
  if archs.length() > 0
    cmake_extra_flags += ['-DCMAKE_OSX_ARCHITECTURES=' + ';'.join(archs)]
    message('Using macOS architectures: ' + ';'.join(archs))
  endif
endif

# Output paths - platform specific
if host_system == 'windows'
  # Windows uses .lib extension
  lib_ext = '.lib'
  lib_prefix = ''
else
  # Linux and macOS use .a extension
  lib_ext = '.a'
  lib_prefix = 'lib'
endif

if cmake_build_type == 'Release'
  msquic_lib_path = msquic_build / 'bin' / 'Release' / (lib_prefix + 'msquic' + lib_ext)
else
  msquic_lib_path = msquic_build / 'bin' / 'Debug' / (lib_prefix + 'msquic' + lib_ext)
endif

# CMake configuration command
cmake_args = [
  'cmake',
  '-S', msquic_src,
  '-B', msquic_build,
  '-DCMAKE_BUILD_TYPE=' + cmake_build_type,
  '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
  '-DQUIC_BUILD_SHARED=OFF',
  '-DQUIC_BUILD_TEST=OFF',
  '-DQUIC_BUILD_TOOLS=OFF',
  '-DQUIC_BUILD_PERF=OFF',
  '-DQUIC_ENABLE_LOGGING=OFF',
  '-DQUIC_USE_EXTERNAL_OPENSSL=ON',
  '-DQUIC_TLS_LIB=openssl',
  '-DCMAKE_PREFIX_PATH=' + get_option('qt_prefix'),
] + cmake_extra_flags

cmake_configure = run_command(
  cmake_args,
  check: true,
  capture: true
)

message('msquic CMake configure output: ' + cmake_configure.stdout())

# Build msquic using CMake
# We use a custom_target to build so it happens at build time, not configure time
cmake_prog = find_program('cmake')

# Custom target to build msquic
# Copy command differs on Windows
if host_system == 'windows'
  copy_cmd = ['cmd', '/c', 'copy', msquic_lib_path, '@OUTPUT@']
else
  copy_cmd = ['sh', '-c', 'cp ' + msquic_lib_path + ' @OUTPUT@']
endif

msquic_build_target = custom_target(
  'msquic_build',
  output: lib_prefix + 'msquic' + lib_ext,
  command: [
    cmake_prog,
    '--build', msquic_build,
    '--config', cmake_build_type,
    '--parallel',
    '&&'
  ] + copy_cmd,
  build_by_default: true,
  console: true
)

# Find system libraries that msquic needs (platform-specific)
thread_dep = dependency('threads')
msquic_link_deps = [thread_dep]

if host_system == 'linux'
  # Linux-specific dependencies
  atomic_dep = cc.find_library('atomic', required: true)
  numa_dep = cc.find_library('numa', required: false)
  msquic_link_deps += [atomic_dep]
  if numa_dep.found()
    msquic_link_deps += numa_dep
  endif
elif host_system == 'darwin'
  # macOS might need CoreFoundation and Security frameworks
  # msquic on macOS typically uses system OpenSSL or provides its own
  corefoundation_dep = dependency('appleframeworks', modules: ['CoreFoundation', 'Security'], required: false)
  if corefoundation_dep.found()
    msquic_link_deps += corefoundation_dep
  endif
elif host_system == 'windows'
  # Windows needs ws2_32, secur32, etc. - typically handled by msquic's CMake
  # Add Windows-specific libraries if needed
  ws2_dep = cc.find_library('ws2_32', required: false)
  if ws2_dep.found()
    msquic_link_deps += ws2_dep
  endif
endif

# Create the msquic dependency
# Note: We link to the static library via link_args since it's from a custom_target
msquic_dep = declare_dependency(
  include_directories: include_directories('src/inc'),
  compile_args: ['-DQUIC_BUILD_STATIC'],
  link_args: [
    msquic_lib_path,
  ],
  dependencies: msquic_link_deps,
  sources: msquic_build_target
)

meson.override_dependency('msquic', msquic_dep)
