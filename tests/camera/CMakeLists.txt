include(GoogleTest)

add_executable(camera_tester_tests test_camera_tester.cpp)
target_link_libraries(camera_tester_tests PRIVATE camera_tester gtest_main)
target_include_directories(camera_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(camera_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(camera_tester_tests PRIVATE --coverage)
  target_link_options(camera_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(camera_tester_tests)
