#include "video_show.h"

videoshow::videoshow(QWidget *parent)
    : QWidget(parent)
{
    setAutoFillBackground(true);
}

void videoshow::paintEvent(QPaintEvent *){
    try{
        QPainter painter(this);

        if(!img.isNull()){
			// QImage image((QString)("/home/mixtile/share/rockiva/demo/command_line_demo/install/rockiva_rk3588_linux/rockiva_demo/images/out/frames/") + "0000.jpg");
            // painter.drawImage(QPointF(0,0),image);
            painter.drawImage(QPointF(0,0),img);

            for (int i = 0; i < detresult.objNum; i++) {
                int re_obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.width,  detresult.faceInfo[i].faceRect.topLeft.x);
                int re_obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.height, detresult.faceInfo[i].faceRect.topLeft.y);
                int re_obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.width,  detresult.faceInfo[i].faceRect.bottomRight.x);
                int re_obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.height, detresult.faceInfo[i].faceRect.bottomRight.y);
                //printf(" xxxccccxxx ----- (%d, %d) (%d, %d)", re_obj_rect_x1, re_obj_rect_y1, re_obj_rect_x2, re_obj_rect_y2);
                QRect rect(re_obj_rect_x1, re_obj_rect_y1,
                re_obj_rect_x2 - re_obj_rect_x1 + 1, re_obj_rect_y2 - re_obj_rect_y1 + 1);
                // 设置矩形框的颜色和线宽
                painter.setPen(QPen(Qt::blue, 2));
                // 在窗口上画矩形框
                painter.drawRect(rect);
            }
            // detresult.objNum = 0;
        }   

    }catch(...){}
}
