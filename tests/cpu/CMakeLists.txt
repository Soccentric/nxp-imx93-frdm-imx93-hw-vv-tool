include(GoogleTest)

add_executable(cpu_tester_tests test_cpu_tester.cpp)
target_link_libraries(cpu_tester_tests PRIVATE cpu_tester gtest_main)
target_include_directories(cpu_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(cpu_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(cpu_tester_tests PRIVATE --coverage)
  target_link_options(cpu_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(cpu_tester_tests)
