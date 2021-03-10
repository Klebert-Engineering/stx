cmake_minimum_required(VERSION 3.14.0)
enable_testing()

# Do not build tests, if stx is bundled
if (NOT DEFINED PROJECT_NAME)
  project(stx)
  set(NOT_SUBPROJECT ON)
endif()

add_library(stx INTERFACE)

target_include_directories(stx
  INTERFACE
    include)

target_compile_features(stx
  INTERFACE
    cxx_std_17)

add_library(stx::stx ALIAS stx)

if (NOT_SUBPROJECT)
  add_subdirectory(test)
endif()
