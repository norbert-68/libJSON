cmake_minimum_required (VERSION 2.6)
project (libJSON)

set (LIBJSON_VERSION_MAJOR 1)
set (LIBJSON_VERSION_MINOR 0)

configure_file (
    "${PROJECT_SOURCE_DIR}/include/libJSON/config.hxx.in"
    "${PROJECT_BINARY_DIR}/include/libJSON/config.hxx"
)

include_directories (
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)

add_subdirectory (src)

add_executable (JSONSample JSONSample.cxx)

