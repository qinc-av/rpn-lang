cmake_minimum_required (VERSION 3.24)

include(${CMAKE_CURRENT_SOURCE_DIR}/../rpn-lang.cmake)

find_package(Catch2 3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (rpn-tests)

if (${Catch2_FOUND})

add_executable(stack-test  ${RPN_LANG_SRCS} stack-test.cpp)

set_target_properties(stack-test PROPERTIES
          CXX_STANDARD 20
          CXX_EXTENSIONS OFF
          )

target_include_directories(stack-test PRIVATE ${RPN_LANG_DIR} ${RPN_LANG_DIR}/third_party)
target_link_libraries(stack-test PRIVATE Catch2::Catch2WithMain)

add_executable(runtime-test ${RPN_LANG_SRCS} runtime-test.cpp )

set_target_properties(runtime-test PROPERTIES
          CXX_STANDARD 20
          CXX_EXTENSIONS OFF
          )

target_include_directories(runtime-test PRIVATE ${RPN_LANG_DIR} ${RPN_LANG_DIR}/third_party)
target_compile_definitions(runtime-test PRIVATE TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(runtime-test PRIVATE Catch2::Catch2WithMain)

else()
message("couldn't find catch2")
endif()
