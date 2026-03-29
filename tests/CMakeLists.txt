find_package(Catch2 3)

if (Catch2_FOUND)

add_executable(stack-test stack-test.cpp)
set_target_properties(stack-test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_link_libraries(stack-test PRIVATE rpn-lang Catch2::Catch2WithMain)

add_executable(runtime-test runtime-test.cpp)
set_target_properties(runtime-test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_definitions(runtime-test PRIVATE TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(runtime-test PRIVATE rpn-lang Catch2::Catch2WithMain)

else()
  message("couldn't find catch2")
endif()
