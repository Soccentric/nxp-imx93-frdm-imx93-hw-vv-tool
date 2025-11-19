add_library(camera_tester STATIC)
target_sources(camera_tester
  PRIVATE
    camera_tester.cpp
)
target_include_directories(camera_tester
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(camera_tester PUBLIC cxx_std_17)

# Try to find V4L2 libraries, but don't fail if not found
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(V4L2 QUIET libv4l2)
  if(V4L2_FOUND)
    target_link_libraries(camera_tester PRIVATE ${V4L2_LIBRARIES})
    target_include_directories(camera_tester PRIVATE ${V4L2_INCLUDE_DIRS})
  endif()
endif()

# Install
install(TARGETS camera_tester
  EXPORT imx93_peripheral_testTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)