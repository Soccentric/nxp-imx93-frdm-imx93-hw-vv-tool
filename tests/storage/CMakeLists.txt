include(GoogleTest)

add_executable(storage_tester_tests test_storage_tester.cpp)
target_link_libraries(storage_tester_tests PRIVATE storage_tester gtest_main)
target_include_directories(storage_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(storage_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(storage_tester_tests PRIVATE --coverage)
  target_link_options(storage_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(storage_tester_tests)
