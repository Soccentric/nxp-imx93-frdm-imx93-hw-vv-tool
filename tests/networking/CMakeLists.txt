include(GoogleTest)

add_executable(networking_tester_tests test_networking_tester.cpp)
target_link_libraries(networking_tester_tests PRIVATE networking_tester gtest_main)
target_include_directories(networking_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(networking_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(networking_tester_tests PRIVATE --coverage)
  target_link_options(networking_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(networking_tester_tests)
