add_executable(nxp-imx93-hw-vv-tool nxp_imx93_hw_vv_tool.cpp)
target_link_libraries(nxp-imx93-hw-vv-tool PRIVATE gpio_tester cpu_tester camera_tester gpu_tester memory_tester storage_tester display_tester usb_tester networking_tester power_tester form_factor_tester CLI11::CLI11)
target_include_directories(nxp-imx93-hw-vv-tool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_features(nxp-imx93-hw-vv-tool PRIVATE cxx_std_17)

# Install
install(TARGETS nxp-imx93-hw-vv-tool
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)