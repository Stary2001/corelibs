set (CMAKE_CXX_STANDARD 11)

file(GLOB_RECURSE SOURCES "src/*.cpp")
find_package(GnuTLS)

include_directories(include ${GnuTLS_INCLUDE_DIRS})

add_library(sock ${SOURCES})
target_link_libraries(sock ${GnuTLS_LIBRARY})
install(TARGETS sock ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/libsock)
