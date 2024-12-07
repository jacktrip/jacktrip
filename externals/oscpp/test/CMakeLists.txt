set(warnings -Wall -Wextra -Wno-unused-parameter -Werror)

add_compile_options(
  "$<$<CXX_COMPILER_ID:Clang>:${warnings}>"
  "$<$<CXX_COMPILER_ID:AppleClang>:${warnings}>"
  "$<$<CXX_COMPILER_ID:GNU>:${warnings}>"
)

# =============================================================================
# autocheck tests

add_executable(oscpp_autocheck
    oscpp_autocheck.cpp
)

target_include_directories(oscpp_autocheck PRIVATE
    ../include
    autocheck/include
)

add_test(oscpp_autocheck oscpp_autocheck)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/README.cpp
    COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/../tools/mdcode.rb
                 ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
                 ${CMAKE_CURRENT_BINARY_DIR}/README.cpp
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
)

# =============================================================================
# README code

add_executable(oscpp_readme
  ${CMAKE_CURRENT_BINARY_DIR}/README.cpp
)

target_include_directories(oscpp_readme PRIVATE
    ../include
)

add_test(oscpp_readme oscpp_readme)
