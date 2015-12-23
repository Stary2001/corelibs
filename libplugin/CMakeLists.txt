set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES "src/*.cpp")

include_directories( include )

add_library(plugin ${SOURCES})
install(TARGETS plugin ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/libplugin)