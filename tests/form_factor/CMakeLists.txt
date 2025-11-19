include(GoogleTest)

add_executable(form_factor_tester_tests test_form_factor_tester.cpp)
target_link_libraries(form_factor_tester_tests PRIVATE form_factor_tester gtest_main)
target_include_directories(form_factor_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(form_factor_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(form_factor_tester_tests PRIVATE --coverage)
  target_link_options(form_factor_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(form_factor_tester_tests)
