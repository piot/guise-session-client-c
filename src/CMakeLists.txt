cmake_minimum_required(VERSION 3.17)
project(guise_session_client C)

add_subdirectory("deps/piot/clog/src/lib")
add_subdirectory("deps/piot/tiny-libc/src/lib")
add_subdirectory("deps/piot/flood-c/src/lib")
add_subdirectory("deps/piot/guise-serialize-c/src/lib")
add_subdirectory("deps/piot/datagram-transport-c/src/lib")
add_subdirectory("lib")


add_subdirectory("deps/piot/imprint/src/lib")
add_subdirectory("deps/piot/secure-random-c/src/lib")
add_subdirectory("deps/piot/guise-client-c/src/lib")
add_subdirectory("deps/piot/udp-client-c/src/lib")
add_subdirectory("deps/piot/monotonic-time-c/src/lib")

# add_subdirectory("test")

add_subdirectory("cli")
