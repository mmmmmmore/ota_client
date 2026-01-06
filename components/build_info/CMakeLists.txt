set(BUILD_INFO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${BUILD_INFO_GEN_DIR})

# Generate a header embedding the build-time OTA version
# Use CMAKE_CURRENT_LIST_DIR to reference the original CMake file location
configure_file(${CMAKE_CURRENT_LIST_DIR}/version.h.in ${BUILD_INFO_GEN_DIR}/build_info_version.h @ONLY)

idf_component_register(SRCS "build_info.c" INCLUDE_DIRS "include" "${BUILD_INFO_GEN_DIR}")
