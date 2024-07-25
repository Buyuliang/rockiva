cmake_minimum_required(VERSION 3.5) 

project(face)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加图片目录
file(MAKE_DIRECTORY images)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set (SRCS main.cpp
        mainwindow.cpp
        yuyv_qthread.cpp
        video_show.cpp
        show_pci_thread.cpp
        show_pci_info_thread.cpp
        calculate_thread.cpp)

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

add_executable(face_show
        ${SRCS}
        ${ROCKIVA_APP_SRCS_SDK}
        ${UTILS_SRCS_SDK}
)

target_link_libraries(face_show
        Qt5::Core 
        Qt5::Gui 
        Qt5::Widgets
        ${ROCKIVA_LIBS}
        ${COMMON_LIBS}
)
#target_link_libraries(face PRIVATE )

# 将图片文件添加到项目
file(GLOB IMAGES "images/*.png" "images/*.jpg" "images/*.jpeg")
foreach(IMAGES_FILE ${IMAGES})
  get_filename_component(IMAGES_PATH ${IMAGES_FILE} PATH)
  add_custom_command(
    TARGET face_show POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${IMAGES_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${IMAGES_PATH}
  )
endforeach()

install(TARGETS face_show DESTINATION ${OUT_PATH})