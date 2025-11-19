add_library(memory_tester STATIC)
target_sources(memory_tester
  PRIVATE
    memory_tester.cpp
)
target_include_directories(memory_tester
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(memory_tester PUBLIC cxx_std_17)

# Install
install(TARGETS memory_tester
  EXPORT imx93_peripheral_testTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)