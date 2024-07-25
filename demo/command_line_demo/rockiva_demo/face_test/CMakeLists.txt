cmake_minimum_required(VERSION 3.5) 

project(face)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set (SRCS main.cpp
        mainwindow.cpp
        videoshow.cpp
        v4l2_thread.cpp)

set(ROCKIVA_APP_SRCS_SDK
        ../iva_app.c
        ../modules/iva_app_det.c
        ../modules/iva_app_ba.c
        ../modules/iva_app_face.c
        ../modules/iva_app_object.c
        ../modules/iva_app_plate.c
        ../modules/iva_app_pose.c
        ../modules/iva_app_ts.c
)

set(UTILS_SRCS_SDK
        ../utils/common_utils.c
        ../utils/face_db.c
        ../utils/image_utils.c
        ../utils/image_buffer.c
)

add_executable(face
        ${SRCS}
        ${ROCKIVA_APP_SRCS_SDK}
        ${UTILS_SRCS_SDK}
)

target_link_libraries(face
        Qt5::Core 
        Qt5::Gui 
        Qt5::Widgets
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)
#target_link_libraries(face PRIVATE )

install(TARGETS face DESTINATION ${OUT_PATH})