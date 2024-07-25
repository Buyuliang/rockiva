cmake_minimum_required(VERSION 3.10)

project(rockiva_demo)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "64bit")
    set (TARGET_LIB_ARCH lib64)
else()
    message(STATUS "32bit")
    set (TARGET_LIB_ARCH lib)
endif()

set(OUT_PATH ${PROJECT_NAME})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(3rdparty/stb_image)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sqlite3)

set(JPEG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/jpeg_turbo)
if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(LIBJPEG ${JPEG_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_ANDROID_ARCH_ABI}/libturbojpeg.a)
else()
    if (CMAKE_C_COMPILER MATCHES "uclibc")
        set(LIBJPEG ${JPEG_PATH}/${CMAKE_SYSTEM_NAME}/${TARGET_LIB_ARCH}_uclibc/libturbojpeg.a)
    else()
        set(LIBJPEG ${JPEG_PATH}/${CMAKE_SYSTEM_NAME}/${TARGET_LIB_ARCH}/libturbojpeg.a)
    endif()
endif()
include_directories(${JPEG_PATH}/include)

# mpp
set(MPP_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/mpp)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_C_COMPILER MATCHES "uclibc")
        set(MPP_LIBS ${MPP_PATH}/${CMAKE_SYSTEM_NAME}/${TARGET_LIB_ARCH}_uclibc/librockchip_mpp.so)
    else()
        set(MPP_LIBS ${MPP_PATH}/${CMAKE_SYSTEM_NAME}/${TARGET_LIB_ARCH}/librockchip_mpp.so)
    endif()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(MPP_LIBS ${MPP_PATH}/${CMAKE_SYSTEM_NAME}/${CMAKE_ANDROID_ARCH_ABI}/libmpp.so)
endif()
include_directories(${MPP_PATH}/include)

# zlmediakit
set(ZLMEDIAKIT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/zlmediakit)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (CMAKE_C_COMPILER MATCHES "uclibc")
        message("zlmediakit no support uclibc")
    else()
        include_directories(${ZLMEDIAKIT_PATH}/include)
        set(ZLMEDIAKIT_LIBS ${ZLMEDIAKIT_PATH}/${CMAKE_SYSTEM_NAME}/${TARGET_LIB_ARCH}/libmk_api.so)
    endif()
endif()

if (ZLMEDIAKIT_LIBS)
    add_definitions(-DBUILD_VIDEO_RTSP)
endif()

set(UTILS_SRCS
        utils/common_utils.c
        utils/face_db.c
        utils/image_utils.c
        utils/image_buffer.c
)

set(COMMON_LIBS
        sqlite3
        dl
        m
        ${LIBJPEG}
)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
set(COMMON_LIBS
        ${COMMON_LIBS}
        pthread
)
endif()

set(ROCKIVA_APP_SRCS
        iva_app.c
        modules/iva_app_det.c
        modules/iva_app_ba.c
        modules/iva_app_face.c
        modules/iva_app_object.c
        modules/iva_app_plate.c
        modules/iva_app_pose.c
        modules/iva_app_ts.c
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/face_test)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/face_show)

add_executable(rockiva_demo
        rockiva_demo.c
        ${ROCKIVA_APP_SRCS}
        ${UTILS_SRCS}
)

target_link_libraries(rockiva_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)

if (MPP_LIBS)
add_executable(rockiva_video_demo
        rockiva_video_demo.cpp
        utils/mpp_decoder.cpp
        ${ROCKIVA_APP_SRCS}
        ${UTILS_SRCS}
)
target_link_libraries(rockiva_video_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
        ${MPP_LIBS}
        ${ZLMEDIAKIT_LIBS}
)
endif()

add_executable(rockiva_one_demo
        rockiva_one_demo.c 
        ${UTILS_SRCS}
)

target_link_libraries(rockiva_one_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)

add_executable(rockiva_face_import_demo
        face_recog_import_demo.c
        ${UTILS_SRCS}
)

target_link_libraries(rockiva_face_import_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)

add_executable(rockiva_face_recog_demo
        face_recog_picture_demo.c
        ${UTILS_SRCS}
)

target_link_libraries(rockiva_face_recog_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)

add_executable(rockiva_face_search_demo
        face_recog_search_demo.c
        ${UTILS_SRCS}
)

target_link_libraries(rockiva_face_search_demo
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)

install(TARGETS rockiva_demo DESTINATION ${OUT_PATH})
install(TARGETS rockiva_one_demo DESTINATION ${OUT_PATH})
install(TARGETS rockiva_face_import_demo DESTINATION ${OUT_PATH})
install(TARGETS rockiva_face_recog_demo DESTINATION ${OUT_PATH})
install(TARGETS rockiva_face_search_demo DESTINATION ${OUT_PATH})

if (MPP_LIBS)
install(TARGETS rockiva_video_demo DESTINATION ${OUT_PATH})
install(PROGRAMS ${MPP_LIBS} DESTINATION lib)
endif()

if (ZLMEDIAKIT_LIBS)
install(PROGRAMS ${ZLMEDIAKIT_LIBS} DESTINATION lib)
endif()