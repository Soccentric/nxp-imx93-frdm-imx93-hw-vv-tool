cmake_minimum_required(VERSION 3.16)
project(form_factor_tester)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create form factor tester library
add_library(form_factor_tester STATIC
    form_factor_tester.cpp
)

target_include_directories(form_factor_tester
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Link against common utilities if available
if(TARGET common_utils)
    target_link_libraries(form_factor_tester PRIVATE common_utils)
endif()

# Install library
install(TARGETS form_factor_tester
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install headers
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../include/form_factor_tester.h
    DESTINATION include/imx93_peripheral_test
)