cmake_minimum_required (VERSION 3.10)
project (rpn-cnc)
add_executable(rpn-cnc  rpn-controller.cpp cnc-app.cpp rpn-stack.cpp trpn.cpp)
set_target_properties(rpn-cnc PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )

add_executable(stack-test  rpn-stack.cpp stack-test.cpp)
set_target_properties(stack-test PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )

add_executable(runtime-test  rpn-stack.cpp rpn-runtime.cpp runtime-test.cpp)
set_target_properties(runtime-test PROPERTIES
          CXX_STANDARD 17
          CXX_EXTENSIONS OFF
          )
