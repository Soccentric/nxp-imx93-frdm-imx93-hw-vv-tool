include(GoogleTest)

add_executable(gpio_tester_tests test_gpio_tester.cpp)
target_link_libraries(gpio_tester_tests PRIVATE gpio_tester gtest_main)
target_include_directories(gpio_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(gpio_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(gpio_tester_tests PRIVATE --coverage)
  target_link_options(gpio_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(gpio_tester_tests)