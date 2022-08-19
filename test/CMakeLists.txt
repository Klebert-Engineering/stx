project(stx-test)

if (NOT TARGET Catch2)
  FetchContent_Declare(Catch2
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG        "v3.1.0"
    GIT_SHALLOW    ON)
  FetchContent_MakeAvailable(Catch2)
endif()

add_executable(stx-test
  src/string.cpp
  src/format.cpp
  src/unique-tuple.cpp
  src/option-set.cpp
  src/map.cpp)

target_compile_options(stx-test
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>)

target_link_libraries(stx-test
  PUBLIC
    stx
    Catch2::Catch2WithMain)

include(CTest)
add_test(NAME stx-test COMMAND "$<TARGET_FILE:stx-test>")
