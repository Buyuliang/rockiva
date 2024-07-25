#include "iva_app_face.h"

#if IVA_APP_FACE_EN

#include "rockiva_face_api.h"
#include "utils/face_db.h"
#include "utils/image_utils.h"
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define RMOUT "rm -rf  "OUT_CAPTURE_PATH" "
#define PIC_UPDATE_TIME 1
#define recog_score 0.900
#define no_recog_score 0.200 

extern int update_pic = 0;
extern int update_pic_num = 0;
extern int old_update_pic_num = 0;
extern int update_time = 0;
extern RockIvaFaceDetResult detresult = {};
extern RockIvaFaceCapResults capresult = {};
extern RockIvaFaceIdInfo faceinforesult = {};
extern PicResult picresult = {};
extern Person *head = NULL;
extern Node *node_head = NULL;
float epsilon = 0.1;

Node* createNode(const char *ftime, const char *ltime, const char *faceInfo, int count) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory error\n");
        return NULL;
    }
    strcpy(newNode->first_time, ftime);
    strcpy(newNode->last_time, ltime);
    strcpy(newNode->faceInfo, faceInfo);
    newNode->count = count;
    newNode->next = NULL;
    return newNode;
}

void addNode(Node** head, const char *ftime, const char *ltime, const char *faceInfo, int count) {
    Node* newNode = createNode(ftime, ltime, faceInfo, count);
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }   
}

// // 删除节点
// void deleteNodes(struct Node** head, const char *ftime) {
//     struct Node* prev = NULL;
//     struct Node* curr = *head;

//     while (curr != NULL && !strcmp(curr->first_time, ftime)) {
//         prev = curr;
//         curr = curr->next;
//     }

//     if (curr == NULL) {
//         printf("Node with data not found.\n");
//         return;
//     }

//     if (prev == NULL) {
//         *head = curr->next;
//     } else {
//         prev->next = curr->next;
//     }
//     free(curr);
// }

// // 删除链表节点的函数
// void deleteNodes(struct Node** head_ref, const char *ftime) {
//     struct Node* temp = *head_ref, *prev;
    
//     // 如果头结点本身持有要删除的节点
//     if (temp != NULL && !strcmp(temp->first_time, ftime)) {
//         *head_ref = temp->next; // 改变头结点
//         free(temp); // 释放旧头结点
//         temp = head_ref;
//     }
//     prev = temp;
//     // 在链表中搜索要删除的节点，并跟踪前一个节点
//     while (temp != NULL && !strcmp(temp->first_time, ftime)) {
//         if (temp->next != NULL) { // 如果要删除的节点不是最后一个节点
//             prev->next = temp->next; // 将前一个节点的next指向要删除节点的下一个节点
//             free(temp); // 释放要删除的节点内存
//         } else {
//             prev->next = NULL;
//             break;
//         }
//         temp = prev->next; // 移动到下一个节点继续判断
//     } 
// }

// 删除链表中所有 data 值为 1 的节点
void deleteNodes(struct Node** head_ref, const char *ftime) {
    struct Node* temp = *head_ref;
    struct Node* prev = NULL;
    printf("\n\n tom test LINE:%d\n\n", __LINE__);

    // 在链表中遍历，删除所有 data 值为 1 的节点
    while (temp != NULL) {
        if (!strcmp(temp->first_time, ftime)) {
            // printf("\n\n tom test LINE:%d\n\n", __LINE__);
            if (prev == NULL) {
                *head_ref = temp->next; // 头结点为要删除节点，则改变头结点
            } else {
                prev->next = temp->next; // 否则，将前一个节点的 next 指向要删除节点的下一个节点
            }
            // printf("\n\n tom test LINE:%d\n\n", __LINE__);
            free(temp); // 释放要删除的节点内存
            // printf("\n\n tom test LINE:%d\n\n", __LINE__);
        } else {
            // printf("\n\n tom test LINE:%d\n\n", __LINE__);
            prev = temp; // 移动到下一个节点继续判断
            // printf("\n\n tom test LINE:%d\n\n", __LINE__);
        }
        // printf("\n\n tom test LINE:%d\n\n", __LINE__);
        if (prev != NULL) {
            temp = prev->next; // 移动到下一个节点继续判断
        }
        // printf("\n\n tom test LINE:%d\n\n", __LINE__);
    }
    // printf("\n\n tom test LINE:%d\n\n", __LINE__);
}

Node* merge(Node* first, Node* second) {
    if (first == NULL) return second; 
    if (second == NULL) return first; 
    if (first->count > second->count) { 
        first->next = merge(first->next, second); 
        return first; 
    } else { 
        second->next = merge(first, second->next); 
        return second; 
    } 
} 

void mergeSort(Node** headRef) { 
    if (*headRef == NULL || (*headRef)->next == NULL) 
        return; 
    Node* slow = *headRef; 
    Node* fast = (*headRef)->next; 
    while (fast != NULL && fast->next != NULL) { 
        slow = slow->next; 
        fast = fast->next->next; 
    } 
    Node* secondHalf = slow->next; 
    slow->next = NULL; 
    mergeSort(&secondHalf); 
    mergeSort(headRef); 
    *headRef = merge(*headRef, secondHalf); 
} 

void print_queue(Node *head) {
    Node *current = head;
    while (current != NULL) {
        printf("first_time: %s, last_time: %s, count: %d\n", current->first_time, current->last_time, current->count);
        current = current->next;
    }
}

char* substringAfterChar(char* str, char c) {
    char *ptr = NULL;
    ptr = strchr(str, c);
    if (ptr != NULL) {
        // 移动指针到字符之后，然后复制从该字符后的字符串
        ptr++;
        char* newStr = malloc(strlen(ptr) + 1); // 分配内存空间
        strcpy(newStr, ptr); // 复制字符串
        return newStr; // 返回新字符串
    } else {
        return NULL; // 如果字符未找到，返回NULL
    }
}

void extract_string(char* str, char* result, int start, int end) {
    int len = strlen(str);
    if (start < 0 || end > len) {
        printf("Invalid start or end index.\n");
        return;
    }
    int i;
    for (i = 0; i < start; i++) {
        if (str[i] == '\0') {
            break;
        }
    }
    int j;
    for (j = end; j < len; j++) {
        if (str[j] == '\0') {
            break;
        }
    }
    if (i >= j) {
        result[0] = '\0';
        return;
    }
    int k = 0;
    for (k = i; k < j; k++) {
        result[k - i] = str[k];
    }
    result[k - i] = '\0';
}

void crop_image(unsigned char* image, char* image_crop, int width, int height, int xmin, int ymin, int width_crop, int height_crop, char *savepath) {
    // 计算裁剪区域的行列索引
    int row_start = ymin;
    int row_end = ymin + height_crop;
    int col_start = xmin;
    int col_end = xmin + width_crop;

    // 检查裁剪区域是否超出图像边界
    if (row_start < 0) row_start = 0;
    if (row_end > height) row_end = height;
    if (col_start < 0) col_start = 0;
    if (col_end > width) col_end = width;

    // 计算裁剪后的图像大小
    int size_crop = (col_end - col_start) * (row_end - row_start);

    // 分配内存以存储裁剪后的图像
    // unsigned char* image_crop = (unsigned char*)malloc(size_crop * 3 * sizeof(unsigned char));

    // 复制裁剪区域的数据到新图像中
    for (int i = row_start; i < row_end; i++) {
        for (int j = col_start; j < col_end; j++) {
            int index = (i - row_start) * (col_end - col_start) + (j - col_start);
            image_crop[index * 3] = image[i * width * 3 + j * 3]; // R
            image_crop[index * 3 + 1] = image[i * width * 3 + j * 3 + 1]; // G
            image_crop[index * 3 + 2] = image[i * width * 3 + j * 3 + 2]; // B
        }
    }

    // 输出裁剪后的图像大小
    // printf("Cropped image size: %dx%d\n", col_end - col_start, row_end - row_start);
}

void FaceDetResultCallback(const RockIvaFaceDetResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    // IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    // char save_path[PATH_MAX] = {0};
    // char time_str[20]; // 用于存储格式化后的时间字符串
    // time_t current_time;
    // struct tm *local_time;
    // update_pic = 0;

    memcpy(&detresult, result, sizeof(RockIvaFaceDetResult));
//     printf("FaceDetResultCallback channelId %d frameId %d face num: %d format:%d\n", detresult.channelId, detresult.frameId, detresult.objNum);
    
//     if (status == ROCKIVA_SUCCESS) {}

//     RockIvaImage* frame = &(result->frame);
//     printf("FaceDetResultCallback channelId %d frameId %d face num: %d format:%d\n", result->channelId, result->frameId, result->objNum, frame->info.format);

//     // draw result
//     // 拷贝一份避免画框改写原图数据
//     RockIvaImage drawImage;
//     RockIvaImage drawcutImage;
//     memset(&drawImage, 0, sizeof(RockIvaImage));
//     memset(&drawcutImage, 0, sizeof(RockIvaImage));
//     if (frame->info.format == ROCKIVA_IMAGE_FORMAT_RGB888) {
//         ROCKIVA_IMAGE_Clone(frame, &drawImage);
//     } else {
//         drawImage.info.width = frame->info.width;
//         drawImage.info.height = frame->info.height;
//         drawImage.info.format = ROCKIVA_IMAGE_FORMAT_RGB888;
//         ROCKIVA_IMAGE_Convert(frame, &drawImage);
//     }

//     drawcutImage.info.format = ROCKIVA_IMAGE_FORMAT_RGB888;
//     if (old_update_pic_num == result->objNum) {
//         goto go_out;
//     }
//     old_update_pic_num = update_pic_num;
//     update_pic_num = result->objNum;
//     picresult.objNum = result->objNum;
//     for (int i = 0; i < result->objNum; i++) {
//         RockIvaFaceInfo* face = &result->faceInfo[i];
//         int re_obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.width,  detresult.faceInfo[i].faceRect.topLeft.x);
//         int re_obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.height, detresult.faceInfo[i].faceRect.topLeft.y);
//         int re_obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.width,  detresult.faceInfo[i].faceRect.bottomRight.x);
//         int re_obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(detresult.frame.info.height, detresult.faceInfo[i].faceRect.bottomRight.y);
//         printf("(%d, %d) (%d, %d)  objId:%d\n", re_obj_rect_x1, re_obj_rect_y1, re_obj_rect_x2, re_obj_rect_y2, detresult.faceInfo[i].objId);
//         int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->faceRect.topLeft.x);
//         int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->faceRect.topLeft.y);
//         int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->faceRect.bottomRight.x);
//         int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->faceRect.bottomRight.y);
//         drawcutImage.info.width = obj_rect_x2 - obj_rect_x1 + 1;
//         drawcutImage.info.height = obj_rect_y2 - obj_rect_y1 + 1;
//         drawcutImage.dataAddr = malloc(frame->info.width * frame->info.height * 3);
//         if (drawcutImage.dataAddr == NULL) {
//             printf("malloc addr fail!\n");
//             goto go_out;
//         }
//         // 获取当前系统时间
//         current_time = time(NULL);
//         // 将当前系统时间转换为本地时间
//         local_time = localtime(&current_time);
//         // 将本地时间格式化为字符串
//         strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
//         snprintf(save_path, PATH_MAX,
//                 "%s/%s_%d.jpg",
//                 OUT_CAPTURE_PATH, time_str, i);
//         printf("save_path:%s\n", save_path);
//         crop_image(drawImage.dataAddr, drawcutImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1, drawcutImage.info.width, drawcutImage.info.height, save_path);
//         SaveImage(save_path, &(drawcutImage));
//         strcpy(picresult.faceInfo[i], save_path);
//         // DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
//                         //  obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);
//         char text[256] = {0};
//         snprintf(text, 64, "%d-%d_%d_%d_%d_%d_%d_%d_%d_%d_%d", face->objId, face->faceQuality.score,
//                  face->faceQuality.faceScore, face->faceQuality.clarity, face->faceQuality.angle.pitch,
//                  face->faceQuality.angle.roll, face->faceQuality.angle.yaw, face->faceQuality.eyesScore,
//                  face->faceQuality.noseScore, face->faceQuality.mouthScore, face->mask);
//         // DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1, obj_rect_y1 - 22, 10,
//                     // COLOR_BLUE);
//         for (int i = 0; i < face->landmarksNum; i++) {
//             int px = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->landmarks[i].x);
//             int py = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->landmarks[i].y);
//             // DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, px, py, 1, 1, COLOR_BLUE, 2);
//         }
//         free(drawcutImage.dataAddr);
//     }
//     update_pic = 1;
//     // char save_path[PATH_MAX] = {0};
//     // snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
//     // printf("%lu before write image\n", GetTimeStampMS());
//     // SaveImage(save_path, &(drawImage));
//     // printf("%lu after write image\n", GetTimeStampMS());
//     // printf("save detect result to: %s\n", save_path);
// go_out:
//     ROCKIVA_IMAGE_Release(&drawImage);
}

void FaceAnalyseResultCallback(const RockIvaFaceCapResults* result, const RockIvaExecuteStatus status, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    RockIvaFaceTaskParams* faceParams = iva_ctx->faceData;
    char time_str[20]; // 用于存储格式化后的时间字符串
    time_t current_time;
    struct tm *local_time;
    int update_face_feature = 1;
    update_pic = 0;
    int count = 1;

    memcpy(&capresult, result, sizeof(RockIvaFaceCapResults));
    printf("FaceAnalyseResultCallback channelId=%d frameId=%d type=%d num=%d\n", result->channelId, result->frameId,
        result->faceCapFrameType, result->num);

    if (status == ROCKIVA_SUCCESS) {
        picresult.objNum = result->num;
        for (int i = 0; i < result->num; i++) {
            RockIvaFaceCapResult* face_result = &(result->faceResults[i]);
            printf("\ncapture face id=%d box=(%d %d %d %d) mask=%d\n", face_result->faceInfo.objId,
                face_result->faceInfo.faceRect.topLeft.x, face_result->faceInfo.faceRect.topLeft.y,
                face_result->faceInfo.faceRect.bottomRight.x, face_result->faceInfo.faceRect.bottomRight.y,
                face_result->faceAnalyseInfo.faceAttr.mask);
            char faceId[ROCKIVA_FACE_INFO_SIZE_MAX] = {0};
            float score = 0.0;
            // 设置人脸特征数据
            RockIvaFaceIdInfo faceinf;
            if (result->faceCapFrameType != ROCKIVA_FACE_CAP_TYPE_FORENOTICE) {
                // 人脸识别
                if (faceParams->faceTaskType.faceRecognizeEnable == 1) {
                    if (face_result->qualityResult == ROCKIVA_FACE_QUALITY_OK) {
                        RockIvaFaceSearchResults search_results;
                        ROCKIVA_FACE_SearchFeature("face", face_result->faceAnalyseInfo.feature,
                                                face_result->faceAnalyseInfo.featureSize, 1, 50, &search_results);
                        for (int i = 0; i < search_results.num; i++) {
                            printf("\nserch result %d face_info=%s score=%f\n\n", i, search_results.faceIdScore[i].faceIdInfo,
                                search_results.faceIdScore[i].score);
                            if (i == 0 && fabs(search_results.faceIdScore[0].score - (float)0.700) < (float)0.3) {
                                // strncpy(faceId, search_results.faceIdScore[0].faceIdInfo, ROCKIVA_FACE_INFO_SIZE_MAX);
                                // score = search_results.faceIdScore[0].score;
                                // extract_string(search_results.faceIdScore[0].faceIdInfo, faceinforesult.first_time, 0, 18);
                                // RockIvaFaceIdInfo faceinfo;
                                // // void *feature_data = (void *)malloc(face_result->faceAnalyseInfo.featureSize);
                                
                                char *ptr = substringAfterChar(search_results.faceIdScore[0].faceIdInfo, '_');
                                if (ptr == NULL) {
                                    count = 1;
                                } else {
                                    count = atoi(ptr) + 1;
                                }
                                faceinforesult.count = count;
                                printf("xxxxxxxx ----------- count : %d\n\n",count);
                                // strncpy(faceinf.faceIdInfo, search_results.faceIdScore[0].faceIdInfo, sizeof(search_results.faceIdScore[0].faceIdInfo));
                                memset(faceinforesult.first_time, 0, sizeof(faceinforesult.first_time));
                                // extract_string(search_results.faceIdScore[0].faceIdInfo, faceinforesult.first_time, 0, 16);
                                strncat(faceinforesult.first_time, search_results.faceIdScore[0].faceIdInfo, 19);
                                printf("xxxxxxxx ----------- first_time : %s\n\n", faceinforesult.first_time);
                                // extract_string(search_results.faceIdScore[0].faceIdInfo, faceinf.faceIdInfo, 0, 18);
                                memset(faceinf.faceIdInfo, 0, sizeof(faceinf.faceIdInfo));
                                // strncat(faceinf.faceIdInfo, search_results.faceIdScore[0].faceIdInfo, 19);
                                snprintf(faceinf.faceIdInfo, ROCKIVA_FACE_INFO_SIZE_MAX, "%s_%d", faceinforesult.first_time, count);
                                // // memcpy(feature_data, (void*)face_result->faceAnalyseInfo.feature, face_result->faceAnalyseInfo.featureSize);
                                // ROCKIVA_FACE_FeatureLibraryControl("face", ROCKIVA_FACE_FEATURE_DELETE, &faceinfo, 1, NULL, 0);
                                update_face_feature = 0;
                            }
                            if (fabs(search_results.faceIdScore[i].score - (float)0.700) < (float)0.3) {
                                char tmp[ROCKIVA_FACE_INFO_SIZE_MAX];
                                RockIvaFaceIdInfo faceinfo;
                                memset(tmp, 0, ROCKIVA_FACE_INFO_SIZE_MAX);
                                strncat(tmp, search_results.faceIdScore[i].faceIdInfo, 19);
                                printf("xxxxxxxx ----------- deleteNode : %s\n\n", tmp);
                                deleteNodes(&node_head, tmp);
                                strncpy(faceinfo.faceIdInfo, search_results.faceIdScore[i].faceIdInfo, sizeof(faceinfo.faceIdInfo));
                                ROCKIVA_FACE_FeatureLibraryControl("face", ROCKIVA_FACE_FEATURE_DELETE, &faceinfo, 1, NULL, 0);
                            }
                        }
                        // if (search_results.num == 0 /*&& search_results.faceIdScore[0].score < 0.1 || !update_face_feature*/) {
                            face_data face;
                            IvaImageBuf* image_buf = (IvaImageBuf*)result->frame.extData;
                            char save_path[PATH_MAX];
                            memset(save_path, 0, PATH_MAX);
                            int captureImageW = face_result->captureImage.info.width;
                            int captureImageH = face_result->captureImage.info.height;
                            int face_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(captureImageW, face_result->faceRectOnCaptureImage.topLeft.x);
                            int face_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(captureImageH, face_result->faceRectOnCaptureImage.topLeft.y);
                            int face_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(captureImageW, face_result->faceRectOnCaptureImage.bottomRight.x);
                            int face_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(captureImageH, face_result->faceRectOnCaptureImage.bottomRight.y);
                            current_time = time(NULL);  // 获取当前系统时间
                            local_time = localtime(&current_time);  // 将当前系统时间转换为本地时间
                            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);  // 将本地时间格式化为字符串
                            if (update_face_feature || (search_results.num == 0 || fabs(search_results.faceIdScore[0].score - no_recog_score) <= epsilon)) {
                                count = 1;
                                faceinforesult.count = count;
                            }
                            // if (search_results.num == 0 || search_results.faceIdScore[0].score < 0.3) {
                            //     memset(faceinforesult.first_time, 0, sizeof(faceinforesult.first_time));
                            //     strncpy(faceinforesult.first_time, time_str, sizeof(faceinforesult.first_time));
                            // }
                            snprintf(save_path, PATH_MAX,
                                    "%s/%s_%d.png",
                                    OUT_CAPTURE_PATH, time_str, i);

                            printf("save capture to %s\n", save_path);
                            strcpy(picresult.faceInfo[i], save_path);
                            void *feature_data = (void *)malloc(face_result->faceAnalyseInfo.featureSize);
                            if (update_face_feature) {
                                memset(faceinforesult.first_time, 0, sizeof(faceinforesult.first_time));
                                snprintf(faceinforesult.first_time, ROCKIVA_FACE_INFO_SIZE_MAX, "%s", time_str);
                                memset(faceinf.faceIdInfo, 0, ROCKIVA_FACE_INFO_SIZE_MAX);
                                // strncpy(faceinf.faceIdInfo, time_str, ROCKIVA_FACE_INFO_SIZE_MAX);
                                snprintf(faceinf.faceIdInfo, ROCKIVA_FACE_INFO_SIZE_MAX, "%s_%d", time_str, count);
                            }
                            snprintf(faceinforesult.last_time, ROCKIVA_FACE_INFO_SIZE_MAX, "%s", time_str);

                            addNode(&node_head, faceinforesult.first_time, faceinforesult.last_time, save_path, faceinforesult.count);
                            mergeSort(&node_head);
                            print_queue(node_head);
                            memcpy(feature_data, (void*)face_result->faceAnalyseInfo.feature, face_result->faceAnalyseInfo.featureSize);
                            ROCKIVA_FACE_FeatureLibraryControl("face", ROCKIVA_FACE_FEATURE_INSERT, &faceinf, 1, feature_data, face_result->faceAnalyseInfo.featureSize);
                            free(feature_data);
                            SaveImage(save_path, &(face_result->captureImage));
                            update_pic = 1;
                        // }
                    }
                }
            }
        }
    }
}

int load_face_lib(const char* face_db_path, const char* face_lib_name)
{
    int ret;
    sqlite3* db;
    open_db(face_db_path, &db);

    int face_num_;

    ret = get_face_count(db, &face_num_);
    face_data face_lib_[face_num_];

    get_all_face(db, face_lib_, &face_num_);

    RockIvaFaceIdInfo face_info[face_num_];
    int feature_size = face_lib_->size;
    printf("feature_size=%d\n", feature_size);
    void* feature_data = (void*)malloc(face_num_ * feature_size);
    for (int i = 0; i < face_num_; i++) {
        strncpy(face_info[i].faceIdInfo, face_lib_[i].info, ROCKIVA_FACE_INFO_SIZE_MAX);
        memcpy(feature_data + i * feature_size, face_lib_[i].feature, feature_size);
    }

    release_face_data(face_lib_, face_num_);

    ROCKIVA_FACE_FeatureLibraryControl(face_lib_name, ROCKIVA_FACE_FEATURE_INSERT, face_info, face_num_, feature_data,
                                       feature_size);

    close_db(db);
    free(feature_data);

    printf("load face num %d\n", face_num_);

    return 0;
}

int InitIvaFace(IvaAppContext* ctx)
{
    RockIvaFaceTaskParams* faceParams = malloc(sizeof(RockIvaFaceTaskParams));
    memset(faceParams, 0, sizeof(RockIvaFaceTaskParams));

    faceParams->faceTaskType.faceCaptureEnable = 1;
    faceParams->faceTaskType.faceAttributeEnable = 1;
    faceParams->faceTaskType.faceRecognizeEnable = 1;
    faceParams->faceTaskType.faceLandmarkEnable = 0;

    // 开启戴口罩人脸抓拍
    faceParams->faceCaptureRule.captureWithMask = 0;

    // 最优质量抓拍（人脸消失或超时选一张质量最高人脸）
    faceParams->faceCaptureRule.optType = ROCKIVA_FACE_OPT_BEST;
    // 超时时间，如果一个人脸超过设定时间还未消失也会触发抓拍
    faceParams->faceCaptureRule.optBestOverTime = 10000;

    // 快速抓拍（人脸质量超过设定阈值就触发抓拍）
    faceParams->faceCaptureRule.optType = ROCKIVA_FACE_OPT_FAST;
    faceParams->faceCaptureRule.faceQualityThrehold = 60;

    // 人脸过滤配置
    // 最低人脸质量分阈值，小于阈值将过滤
    faceParams->faceCaptureRule.qualityConfig.minScore = 70;
    // 遮挡阈值，小于阈值将过滤
    faceParams->faceCaptureRule.qualityConfig.minEyescore = 70;
    faceParams->faceCaptureRule.qualityConfig.minMouthScore = 70;

    // 抓拍小图配置
    faceParams->faceCaptureRule.faceCapacity.maxCaptureNum = 40;
    faceParams->faceCaptureRule.captureImageConfig.mode = 1;
    faceParams->faceCaptureRule.captureImageConfig.originImageType = 1;
    faceParams->faceCaptureRule.captureImageConfig.resizeMode = 1;
    faceParams->faceCaptureRule.captureImageConfig.imageInfo.width = 240;
    faceParams->faceCaptureRule.captureImageConfig.imageInfo.height = 240;
    // faceParams->faceCaptureRule.captureImageConfig.imageInfo.width = 240;
    // faceParams->faceCaptureRule.captureImageConfig.imageInfo.height = 320;
    faceParams->faceCaptureRule.captureImageConfig.imageInfo.format = ROCKIVA_IMAGE_FORMAT_RGB888;
    faceParams->faceCaptureRule.captureImageConfig.alignWidth = 16;

    faceParams->faceCaptureRule.captureImageConfig.expand.up = 1;
    faceParams->faceCaptureRule.captureImageConfig.expand.down = 1;
    faceParams->faceCaptureRule.captureImageConfig.expand.left = 1;
    faceParams->faceCaptureRule.captureImageConfig.expand.right = 1;

    // 检测区域配置
    faceParams->faceCaptureRule.detectAreaEn = 0;
    faceParams->faceCaptureRule.detectArea.pointNum = 4;
    faceParams->faceCaptureRule.detectArea.points[0].x = 1000;
    faceParams->faceCaptureRule.detectArea.points[0].y = 1000;
    faceParams->faceCaptureRule.detectArea.points[1].x = 9000;
    faceParams->faceCaptureRule.detectArea.points[1].y = 1000;
    faceParams->faceCaptureRule.detectArea.points[2].x = 9000;
    faceParams->faceCaptureRule.detectArea.points[2].y = 9000;
    faceParams->faceCaptureRule.detectArea.points[3].x = 1000;
    faceParams->faceCaptureRule.detectArea.points[3].y = 9000;

    RockIvaFaceCallback callback;
    callback.detCallback = FaceDetResultCallback;
    callback.analyseCallback = FaceAnalyseResultCallback;
    RockIvaRetCode ret = ROCKIVA_FACE_Init(ctx->handle, faceParams, callback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_FACE_Init error %d\n", ret);
        return -1;
    }
    if (faceParams->faceTaskType.faceRecognizeEnable == 1) {
        // load_face_lib(FACE_DATABASE_PATH, "face");
    }

    ctx->faceData = faceParams;
    return 0;
}

int ReleaseIvaFace(IvaAppContext* ctx)
{
    if (ctx->faceData != NULL) {
        free(ctx->faceData);
        ctx->faceData = NULL;
    }
    return 0;
}

#endif