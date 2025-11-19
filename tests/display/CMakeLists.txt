include(GoogleTest)

add_executable(display_tester_tests test_display_tester.cpp)
target_link_libraries(display_tester_tests PRIVATE display_tester gtest_main)
target_include_directories(display_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(display_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(display_tester_tests PRIVATE --coverage)
  target_link_options(display_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(display_tester_tests)
