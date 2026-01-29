# Native Meson build for msquic
# This overlay builds msquic using CMake directly and exposes the dependency

project('msquic', 'c',
  version: '2.5.6',
  license: 'MIT'
)

# Only support Linux for now
assert(host_machine.system() == 'linux', 'msquic meson overlay only supports Linux')

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

# Output paths
if cmake_build_type == 'Release'
  msquic_lib_path = msquic_build / 'bin' / 'Release' / 'libmsquic.a'
else
  msquic_lib_path = msquic_build / 'bin' / 'Debug' / 'libmsquic.a'
endif

# CMake configuration command
cmake_configure = run_command(
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
  check: true,
  capture: true
)

message('msquic CMake configure output: ' + cmake_configure.stdout())

# Build msquic using CMake
# We use a custom_target to build so it happens at build time, not configure time
cmake_prog = find_program('cmake')

# Custom target to build msquic
msquic_build_target = custom_target(
  'msquic_build',
  output: 'libmsquic.a',
  command: [
    cmake_prog,
    '--build', msquic_build,
    '--config', cmake_build_type,
    '--parallel',
    '&&',
    'cp', msquic_lib_path, '@OUTPUT@'
  ],
  build_by_default: true,
  console: true
)

# Find system libraries that msquic needs
atomic_dep = cc.find_library('atomic', required: true)
numa_dep = cc.find_library('numa', required: false)
thread_dep = dependency('threads')

# Build dependencies list
msquic_link_deps = [atomic_dep, thread_dep]
if numa_dep.found()
  msquic_link_deps += numa_dep
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
