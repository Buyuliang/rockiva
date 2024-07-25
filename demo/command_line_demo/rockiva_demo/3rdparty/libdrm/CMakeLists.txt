cmake_minimum_required(VERSION 3.10)

project(drm)

add_library(drm SHARED IMPORTED)
set_property(TARGET drm PROPERTY IMPORTED_LOCATION
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libdrm.so
)

target_include_directories(drm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
