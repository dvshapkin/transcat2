# 'Google_test' is the subproject name
project(transcat_test)

# 'lib' is the folder with Google Test sources
add_subdirectory(lib)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
        ../transport_catalogue.proto
        ../map_renderer.proto
        ../svg.proto
        ../graph.proto
        ../transport_router.proto)

add_library(transcat_lib
        ../graph.h
        ../ranges.h
        ../router.h
        ../geo.h ../geo.cpp
        ../json.h ../json.cpp
        ../svg.h ../svg.cpp
        ../domain.h ../domain.cpp
        ../json_builder.h ../json_builder.cpp
        ../transport_router.h ../transport_router.cpp
        ../json_reader.h ../json_reader.cpp
        ../map_renderer.h ../map_renderer.cpp
        ../request_handler.h ../request_handler.cpp
        ../transport_catalogue.h ../transport_catalogue.cpp
        ../profile.h
        ../serialization.cpp ../serialization.h
        ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transcat_lib PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transcat_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
target_link_libraries(transcat_lib "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

# 'Google_Tests_run' is the target name
# 'test1.cpp tests2.cpp' are source files with tests
add_executable(transcat_test tests.cpp)
target_link_libraries(transcat_test transcat_lib gtest gtest_main)
