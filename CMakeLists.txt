cmake_minimum_required(VERSION 3.19)
project(transport_catalogue)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_GLIBCXX_DEBUG)
if (UNIX)
    add_compile_options(-O0 -g3)
endif ()

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

add_executable(transport_catalogue
        main.cpp
	    graph.h
        ranges.h
        router.h
        geo.h geo.cpp
        json.h json.cpp
        svg.h svg.cpp
        domain.h domain.cpp
        json_builder.h json_builder.cpp
        transport_router.h transport_router.cpp
        json_reader.h json_reader.cpp
        map_renderer.h map_renderer.cpp
        request_handler.h request_handler.cpp
        transport_catalogue.h transport_catalogue.cpp
        profile.h
        ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)

if (UNIX)
    target_compile_options(transport_catalogue PRIVATE -Werror -Wall -Wextra)
else ()
    target_compile_options(transport_catalogue PRIVATE /W3 /WX)
endif ()

add_subdirectory(tests)
