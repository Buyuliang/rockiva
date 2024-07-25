#include "mainwindow.h"

// void time_fun() {
//     update_time = 1;
//     usleep(100);
//     update_time = 0;
// }

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    /* 获取屏幕的分辨率，Qt官方建议使用这种方法获取屏幕分辨率，防上多屏设备导致对应不上 */
    QList <QScreen *> list_screen =  QGuiApplication::screens();

    /* 如果是ARM平台，直接设置大小为屏幕的大小 */
#if 1
    this->resize(list_screen.at(0)->geometry().width(),
        list_screen.at(0)->geometry().height());
#else
    /* 否则则设置主窗体大小为800x480 */
    this->resize(800, 480);
#endif

    /* 控件实例化 */
    mainWidget  = new QWidget();
    rightWidget = new QWidget();
    leftWidget = new QWidget();

    hboxLayout  = new QHBoxLayout();
    lvboxLayout = new QVBoxLayout();
    rvboxLayout = new QVBoxLayout();


    // startBtn = new QPushButton("开始",this);
    // capBtn   = new QPushButton("拍照",this);
    // recBtn   = new QPushButton("录制",this);
    rlistview = new QListWidget(this);
    rlistview->setIconSize(QSize(150,150));

    llistview = new QListWidget(this);
    // rlistview->setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    // llistview->setSelectionBehavior(QAbstractItemView::SelectItems);
    // 设置QListWidget的尺寸
    llistview->setFixedSize(PIXWIDTH, 200); // 设置宽度为200像素，高度为200像素
    // 设置QListWidget的布局属性为水平布局
    llistview->setLayoutDirection(Qt::LeftToRight); // 设置为水平布局
    llistview->setFlow(QListView::LeftToRight);//从左到右横向的排列,图片和文字也是从左到右排列       
    llistview->setViewMode(QListView::IconMode);//左到右横向的排列,上图下文模式
    llistview->setSpacing(10);
    // llistview->setViewMode(QListView::ListMode);//子项就会从上到下排列,图片和文字是从左到右排列.
    // llistview->setFlow(QListView::TopToBottom);//子项就会从上到下排列,图片和文字是从左到右排列.

    llistview->setIconSize(QSize(150, 100));

    video = new videoshow();
    video->setFixedWidth(PIXWIDTH);
    video->setFixedHeight(PIXHEIGHT);

    // QTextEdit *edit = new QTextEdit();
    // edit->setGeometry(50, 50, 200, 150); // 设置位置和大小
    /* 按钮属性 */
    // recBtn->setMaximumHeight(40);
    // recBtn->setMaximumWidth(100);
    // startBtn->setMaximumHeight(40);
    // startBtn->setMaximumWidth(100);
    // capBtn->setMaximumHeight(40);
    // capBtn->setMaximumWidth(100);
    // capBtn->setEnabled(false);

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setScaledContents(true);
    QLabel *resultLabel = new QLabel(this);
    resultLabel->setWordWrap(true);

    // new QListWidgetItem (QIcon(":/image/face1.png"), tr("Tom"), listview);
    // new QListWidgetItem (tr("bobby"), listview);
    // new QListWidgetItem (tr("kevin"), listview);

    rvboxLayout->addWidget(rlistview);
    // vboxLayout->addWidget(startBtn);
    // vboxLayout->addWidget(imageLabel);
    // vboxLayout->addWidget(resultLabel);
    // vboxLayout->addWidget(capBtn);
    // vboxLayout->addWidget(recBtn);

    leftWidget->setLayout(lvboxLayout);
    lvboxLayout->addWidget(video);
    lvboxLayout->addWidget(llistview);

    rightWidget->setLayout(rvboxLayout);
    // hboxLayout->addWidget(video);
    hboxLayout->addWidget(leftWidget);
    hboxLayout->addWidget(rightWidget);

    mainWidget->setLayout(hboxLayout);
    this->setCentralWidget(mainWidget);

    /* 信号连接 */
    // connect(startBtn,SIGNAL(clicked()),this,SLOT(onStart()));
    // connect(capBtn,SIGNAL(clicked()),this,SLOT(onCap()));
    // connect(recBtn,SIGNAL(clicked()),this,SLOT(onRec()));

    /* 线程任务 */
    t = new YUYVQThread(video);
	t->show_flag = true;
	t->start();

    ct = new CALCULATE_Thread(video);
	ct->start();

    pt = new PIC_Thread(rlistview);
	pt->start();

    ptinf = new PIC_INFO_Thread(llistview);
	ptinf->start();


    // QTimer *timer = new QTimer();
    // QObject::connect(timer, &QTimer::timeout, time_fun);
    // timer->start(1000); // 每1000毫秒（1秒）触发一次

    // dir = new QDir();
    // if(!dir->exists("images")){
    //     dir->mkdir("images");
    // }
    // dir->setCurrent("images");
}

MainWindow::~MainWindow()
{

}

//窗口关闭，进程停止
void MainWindow::closeEvent(QCloseEvent *event)
{
    t->show_flag = false;
    t->wait();
    event->accept();
    qDebug("thread exit");
}

void MainWindow::onStart()
{
    if(t->show_flag == false){
        t->show_flag = true;
        t->start();
        capBtn->setEnabled(true);
        qDebug("start");
    }
}

void MainWindow::onCap()
{
    QDateTime ntime = QDateTime::currentDateTime();
    QString dts = ntime.toString("yyMMddHHmmss");
    if(!video->img.isNull()){
        QString fileName = QCoreApplication::applicationDirPath() + "/images/" + dts + ".png";
        qDebug()<<"正在保存"<<fileName<<"图片，请稍后..."<<Qt::endl;
        video->img.save(fileName, "PNG", -1);
        qDebug()<<"保存完成！ "<<Qt::endl;
    }
}

void MainWindow::onRec()
{
    qDebug("开始录制");
}
