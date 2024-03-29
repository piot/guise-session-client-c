cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS false)


# --- Detect CMake build type, compiler and operating system ---

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message("detected debug build")
  set(isDebug TRUE)
else()
  message("detected release build")
  set(isDebug FALSE)
endif()


if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  set(COMPILER_NAME "clang")
  set(COMPILER_CLANG 1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(COMPILER_NAME "gcc")
  set(COMPILER_GCC 1)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(COMPILER_NAME "msvc")
  set(COMPILER_MSVC 1)
endif()

message("detected compiler: '${CMAKE_C_COMPILER_ID}' (${COMPILER_NAME})")

set(useSanitizers false)

if(useSanitizers)
  message("using sanitizers")
  set(sanitizers "-fsanitize=address")
endif()


# --- Library ---


file(GLOB lib_src FOLLOW_SYMLINKS "./*.c")


add_executable(guise_session_client_cli ${lib_src})


if(APPLE)
  target_compile_definitions(guise_session_client_cli PRIVATE TORNADO_OS_MACOS)
  set(OS_MACOS 1)
elseif(UNIX)
  target_compile_definitions(guise_session_client_cli PRIVATE TORNADO_OS_LINUX)
  set(OS_LINUX 1)
elseif(WIN32)
  target_compile_definitions(guise_session_client_cli PRIVATE TORNADO_OS_WINDOWS)
  set(OS_WINDOWS 1)
endif()

if(isDebug)
  message("Setting definitions based on debug")
  target_compile_definitions(guise_session_client_cli PRIVATE CONFIGURATION_DEBUG)
endif()


if(OS_LINUX)
  message("Linux Detected!")
elseif(OS_MACOS)
  message("MacOS detected!")
elseif(OS_WINDOWS)
  message("Windows detected!")
endif()

target_include_directories(guise_session_client_cli PUBLIC ../include)
target_compile_definitions(guise_session_client_cli PRIVATE _POSIX_C_SOURCE=200809L)

if(COMPILER_CLANG)
  target_compile_options(
    guise_session_client_cli
    PRIVATE -Weverything
            -Werror
            -Wno-padded # the order of the fields in struct can matter (ABI)
            -Wno-declaration-after-statement # must be bug in clang? shows up
                                             # even if std=c99
            -Wno-unsafe-buffer-usage # unclear why it fails on clang-16
            -Wno-unknown-warning-option # support newer clang versions, e.g. clang-16
            -Wno-disabled-macro-expansion # bug in emscripten compiler?
            -Wno-poison-system-directories # might be bug in emscripten compiler?
            ${sanitizers})
elseif(COMPILER_GCC)
target_compile_options(
    guise_session_client_cli
    PRIVATE -Wall
            -Wextra
            -Wpedantic
            -Werror
            -Wno-padded # the order of the fields in struct can matter (ABI)
            ${sanitizers})
elseif(COMPILER_MSVC)
  target_compile_options(
    guise_session_client_cli
    PRIVATE /Wall
            /WX
            # /F 5000000 # stack size
            /wd4820 # bytes padding added after data member
            /wd4668 # bug in winioctl.h (is not defined as a preprocessor macro, replacing with '0' for '#if/#elif')
            /wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
            /wd4005 # Bug in ntstatus.h (macro redefinition)
          )
else()
  target_compile_options(guise_session_client_cli PRIVATE -Wall)
endif()


target_link_libraries(guise_session_client_cli PUBLIC guise-session-client PUBLIC udp-client PUBLIC guise-client)


if(useSanitizers)
  target_link_libraries(guise_session_client_cli PRIVATE ${sanitizers})
endif()

if(NOT isDebug)
  message("optimize!")
  target_compile_options(guise_session_client_cli PRIVATE -O3)
endif()