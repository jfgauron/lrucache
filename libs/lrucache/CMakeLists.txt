cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "/opt/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(lrucache VERSION 1.0
                 DESCRIPTION "A simple Least Recently Used (LRU) Cache"
                 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/nuraft/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    # src/main.cc
    # src/logger.cc
    # src/in_memory_log_store.cc
    src/server.cc
    src/client.cc
    src/cache/client_config.cc
    src/cache/endpoint.cc
    src/cache/cache_server.cc
    src/cache/cache_config.cc
    src/cache/cache_state.cc
    src/cache/cache_storage.cc
    src/cache/snapshot_storage.cc
    src/raft/raft_server.cc
    src/raft/in_memory_log_store.cc
)
add_library(lrucache ${SOURCES})

add_subdirectory(libs/nuraft)
target_link_libraries(lrucache PRIVATE NuRaft::static_lib)

find_package(OpenSSL REQUIRED)
target_link_libraries(lrucache PRIVATE OpenSSL::SSL)

# sockpp-static
option(SOCKPP_BUILD_STATIC "Build static library" ON)
set(SOCKPP_BUILD_STATIC ON)
FetchContent_Declare(
    sockpp
    GIT_REPOSITORY https://github.com/fpagliughi/sockpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(sockpp)
target_link_libraries(lrucache PRIVATE sockpp)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(TEST_SOURCES
        test/main.cc
        test/helpers/utilities.cc
        test/test_cache_state.cc
        test/test_cache_storage.cc
        test/test_cache_snapshot.cc
        test/test_geo_locator.cc
    )

    add_executable(tests ${TEST_SOURCES})
    find_package(Catch2 3 REQUIRED)
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain lrucache)
    target_include_directories(tests PRIVATE src)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests)
endif()