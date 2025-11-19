include(GoogleTest)

add_executable(memory_tester_tests test_memory_tester.cpp)
target_link_libraries(memory_tester_tests PRIVATE memory_tester gtest_main)
target_include_directories(memory_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(memory_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(memory_tester_tests PRIVATE --coverage)
  target_link_options(memory_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(memory_tester_tests)
