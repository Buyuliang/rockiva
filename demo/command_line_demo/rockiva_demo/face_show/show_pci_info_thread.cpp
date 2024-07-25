#include "show_pci_info_thread.h"

PIC_INFO_Thread::PIC_INFO_Thread(QListWidget *parent):QThread(parent){
     this->parent = (QListWidget*)parent;
}

void PIC_INFO_Thread::run(){
    QString times, first_time, last_time;
    while(1) {
        if ((update_pic /*&& old_update_pic_num != update_pic_num*/) /*|| update_time*/) {
            old_update_pic_num = update_pic_num;
            update_pic = 0;
            int i = 0;
            this->parent->clear();
            for (Node *next = node_head; next != NULL; next = next->next) {
                QString path = (QString)(next->faceInfo);
                // QFileInfo fileInfo(path);
                // QString fileName = fileInfo.fileName();
                QListWidgetItem *item = new QListWidgetItem;
                QFont font;
                font.setPointSize(7);
 
                // time =  QString(fileName.left(fileName.lastIndexOf('_')));
                item->setFont(font);
                times = QString::number(next->count);
                first_time = (QString)next->first_time;
                last_time = (QString)next->last_time;
                item->setText(QString("\nNumber of Times: ") + times + "\n\n" + 
                                        "First Visited Time: " + first_time + "\n\n" + 
                                        "last Visited Time: " + last_time + "\n");
                item->setIcon(QIcon(path));
                item->setTextAlignment(Qt::AlignLeft);
                this->parent->insertItem(i++, item);
                if (i == 6) {
                    break;
                }
            }
            // picresult.objNum = 0;
            // usleep(1000 * 1000);
        }
    }
}

PIC_INFO_Thread::~PIC_INFO_Thread(){
}