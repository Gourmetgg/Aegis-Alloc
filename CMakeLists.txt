cmake_minimum_required(VERSION 3.20)

project(
  AegisAlloc
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "This project must be configured with GCC (g++).")
endif()

function(enable_strict_warnings target_name)
  target_compile_options(
    "${target_name}"
    PRIVATE
      -Wall
      -Wextra
      -Werror
      -pedantic
      -Wconversion
  )
endfunction()

include(CTest)
enable_testing()

add_library(aegis_alloc INTERFACE)
target_compile_features(aegis_alloc INTERFACE cxx_std_20)
target_include_directories(
  aegis_alloc
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

if(BUILD_TESTING)
  add_executable(aegis_alloc_tests tests/sanity_test.cpp)
  target_link_libraries(aegis_alloc_tests PRIVATE aegis_alloc GTest::gtest_main)
  enable_strict_warnings(aegis_alloc_tests)

  include(GoogleTest)
  gtest_discover_tests(aegis_alloc_tests)
endif()
