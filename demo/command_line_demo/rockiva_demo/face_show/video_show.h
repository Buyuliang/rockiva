#ifndef VIDEO_SHOW_H
#define VIDEO_SHOW_H

#include <QWidget>
#include <QPainter>
#include <QPointF>

#include "../modules/iva_app_face.h"

#define FRAMEBUFFER_COUNT 4
#define PIXWIDTH    1280
#define PIXHEIGHT   720

class videoshow : public QWidget{
    Q_OBJECT

public:
    explicit videoshow(QWidget *parent = 0);

    /* 显示的每帧图像 */
    QImage img;

    /* 重写父类下的protected方法*/
protected:
    void paintEvent(QPaintEvent *);
};

#endif // VIDEO_SHOW_H
