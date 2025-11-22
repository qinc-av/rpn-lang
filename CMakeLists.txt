# for homebrew:
# CMAKE_PREFIX_PATH=/usr/local/Cellar/qt@5/5.15.8_3/ cmake -B n -G Ninja

cmake_minimum_required (VERSION 3.24)

project(rpn-lang)
include(rpn-lang.cmake)
add_library(rpn-lang)
target_sources(rpn-lang ${RPN_LANG_SRCS})

#add_subdirectory(tests)
#add_subdirectory(ui/qt)

install(TARGETS rpn-lang
  EXPORT rpn-lang-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(EXPORT rpn-lang-targets
  FILE rpn-lang-targets.cmake
  NAMESPACE q::
  DESTINATION lib/cmake/rpn-lang
)
