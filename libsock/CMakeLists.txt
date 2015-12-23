set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES "src/*.cpp")

include_directories( include )

add_library(sock ${SOURCES})
install(TARGETS sock ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/libsock)