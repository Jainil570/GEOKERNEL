# -----------------------------------------------------------------------------
#  gk_set_project_warnings(<target>)
#  Applies a strict, compiler-specific warning set to a GeoKernel-owned target.
#  Third-party targets are never passed through this function.
# -----------------------------------------------------------------------------
function(gk_set_project_warnings target)
  set(MSVC_WARNINGS
    /W4
    /permissive-
    /w14242   # conversion, possible loss of data
    /w14254   # larger bit field to smaller bit field
    /w14263   # member function does not override any base class virtual
    /w14265   # class has virtual functions but destructor is not virtual
    /w14287   # unsigned/negative constant mismatch
    /w14296   # expression is always true/false
    /w14311   # pointer truncation
    /w14545 /w14546 /w14547 /w14549 /w14555
    /w14619   # unknown pragma warning
    /w14640   # thread-unsafe static member initialization
    /w14826   # sign-extended conversion
    /w14905 /w14906
    /w14928   # illegal copy-initialization
    /utf-8
    /Zc:__cplusplus
    /wd4251   # STL members in exported classes: same-toolchain ABI is a documented requirement
    /wd4275)  # exported class derives from header-only template base (gk::core::Component)

  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough)

  set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast)

  if(GK_WARNINGS_AS_ERRORS)
    list(APPEND MSVC_WARNINGS  /WX)
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND GCC_WARNINGS   -Werror)
  endif()

  if(MSVC)
    set(warnings ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(warnings ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(warnings ${GCC_WARNINGS})
  else()
    message(AUTHOR_WARNING "No warning set defined for compiler '${CMAKE_CXX_COMPILER_ID}'")
  endif()

  target_compile_options(${target} PRIVATE ${warnings})
endfunction()
