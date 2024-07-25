cmake_minimum_required(VERSION 3.10)

project(rockiva_demo)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "64bit")
	set (TARGET_LIB_ARCH lib64)
else()
	message(STATUS "32bit")
	set (TARGET_LIB_ARCH lib)
endif()

# librockiva
set(ROCKIVA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../librockiva/rockiva-rk3588-${CMAKE_SYSTEM_NAME}/)
message(STATUS ROCKIVA_PATH=${ROCKIVA_PATH})
include_directories(${ROCKIVA_PATH}/include)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(ROCKIVA_LIBS
	${ROCKIVA_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librockiva.so
	${ROCKIVA_PATH}/${CMAKE_ANDROID_ARCH_ABI}/librknnrt.so
    )
else()
    set(ROCKIVA_LIBS
        ${ROCKIVA_PATH}/${TARGET_LIB_ARCH}/librockiva.so
        ${ROCKIVA_PATH}/${TARGET_LIB_ARCH}/librknnrt.so
    )
endif()

# install prefix
if (NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/../install/rockiva_rk3588_linux)
endif()

# demos
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
add_subdirectory(rockiva_demo)

install(PROGRAMS ${ROCKIVA_LIBS} DESTINATION lib)
