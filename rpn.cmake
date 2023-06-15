
set(RPN_DIR ${CMAKE_CURRENT_LIST_DIR})
set(RPN_SRCS rpn-stack.cpp rpn-interp.cpp types-dict.cpp math-dict.cpp stack-dict.cpp logic-dict.cpp)

list(TRANSFORM RPN_SRCS PREPEND ${RPN_DIR}/src/)

#message(RPN_SRCS: ${RPN_SRCS})

