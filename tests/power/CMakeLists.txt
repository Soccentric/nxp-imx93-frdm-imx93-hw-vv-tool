include(GoogleTest)

add_executable(power_tester_tests test_power_tester.cpp)
target_link_libraries(power_tester_tests PRIVATE power_tester gtest_main)
target_include_directories(power_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(power_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(power_tester_tests PRIVATE --coverage)
  target_link_options(power_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(power_tester_tests)
