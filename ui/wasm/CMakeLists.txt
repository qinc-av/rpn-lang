
cmake_minimum_required (VERSION 3.24)

message("CMAKE_TOOLCHAIN_FILE is ${CMAKE_TOOLCHAIN_FILE}")

set(EMSCRIPTEN On)

if (EMSCRIPTEN)
  set(CMAKE_TOOLCHAIN_FILE "/opt/homebrew/Cellar/emscripten/4.0.1/libexec/cmake/Modules/Platform/Emscripten.cmake")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(rpn-wasm)

set(CMAKE_EXECUTABLE_SUFFIX ".mjs")

include(${CMAKE_CURRENT_SOURCE_DIR}/../../rpn-lang.cmake)

add_executable(rpn-wasm rpn-wasm.cpp ${RPN_LANG_SRCS})

target_compile_options(rpn-wasm PUBLIC
  -matomics
)

if(EMSCRIPTEN)
  target_link_options(rpn-wasm PUBLIC
  --no-entry
  -sASSERTIONS
  -sALLOW_MEMORY_GROWTH=1
  -sMODULARIZE
  -sEXPORT_ES6=1
  -sENVIRONMENT=worker
  -sSINGLE_FILE=1
  -sEXPORT_NAME=rpn_lang
  -sUSE_ES6_IMPORT_META=1
  -O3
)
target_link_libraries(rpn-wasm PUBLIC embind)
endif()

##target_link_libraries(rpn-test-ui)

