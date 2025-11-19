include(GoogleTest)

add_executable(gpu_tester_tests test_gpu_tester.cpp)
target_link_libraries(gpu_tester_tests PRIVATE gpu_tester gtest_main)
target_include_directories(gpu_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(gpu_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(gpu_tester_tests PRIVATE --coverage)
  target_link_options(gpu_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(gpu_tester_tests)
