cmake_minimum_required(VERSION 3.16)
project(power_tester)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create power tester library
add_library(power_tester STATIC
    power_tester.cpp
)

target_include_directories(power_tester
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link against common utilities if available
if(TARGET common_utils)
    target_link_libraries(power_tester PRIVATE common_utils)
endif()

# Install library
install(TARGETS power_tester
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../include/power_tester.h
    DESTINATION include/imx93_peripheral_test
)