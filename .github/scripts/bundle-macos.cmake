# Bundle non-system dynamic libraries into the macOS application.
#
# This script is invoked only by the macOS workflow.
#
# Usage:
# cmake
#   -DAPP_BUNDLE=/path/to/ExedraSim.app
#   -DWEBP_PREFIX=/opt/homebrew/opt/webp
#   -DGLFW_PREFIX=/opt/homebrew/opt/glfw
#   -P .github/scripts/bundle-macos.cmake

if(NOT DEFINED APP_BUNDLE OR NOT EXISTS "${APP_BUNDLE}")
    message(FATAL_ERROR "APP_BUNDLE is missing or does not exist: ${APP_BUNDLE}")
endif()

set(DEPENDENCY_DIRS)

foreach(PREFIX_VAR WEBP_PREFIX GLFW_PREFIX)
    if(DEFINED ${PREFIX_VAR})
        list(APPEND DEPENDENCY_DIRS "${${PREFIX_VAR}}/lib")
    endif()
endforeach()

include(BundleUtilities)

message(STATUS "Bundling dependencies for: ${APP_BUNDLE}")
message(STATUS "Dependency search directories: ${DEPENDENCY_DIRS}")

fixup_bundle("${APP_BUNDLE}" "" "${DEPENDENCY_DIRS}")
verify_app("${APP_BUNDLE}")
