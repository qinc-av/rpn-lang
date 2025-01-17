cmake_minimum_required (VERSION 3.24)

include(${CMAKE_CURRENT_SOURCE_DIR}/../rpn-lang.cmake)

find_package(Catch2 3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (rpn-tests)

if (${Catch2_FOUND})

add_executable(stack-test  ${RPN_LANG_SRCS} stack-test.cpp)
target_include_directories(stack-test PRIVATE ${RPN_LANG_DIR})
target_link_libraries(stack-test PRIVATE Catch2::Catch2WithMain)

add_executable(runtime-test ${RPN_LANG_SRCS} runtime-test.cpp )
target_include_directories(runtime-test PRIVATE ${RPN_LANG_DIR})
target_link_libraries(runtime-test PRIVATE Catch2::Catch2WithMain)

endif()
