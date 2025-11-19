include(GoogleTest)

add_executable(usb_tester_tests test_usb_tester.cpp)
target_link_libraries(usb_tester_tests PRIVATE usb_tester gtest_main)
target_include_directories(usb_tester_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_compile_features(usb_tester_tests PRIVATE cxx_std_17)

if(ENABLE_COVERAGE)
  target_compile_options(usb_tester_tests PRIVATE --coverage)
  target_link_options(usb_tester_tests PRIVATE --coverage)
endif()

gtest_discover_tests(usb_tester_tests)
