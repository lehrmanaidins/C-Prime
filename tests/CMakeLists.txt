include(FetchContent)

# Fetch GoogleTest from GitHub
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
)
# For Windows/MSVC compatibility settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect every test source file under tests/cprime
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cprime/*.cpp)

# Define your test executable
add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link GoogleTest libraries
target_link_libraries(run_tests gtest_main)

# Allow CMake and VS Code to discover the tests
include(GoogleTest)
gtest_discover_tests(run_tests)
