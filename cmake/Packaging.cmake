# -----------------------------------------------------------------------------
#  CPack — produces release archives (used by .github/workflows/release.yml)
# -----------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME                "geokernel")
set(CPACK_PACKAGE_VENDOR              "GeoKernel contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION             "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL        "${PROJECT_HOMEPAGE_URL}")
set(CPACK_RESOURCE_FILE_LICENSE       "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README        "${PROJECT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/\\\\.git/" "/build/" "/out/" "/\\\\.vscode/" "/\\\\.idea/")

include(CPack)
