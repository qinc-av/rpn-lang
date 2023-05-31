cmake_minimum_required (VERSION 3.10)

find_package(Catch2 3 REQUIRED)

project (rpn-cnc)
add_executable(rpn-controller  src/rpn-controller.cpp src/cnc-app.cpp src/rpn-stack.cpp tests/trpn.cpp)
set_target_properties(rpn-controller PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )
target_include_directories(rpn-controller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(stack-test  src/rpn-stack.cpp tests/stack-test.cpp)
set_target_properties(stack-test PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )
target_include_directories(stack-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(stack-test PRIVATE Catch2::Catch2WithMain)

add_executable(runtime-test src/rpn-stack.cpp src/rpn-runtime.cpp src/rpn-dict.cpp tests/runtime-test.cpp )
set_target_properties(runtime-test PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )
target_include_directories(runtime-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(runtime-test PRIVATE Catch2::Catch2WithMain)
