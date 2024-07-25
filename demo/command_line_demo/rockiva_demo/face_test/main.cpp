#include "mainwindow.h"
#include <QApplication>

#include <QTextEdit>
#include <QPainter>

int main(int argc, char *argv[])
{
    /* 输入校验 */
    if (2 != argc) {
        fprintf(stderr, "Usage: %s <video_dev>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    QApplication a(argc, argv);
    MainWindow w;

    // QTextEdit textEdit;
    // textEdit.setText("Hello, world!");

    // 设置 QTextEdit 的位置和大小
    // int x = w.width() / 2 - textEdit.width() / 2;
    // int y = w.height() / 2 - textEdit.height() / 2;
    // textEdit.setGeometry(x, y, textEdit.width(), textEdit.height());

    // w.setCentralWidget(&textEdit);
    // QPainter painter(w);
    // painter.setPen(QPen(Qt::black, 5)); // 设置笔的颜色和宽度
    // painter.drawRect(50, 50, 200, 200);
    
    w.show();
    w.setWindowTitle("QT V4L2 Hammer");

    return a.exec();
}
