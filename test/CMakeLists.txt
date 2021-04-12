project(stx-test)

if (NOT TARGET Catch2::Catch2)
  unset(NOT_SUBPROJECT)
  add_subdirectory(deps/Catch2)
endif()

add_executable(stx-test
  src/main.cpp
  src/string.cpp
  src/format.cpp
  src/unique-tuple.cpp
  src/map.cpp)

target_compile_options(stx-test
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>)

target_link_libraries(stx-test
  PUBLIC
    stx
    Catch2::Catch2)

include(CTest)
add_test(NAME stx-test
  COMMAND "$<TARGET_FILE:stx-test>")
