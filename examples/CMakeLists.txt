foreach(example IN ITEMS load_and_inspect raycast_scene custom_reader_plugin)
  add_executable(example_${example} ${example}.cpp)
  target_link_libraries(example_${example} PRIVATE GeoKernel::geokernel)
  gk_set_project_warnings(example_${example})
endforeach()

if(GK_BUILD_TESTS)
  add_test(NAME example.load_and_inspect
           COMMAND example_load_and_inspect ${PROJECT_SOURCE_DIR}/tests/data/cube.obj)
  add_test(NAME example.raycast_scene COMMAND example_raycast_scene)
  add_test(NAME example.custom_reader_plugin COMMAND example_custom_reader_plugin)
  set_tests_properties(example.load_and_inspect example.raycast_scene example.custom_reader_plugin
                       PROPERTIES LABELS "smoke")
endif()
