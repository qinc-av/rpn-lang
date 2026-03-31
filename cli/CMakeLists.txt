cmake_minimum_required(VERSION 3.24)

add_executable(rpn
  rpn-main.cpp
  output.cpp
)

set_property(TARGET rpn PROPERTY CXX_STANDARD 20)

target_link_libraries(rpn PRIVATE rpn-lang)

target_include_directories(rpn PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
)

install(TARGETS rpn RUNTIME DESTINATION bin)
