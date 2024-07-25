#ifndef SHOW_PCI_THREAD_H
#define SHOW_PCI_THREAD_H

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

#include "../modules/iva_app_face.h"

class PIC_Thread : public QThread
{
public:
    explicit PIC_Thread(QListWidget *parent = 0);
    ~PIC_Thread();

    QListWidget *parent;
public:
    void run();
};

#endif