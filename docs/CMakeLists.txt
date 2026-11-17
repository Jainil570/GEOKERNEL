# API reference via Doxygen:  cmake -DGK_BUILD_DOCS=ON ... && cmake --build <dir> --target docs
find_package(Doxygen REQUIRED dot)

set(GK_DOXYGEN_INPUT  "${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/README.md ${CMAKE_CURRENT_SOURCE_DIR}/ARCHITECTURE.md")
set(GK_DOXYGEN_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/api")

configure_file(Doxyfile.in "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)

add_custom_target(docs
  COMMAND Doxygen::doxygen "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  COMMENT "Generating API documentation -> ${GK_DOXYGEN_OUTPUT}/html/index.html"
  VERBATIM)
