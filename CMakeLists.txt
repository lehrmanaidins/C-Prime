cmake_minimum_required(VERSION 3.10.0)
project(c-prime VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(c-prime ./src/c-prime.cpp)

# Rebuild when an embedded resource (src/embedded/resources.hpp `#embed`s these)
# changes.
set_property(TARGET c-prime APPEND PROPERTY OBJECT_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/lists/separators.txt
    ${CMAKE_SOURCE_DIR}/src/lists/delimiters.txt
    ${CMAKE_SOURCE_DIR}/src/lists/keywords.txt
    ${CMAKE_SOURCE_DIR}/src/lists/primitives.txt
    ${CMAKE_SOURCE_DIR}/src/lists/operators.txt
    ${CMAKE_SOURCE_DIR}/src/lists/cpp_builtins.txt
    ${CMAKE_SOURCE_DIR}/src/runtime/c-prime.hpp
    ${CMAKE_SOURCE_DIR}/src/runtime/std.hpp
    ${CMAKE_SOURCE_DIR}/src/runtime/std.hprime
)

