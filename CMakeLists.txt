cmake_minimum_required(VERSION 3.10)
project(NetSim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(netsim/include)

set(NETSIM_SOURCES
    netsim/src/factory.cpp
    netsim/src/helpers.cpp
    netsim/src/nodes.cpp
    netsim/src/package.cpp
    netsim/src/reports.cpp
    netsim/src/simulation.cpp
    netsim/src/storage_types.cpp
)

add_library(netsim_lib ${NETSIM_SOURCES})

add_executable(NetSimApp netsim/src/main.cpp)
target_link_libraries(NetSimApp netsim_lib)

# Tests
add_executable(NetSimTest netsim/test/main_test.cpp)
target_link_libraries(NetSimTest netsim_lib)

# Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(NetSimGTest netsim/test/main_gtest.cpp)
target_link_libraries(NetSimGTest netsim_lib gtest_main)

# Official Tests from netsim_tests
add_executable(NetSimOfficialTest
    netsim/netsim_tests/test/main_gtest.cpp
    netsim/netsim_tests/test/test_Factory.cpp
    netsim/netsim_tests/test/test_factory_io.cpp
    netsim/netsim_tests/test/test_nodes.cpp
    netsim/netsim_tests/test/test_package.cpp
    netsim/netsim_tests/test/test_reports.cpp
    netsim/netsim_tests/test/test_simulate.cpp
    netsim/netsim_tests/test/test_storage_types.cpp
)
target_include_directories(NetSimOfficialTest PRIVATE netsim/netsim_tests/include netsim/netsim_tests/mocks)
target_link_libraries(NetSimOfficialTest netsim_lib gmock gtest)

# Copy data files to build directory for IDE execution
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/factory_structure.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
