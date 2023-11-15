cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(lruserver VERSION 1.0
                  DESCRIPTION "A server using the lrucache library"
                  LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${CMAKE_SOURCE_DIR}/libs/lrucache/include)

set(SOURCES_SERVER
    src/lruserver.cc
)

set(SOURCES_CLIENT
    src/lruclient.cc
)

configure_file(${CMAKE_SOURCE_DIR}/cache1.ini ${CMAKE_SOURCE_DIR}/build COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/cache2.ini ${CMAKE_SOURCE_DIR}/build COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/cache3.ini ${CMAKE_SOURCE_DIR}/build COPYONLY)

add_subdirectory(${CMAKE_SOURCE_DIR}/libs/lrucache)

add_executable(lruserver ${SOURCES_SERVER})
set_property(TARGET lruserver PROPERTY CXX_STANDARD 17)
target_link_libraries(lruserver PRIVATE lrucache sockpp-static crypto pthread)
target_compile_features(lruserver PRIVATE cxx_std_17)

add_executable(lruclient ${SOURCES_CLIENT})
set_property(TARGET lruclient PROPERTY CXX_STANDARD 17)
target_link_libraries(lruclient PRIVATE lrucache sockpp-static crypto pthread)
target_compile_features(lruclient PRIVATE cxx_std_17)

