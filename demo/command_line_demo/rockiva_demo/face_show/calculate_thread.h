#ifndef CALCULATE__THREAD_H
#define CALCULATE__THREAD_H

#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <QMutex>
#include <QListWidget>
#include <QScreen>
#include <QCoreApplication>
#include <QDir>
#include <QDateTime>
#include <QCloseEvent>
#include <QLabel>
#include <QListWidgetItem>
#include <QIcon>
#include <QFile>
#include <QStandardItemModel>

#include <stdio.h>
#include <stdlib.h>

#include "video_show.h"

#include "../iva_app.h"

#include "../utils/common_utils.h"
#include "../utils/image_buffer.h"
#include "../utils/image_utils.h"

class CALCULATE_Thread : public QThread
{
public:
    explicit CALCULATE_Thread(QWidget *parent = 0);
    ~CALCULATE_Thread();

    videoshow *parent;
    
public:
    void run();
    int process(char *buf, int len);
};

#endif