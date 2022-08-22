cmake_minimum_required(VERSION 3.14.0)
enable_testing()

include(FetchContent)

project(stx)

# Do not build tests, if stx is bundled
set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MAIN_PROJECT ON)
endif()

add_library(stx INTERFACE)

target_include_directories(stx
  INTERFACE
    include)

target_compile_features(stx
  INTERFACE
    cxx_std_17)

add_library(stx::stx ALIAS stx)

if (MAIN_PROJECT)
  add_subdirectory(test)
endif()
