cmake_minimum_required(VERSION 3.10)
project(SSLChatServer VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# For PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED IMPORTED_TARGET libpqxx)

# Define the executable
add_executable(ssl_server ssl_server.h ssl_server.cpp scipher.cpp)

# Include directories for Boost, OpenSSL, and PQXX
target_include_directories(ssl_server PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${PQXX_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(ssl_server
    PRIVATE
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
    PkgConfig::PQXX
)

# Find Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Request to configure Doxyfile with correct paths
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    # Add a target for Doxygen in CMake
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else()
    message("Doxygen needs to be installed to generate the documentation.")
endif()

# Set compiler options
target_compile_options(ssl_server PRIVATE -Wall -Wextra -Wpedantic)

# Installation settings
install(TARGETS ssl_server RUNTIME DESTINATION bin)

# Download and compile Google Test and Google Mock
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add a test target
add_executable(tests
    ssl_server_test.cpp
    ssl_server.h
)

target_link_libraries(tests
    PRIVATE
    gtest_main
    gtest
    gmock
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
    PkgConfig::PQXX
)

# Add tests
add_test(NAME tests COMMAND tests)

