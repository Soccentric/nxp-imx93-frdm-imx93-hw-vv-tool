add_library(networking_tester STATIC)
target_sources(networking_tester
  PRIVATE
    networking_tester.cpp
)
target_include_directories(networking_tester
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(networking_tester PUBLIC cxx_std_17)

# Install
install(TARGETS networking_tester
  EXPORT imx93_peripheral_testTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)