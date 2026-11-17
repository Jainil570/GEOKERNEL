# -----------------------------------------------------------------------------
#  gk_enable_clang_tidy(<target>)
#  Runs clang-tidy (configured by the repo-root .clang-tidy) on every
#  translation unit of the target as part of the normal build.
# -----------------------------------------------------------------------------
if(GK_ENABLE_CLANG_TIDY)
  find_program(GK_CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16)
  if(NOT GK_CLANG_TIDY_EXE)
    message(FATAL_ERROR "GK_ENABLE_CLANG_TIDY=ON but clang-tidy was not found")
  endif()
  message(STATUS "GeoKernel: clang-tidy -> ${GK_CLANG_TIDY_EXE}")
endif()

function(gk_enable_clang_tidy target)
  if(NOT GK_ENABLE_CLANG_TIDY)
    return()
  endif()

  set(_tidy_cmd "${GK_CLANG_TIDY_EXE}" "-p=${CMAKE_BINARY_DIR}")
  if(GK_WARNINGS_AS_ERRORS)
    list(APPEND _tidy_cmd "--warnings-as-errors=*")
  endif()
  set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${_tidy_cmd}")
endfunction()
