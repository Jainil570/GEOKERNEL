# -----------------------------------------------------------------------------
#  Sanitizers are applied globally (before dependencies are fetched) so that
#  GoogleTest and every GeoKernel target share one instrumented runtime.
#  Mixing instrumented and non-instrumented STL code yields false positives
#  (e.g. container-overflow), so a global switch is the safe choice.
# -----------------------------------------------------------------------------
set(_gk_sanitizers "")

if(GK_ENABLE_ASAN)
  list(APPEND _gk_sanitizers "address")
endif()

if(GK_ENABLE_UBSAN)
  if(MSVC)
    message(WARNING "UBSan is not supported by MSVC — ignoring GK_ENABLE_UBSAN")
  else()
    list(APPEND _gk_sanitizers "undefined")
  endif()
endif()

if(_gk_sanitizers)
  list(JOIN _gk_sanitizers "," _gk_sanitizer_list)
  message(STATUS "GeoKernel: sanitizers enabled -> ${_gk_sanitizer_list}")

  if(MSVC)
    # MSVC only supports ASan; it needs /Zi for readable reports.
    add_compile_options(/fsanitize=address /Zi)
    add_link_options(/INCREMENTAL:NO)
  else()
    add_compile_options(
      -fsanitize=${_gk_sanitizer_list}
      -fno-omit-frame-pointer
      -fno-optimize-sibling-calls
      -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${_gk_sanitizer_list})
  endif()
endif()
