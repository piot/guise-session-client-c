cmake_minimum_required(VERSION 3.17)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS false)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(isDebug TRUE)
else()
  message("no debug")
  set(isDebug FALSE)
endif()

if(isDebug)
  message("Debug build detected")
  add_compile_definitions(CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)
  set(CONFIGURATION_DEBUG 1)
endif()

add_executable(
  guise_sessions_client_cli
  main.c)

target_link_libraries(guise_sessions_client_cli guise-session-client udp-client guise-client)

target_compile_options(
  guise_sessions_client_cli
  PRIVATE -Wall
          -Wextra
          -Wshadow
          -Wstrict-aliasing
          -pedantic
          -Wno-declaration-after-statement
          -Wno-extra-semi-stmt
          -Wno-undef
          -Wno-unused-variable
          -Wno-unused-parameter
          -Wno-padded
          -Werror=implicit-function-declaration
          -Werror=incompatible-pointer-types
          -Werror=missing-prototypes
          -Werror=int-conversion
          -Werror=return-type
          -Werror=incompatible-function-pointer-types
)# -Werror=incompatible-pointer-types-discards-qualifiers
if(COMPILER_CLANG)
  target_compile_options(
    guise_sessions_client_cli PRIVATE -Wmost -Weverything
                                      -Werror=missing-variable-declarations)
endif()

target_include_directories(guise_sessions_client_cli PRIVATE ../include)
