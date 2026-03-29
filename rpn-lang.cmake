
set(RPN_LANG_DIR ${CMAKE_CURRENT_LIST_DIR})
set(RPN_LANG_SRCS
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
)

list(TRANSFORM RPN_LANG_SRCS PREPEND ${RPN_LANG_DIR}/src/)

#message(RPN_LANG_SRCS: ${RPN_LANG_SRCS})

