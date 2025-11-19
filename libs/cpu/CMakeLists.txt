add_library(cpu_tester STATIC)
target_sources(cpu_tester
  PRIVATE
    cpu_tester.cpp
)
target_include_directories(cpu_tester
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(cpu_tester PUBLIC cxx_std_17)

# Install
install(TARGETS cpu_tester
  EXPORT imx93_peripheral_testTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)