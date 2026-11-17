add_executable(geokernel-cli geokernel-cli/main.cpp)
target_link_libraries(geokernel-cli PRIVATE GeoKernel::geokernel)
gk_set_project_warnings(geokernel-cli)

if(GK_INSTALL)
  install(TARGETS geokernel-cli RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT runtime)
endif()

# Smoke tests: the CLI must run end-to-end against the shipped test assets.
if(GK_BUILD_TESTS)
  add_test(NAME cli.version COMMAND geokernel-cli version)
  add_test(NAME cli.components COMMAND geokernel-cli components)
  add_test(NAME cli.info
           COMMAND geokernel-cli info ${PROJECT_SOURCE_DIR}/tests/data/cube.obj)
  add_test(NAME cli.raycast
           COMMAND geokernel-cli --metrics raycast ${PROJECT_SOURCE_DIR}/tests/data/cube_ascii.stl
                   0.5 0.7 -5  0 0 1)
  add_test(NAME cli.bad_file COMMAND geokernel-cli info does-not-exist.stl)
  add_test(NAME cli.usage    COMMAND geokernel-cli)
  set_tests_properties(cli.bad_file cli.usage PROPERTIES WILL_FAIL TRUE)
  set_tests_properties(cli.version cli.components cli.info cli.raycast cli.bad_file cli.usage
                       PROPERTIES LABELS "smoke")
endif()
