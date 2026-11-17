# Out-of-tree consumer used by CI to prove the installed CMake package works:
#   cmake -S tests/install_consumer -B build/consumer -DCMAKE_PREFIX_PATH=<install prefix>
cmake_minimum_required(VERSION 3.20)
project(geokernel_consumer LANGUAGES CXX)

find_package(GeoKernel 1.0 REQUIRED CONFIG)

add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE GeoKernel::geokernel)
