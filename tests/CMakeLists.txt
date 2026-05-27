find_package(Catch2 3)

if (Catch2_FOUND)

  add_executable(stack-test stack-test.cpp)
  set_target_properties(stack-test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
  target_link_libraries(stack-test PRIVATE rpn-lang Catch2::Catch2WithMain)


  # Per-domain test binaries — one per dictionary. Each receives TESTS_DIR
  # (harmless if unused) so test files can load .rpn fixtures.
  # The EXISTS guard lets per-domain files appear incrementally during the
  # Phase 3 extraction without breaking the configure between tasks.
  set(rpn_domain_tests
    core
    finance
    timecode
    stats
    fraction
    numeric
    geometry
  )
  foreach(dom IN LISTS rpn_domain_tests)
    set(src "${dom}-test.cpp")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
      add_executable(${dom}-test ${src})
      set_target_properties(${dom}-test PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
      target_compile_definitions(${dom}-test PRIVATE TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
      target_link_libraries(${dom}-test PRIVATE rpn-lang Catch2::Catch2WithMain)
    endif()
  endforeach()

else()
  message("couldn't find catch2")
endif()
