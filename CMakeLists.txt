# for homebrew:
# CMAKE_PREFIX_PATH=/usr/local/Cellar/qt@5/5.15.8_3/ cmake -B n -G Ninja

cmake_minimum_required (VERSION 3.24)

project(rpn-lang)

include(rpn-lang.cmake)

add_library(rpn-lang)
set_property(TARGET rpn-lang PROPERTY CXX_STANDARD 20)

target_sources(rpn-lang
  PRIVATE
  ${RPN_LANG_SRCS}

  PUBLIC FILE_SET HEADERS
#  BASE_DIRS
  FILES
  rpn.h rpn-hl.h
)

target_include_directories(rpn-lang INTERFACE $<INSTALL_INTERFACE:include>)

#add_subdirectory(tests)
#add_subdirectory(ui/qt)

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
