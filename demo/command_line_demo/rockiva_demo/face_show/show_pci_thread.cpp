#include "show_pci_thread.h"

PIC_Thread::PIC_Thread(QListWidget *parent):QThread(parent){
     this->parent = (QListWidget*)parent;
}

void PIC_Thread::run(){
    QString gender, emotion, eyeGlass, smile, time;
    while(1) {
        if ((update_pic /*&& old_update_pic_num != update_pic_num*/) /*|| update_time*/) {
            old_update_pic_num = update_pic_num;
            update_pic = 0;
            for (int i = 0; i < picresult.objNum; i++) {
                QString path = (QString)(picresult.faceInfo[i]);
                QFileInfo fileInfo(path);
                QString fileName = fileInfo.fileName();
                QListWidgetItem *item = new QListWidgetItem;
                QFont font;
                font.setPointSize(11);
                switch(capresult.faceResults[0].faceAnalyseInfo.faceAttr.gender) {
                    case 1:
                        gender = "Male";
                        break;
                    case 2:
                        gender = "Female";
                        break;
                    default:
                        gender = "Unknown";
                        break;
                }
                // switch(capresult.faceResults[0].faceAnalyseInfo.faceAttr.emotion) {
                //     case 1:
                //         emotion = "ANGRY";
                //         break;
                //     case 2:
                //         emotion = "CALM";
                //         break;
                //     case 3:
                //         emotion = "CONFUSED";
                //         break;
                //     case 4:
                //         emotion = "DISGUST";
                //         break;
                //     case 5:
                //         emotion = "HAPPY";
                //         break;
                //     case 6:
                //         emotion = "SAD";
                //         break;
                //     case 7:
                //         emotion = "SCARED";
                //         break;
                //     case 8:
                //         emotion = "SURPRISED";
                //         break;
                //     case 9:
                //         emotion = "SQUINT";
                //         break;
                //     case 10:
                //         emotion = "SCREAM";
                //         break;
                //     default:
                //         emotion = "UNKNOWN";
                //         break;
                // }
                switch(capresult.faceResults[0].faceAnalyseInfo.faceAttr.eyeGlass) {
                    case 1:
                        eyeGlass = "No";
                        break;
                    case 2:
                        eyeGlass = "Yes";
                        break;
                    case 3:
                        eyeGlass = "Yes";
                        break;
                    default:
                        eyeGlass = "Unknown";
                        break;
                }
                switch(capresult.faceResults[0].faceAnalyseInfo.faceAttr.smile) {
                    case 1:
                        smile = "Yes";
                        break;
                    case 2:
                        smile = "No";
                        break;
                    default:
                        smile = "Unknown";
                        break;
                }
                time =  QString(fileName.left(fileName.lastIndexOf('_')));
                item->setFont(font);
                item->setText(QString("\n   Gender: ") + gender + "\n\n" + 
                                        "   Smile: " + smile + "\n\n" + 
                                        "   Glasses: " + eyeGlass + "\n\n" + 
                                        "   Time: " + time + "\n");  // 去掉文件扩展名
                item->setIcon(QIcon(path));
                this->parent->insertItem(0, item);
            }
            picresult.objNum = 0;
            // usleep(1000 * 1000);
        }
    }
}

PIC_Thread::~PIC_Thread(){
}