cmake_minimum_required(VERSION 3.10.0)
project(c-prime VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(cprime ./src/cprime.cpp)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -d)

# enable_testing()
# add_subdirectory(tests)
