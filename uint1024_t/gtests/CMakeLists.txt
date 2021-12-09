cmake_minimum_required(VERSION 3.14)
project(gtests)

# GoogleTest requires at least C++11
set(CMAKE_CXX_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  tests
  tests.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../uint1024.c
)

#${CMAKE_CURRENT_SOURCE_DIR}/libuint1024.a
target_link_libraries(
  tests 
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)