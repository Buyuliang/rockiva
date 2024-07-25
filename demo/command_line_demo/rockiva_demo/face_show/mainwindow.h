#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGuiApplication>
#include <QScreen>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QCloseEvent>
#include <QListWidget>
#include <QLabel>
#include <QListWidgetItem>
#include <QIcon>
#include <QFile>
#include <QStandardItemModel>
#include <QTimer>
#include <QTextEdit>

#include "video_show.h"
#include "yuyv_qthread.h"
#include "show_pci_thread.h"
#include "show_pci_info_thread.h"
#include "calculate_thread.h"
#include "../modules/iva_app_face.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /* 主容器，Widget也可以当作一种容器 */
    QWidget *mainWidget;
    QWidget *rightWidget;
    QWidget *leftWidget;
    /* 界面水平区域布局 */
    QHBoxLayout *hboxLayout;
    /* 界面垂直区域布局 */
    QVBoxLayout *rvboxLayout;
    QVBoxLayout *lvboxLayout;
    videoshow   *video;
    QPushButton *startBtn;
    QPushButton *capBtn;
    QPushButton *recBtn;
    QLabel *imageLabel;
    QLabel *resultLabel;

    QListWidget *rlistview;
    QListWidget *llistview;

    YUYVQThread *t;
    PIC_Thread *pt;
    PIC_INFO_Thread *ptinf;
    CALCULATE_Thread *ct;
    /* 照片保存位置 */
    QDir *dir;

private slots:
    void onRec();
    void onStart();
    void onCap();

    //重写窗口关闭事件
protected :
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
