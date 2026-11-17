add_executable(geokernel_bench bench_intersection.cpp)
target_link_libraries(geokernel_bench PRIVATE GeoKernel::geokernel benchmark::benchmark benchmark::benchmark_main)
gk_set_project_warnings(geokernel_bench)

# Convenience target: cmake --build --preset release --target run-benchmarks
add_custom_target(run-benchmarks
  COMMAND geokernel_bench --benchmark_counters_tabular=true
  DEPENDS geokernel_bench
  USES_TERMINAL)
