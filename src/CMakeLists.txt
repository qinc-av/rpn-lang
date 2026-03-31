cmake_minimum_required (VERSION 3.24)

project(rpn-lang-dev)

add_library(rpn-lang
  rpn-stack.cpp
  rpn-interp.cpp
  types-dict.cpp
  math-dict.cpp
  stack-dict.cpp
  logic-dict.cpp
  fraction-dict.cpp
  fraction.cpp
  timecode-dict.cpp
  vecmx3.cpp
  matrix-dict.cpp
  stats-dict.cpp
  geometry.cpp
  geometry-dict.cpp
  vec3-dict.cpp
  mx3-dict.cpp
  keypad-dict.cpp
  rpn-stdlib.cpp
  rpn-hl.cpp
  rpn-mcp.cpp
)

set_property(TARGET rpn-lang PROPERTY CXX_STANDARD 20)

target_sources(rpn-lang
  PUBLIC FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..
  FILES
  ../rpn.h ../rpn-hl.h ../vecmx3.h ../rpn-matrix.h
)

target_include_directories(rpn-lang
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third_party>
         $<INSTALL_INTERFACE:include/rpn-lang/third_party>
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
            $<INSTALL_INTERFACE:include>
)

install(TARGETS rpn-lang
  EXPORT rpn-langConfig
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  FILE_SET HEADERS DESTINATION include/rpn-lang
)

install(EXPORT rpn-langConfig
  FILE rpn-langConfig.cmake
  NAMESPACE q::
  DESTINATION share/rpn-lang
)
