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
#  BASE_DIRS ${MY_LIB_SOURCE_DIR}
  FILES
  rpn.h rpn-hl.h
)

#add_subdirectory(tests)
#add_subdirectory(ui/qt)

install(TARGETS rpn-lang
  EXPORT rpn-langTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  FILE_SET HEADERS DESTINATION include/rpn-lang
)

install(EXPORT rpn-langTargets
  FILE rpn-langTargets.cmake
  NAMESPACE q::
  DESTINATION lib/cmake/rpn-lang
)
