cmake_minimum_required(VERSION 3.20)

project(
  AegisAlloc
  VERSION 0.1.0
  DESCRIPTION "Aegis-Alloc custom allocator scaffolding for C++20"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(FATAL_ERROR "Aegis-Alloc requires GCC (g++) for this scaffold.")
endif()

function(enable_strict_warnings target_name)
  target_compile_options(
    "${target_name}"
    PRIVATE
      -Wall
      -Wextra
      -Werror
      -Wpedantic
      -Wconversion
  )
endfunction()

add_library(
  aegis_alloc
  STATIC
    src/LinearAllocator.cpp
    src/StackAllocator.cpp
)
target_compile_features(aegis_alloc PUBLIC cxx_std_20)
target_include_directories(
  aegis_alloc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
enable_strict_warnings(aegis_alloc)

include(CTest)
enable_testing()

if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(
    aegis_alloc_tests
    tests/sanity_test.cpp
    tests/linear_allocator_stress_test.cpp
  )
  target_link_libraries(aegis_alloc_tests PRIVATE aegis_alloc GTest::gtest_main)
  target_compile_features(aegis_alloc_tests PRIVATE cxx_std_20)
  enable_strict_warnings(aegis_alloc_tests)

  include(GoogleTest)
  gtest_discover_tests(aegis_alloc_tests)
endif()
