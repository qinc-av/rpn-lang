
set(RPN_DIR ${CMAKE_CURRENT_LIST_DIR})
set(RPN_SRCS rpn-stack.cpp rpn-runtime.cpp rpn-dict.cpp)

list(TRANSFORM RPN_SRCS PREPEND ${RPN_DIR}/src/)

#message(RPN_SRCS: ${RPN_SRCS})

