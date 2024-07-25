/****************************************************************************
 *
 *    Copyright (c) 2022 by Rockchip Corp.  All rights reserved.
 *
 *    The material in this file is confidential and contains trade secrets
 *    of Rockchip Corporation. This is proprietary information owned by
 *    Rockchip Corporation. No part of this work may be disclosed,
 *    reproduced, copied, transmitted, or used in any way for any purpose,
 *    without the express written permission of Rockchip Corporation.
 *
 *****************************************************************************/
#include <dirent.h>
#include <linux/limits.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include "rockiva_face_api.h"

#include "iva_app_ctx.h"
#include "utils/face_db.h"
#include "utils/image_buffer.h"
#include "utils/image_utils.h"

#define FACE_LIBRARY_NAME "face"

void rockiva_face_detection_callback(const RockIvaFaceDetResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;

    printf("FaceDetResultCallback channelId %d frameId %d face num: %d\n", result->channelId, result->frameId, result->objNum);

    if (status == ROCKIVA_SUCCESS) {}

    RockIvaImage* frame = &(result->frame);

    // draw result
    // 拷贝一份避免画框改写原图数据
    RockIvaImage drawImage;
    memset(&drawImage, 0, sizeof(RockIvaImage));
    if (frame->info.format == ROCKIVA_IMAGE_FORMAT_RGB888) {
        ROCKIVA_IMAGE_Clone(frame, &drawImage);
    } else {
        drawImage.info.width = frame->info.width;
        drawImage.info.height = frame->info.height;
        drawImage.info.format = ROCKIVA_IMAGE_FORMAT_RGB888;
        ROCKIVA_IMAGE_Convert(frame, &drawImage);
    }

    for (int i = 0; i < result->objNum; i++) {
        RockIvaFaceInfo* face = &result->faceInfo[i];
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->faceRect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->faceRect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->faceRect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->faceRect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                         obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);
        char text[256] = {0};
        snprintf(text, 64, "%d-%d_%d_%d_%d_%d_%d_%d_%d_%d_%d", face->objId, face->faceQuality.score,
                 face->faceQuality.faceScore, face->faceQuality.clarity, face->faceQuality.angle.pitch,
                 face->faceQuality.angle.roll, face->faceQuality.angle.yaw, face->faceQuality.eyesScore,
                 face->faceQuality.noseScore, face->faceQuality.mouthScore, face->mask);
        DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1, obj_rect_y1 - 22, 10,
                    COLOR_BLUE);
        for (int i = 0; i < face->landmarksNum; i++) {
            int px = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, face->landmarks[i].x);
            int py = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, face->landmarks[i].y);
            DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, px, py, 1, 1, COLOR_BLUE, 2);
        }
    }
    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
    printf("%lu before write image\n", GetTimeStampMS());
    SaveImage(save_path, &(drawImage));
    printf("%lu after write image\n", GetTimeStampMS());
    printf("save detect result to: %s\n", save_path);

    ROCKIVA_IMAGE_Release(&drawImage);
}

void rockiva_face_analyse_callback(const RockIvaFaceCapResults* result, const RockIvaExecuteStatus status,
                                   void* userdata)
{
    printf("rockiva_face_analyse_callback frameId=%d status=%d num=%d\n",
        result->frameId, status, result->num);

    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;

    if (status == ROCKIVA_SUCCESS && result->num > 0) {
        IvaImageBuf* image_buf = (IvaImageBuf*)result->frame.extData;

        int feature_size = result->faceResults[0].faceAnalyseInfo.featureSize;
        int face_num = result->num;

        for (int i = 0; i < face_num; i++) {
            RockIvaFaceCapResult* face_result = &(result->faceResults[i]);
            printf("face %d quality result: %d\n", face_result->faceInfo.objId, face_result->qualityResult);
            if (face_result->qualityResult != ROCKIVA_FACE_QUALITY_OK) {
                continue;
            }

            RockIvaFaceSearchResults search_result;
            ROCKIVA_FACE_SearchFeature(FACE_LIBRARY_NAME, face_result->faceAnalyseInfo.feature, feature_size, 1, 5, &search_result);

            RockIvaFaceInfo* faceInfo = &face_result->faceInfo;
            printf("search result for face %d box=(%d %d %d %d)\n", faceInfo->objId,
                faceInfo->faceRect.topLeft.x, faceInfo->faceRect.topLeft.y,
                faceInfo->faceRect.bottomRight.x, faceInfo->faceRect.bottomRight.y);
            for (int i = 0; i < search_result.num; i++) {
                printf(" serch result %d face_info=%s score=%f\n", i,
                    search_result.faceIdScore[i].faceIdInfo, search_result.faceIdScore[i].score);
            }
        }
    }
}

void rockiva_image_release_callback(const RockIvaReleaseFrames* releaseFrames, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    printf("FrameReleaseCallback count=%d\n", releaseFrames->count);
    for (int i = 0; i < releaseFrames->count; i++) {
        IvaImageBuf* img_buf = releaseFrames->frames[i].extData;
        printf("release img_buf=%p\n", img_buf);
        if (img_buf != NULL) {
            free(img_buf->image.dataAddr);
            if (img_buf->extraInfo != NULL) {
                free(img_buf->extraInfo);
            }
            RetureImageBuffer(iva_ctx->imgBufManager, img_buf);
        } else {
            printf("FrameReleaseCallback: can not find image buffer for frameId=%d\n", releaseFrames->frames[i].frameId);
        }
    }
}

int triger_recog(IvaAppContext* iva_ctx, const char* image_path, const char* image_name)
{
    RockIvaRetCode ret;
    int reti;
    int width = 0;
    int height = 0;
    int channel = 0;
    static int frame_index = 0;
    frame_index++;

    IvaImageBuf* image_buf = AcquireImageBuffer(iva_ctx->imgBufManager, 1000);

    reti = ReadImage(image_path, &image_buf->image);
    if (reti != 0) {
        return -1;
    }

    // 设置图片名
    image_buf->extraInfo = malloc(strlen(image_name)+1);
    strcpy(image_buf->extraInfo, image_name);

    image_buf->image.frameId = frame_index;
    image_buf->image.extData = image_buf;

    ret = ROCKIVA_PushFrame(iva_ctx->handle, &(image_buf->image), NULL);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_PushFrame fail ret=%d\n", ret);
        return -1;
    }

    return 0;
}

int load_face_lib(const char *face_db_path, const char *face_lib_name) {
    printf("loading face database: %s lib_name:%s\n", face_db_path, face_lib_name);

    int ret;
    sqlite3* db;
    open_db(face_db_path, &db);

    int face_num_;

    ret = get_face_count(db, &face_num_);
    face_data face_lib_[face_num_];

    get_all_face(db, face_lib_, &face_num_);
    printf("face num: %d\n", face_num_);

    if (face_num_ > 0) {
        RockIvaFaceIdInfo face_info[face_num_];
        int feature_size = face_lib_->size;
        printf("feature_size=%d\n", feature_size);
        void *feature_data = (void *)malloc(face_num_*feature_size);
        for (int i = 0; i < face_num_; i++) {
            strncpy(face_info[i].faceIdInfo, face_lib_[i].info, ROCKIVA_FACE_INFO_SIZE_MAX);
            memcpy(feature_data+i*feature_size, face_lib_[i].feature, feature_size);
        }

        release_face_data(face_lib_, face_num_);

        ROCKIVA_FACE_FeatureLibraryControl(face_lib_name, ROCKIVA_FACE_FEATURE_INSERT, face_info, face_num_, feature_data, feature_size);
        free(feature_data);
    }
    close_db(db);

    printf("load face num %d\n", face_num_);

    return 0;
}

int main(int argc, char** argv)
{
    char* recog_image_dir = argv[1];

    printf("recog_image_dir: %s\n", recog_image_dir);

    RockIvaRetCode ret;

    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));

    iva_ctx.imgBufManager = CreateImageBufferManager(5);

    RockIvaInitParam* commonParams = &(iva_ctx.commonParams);

    // 配置模型路径
    snprintf(commonParams->modelPath, ROCKIVA_PATH_LENGTH, MODEL_DATA_PATH);

    // 配置授权信息
    if (access(LICENSE_PATH, R_OK) == 0) {
        char *license_key;
        int license_size;
        license_size = ReadDataFile(LICENSE_PATH, &license_key);
        if (license_key != NULL && license_size > 0) {
            commonParams->license.memAddr = license_key;
            commonParams->license.memSize = license_size;
        }
    }

    commonParams->detModel = ROCKIVA_DET_MODEL_CLS7;

    // 非连续视频流的图像使用PICTURE模式
    ROCKIVA_Init(&iva_ctx.handle, ROCKIVA_MODE_PICTURE, commonParams, &iva_ctx);

    if (commonParams->license.memAddr != NULL) {
        free(commonParams->license.memAddr);
    }

    // 配置人脸回调
    RockIvaFaceCallback callback;
    callback.detCallback = rockiva_face_detection_callback;
    callback.analyseCallback = rockiva_face_analyse_callback;

    // 配置为正常模式，人脸识别使能
    RockIvaFaceTaskParams faceParams;
    memset(&faceParams, 0, sizeof(RockIvaFaceTaskParams));
    faceParams.mode = ROCKIVA_FACE_MODE_NORMAL;
    faceParams.faceTaskType.faceRecognizeEnable = 1;

    // 人脸功能初始化
    ret = ROCKIVA_FACE_Init(iva_ctx.handle, &faceParams, callback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_FACE_Init error %d\n", ret);
        return -1;
    }

    // 配置帧释放回调
    ROCKIVA_SetFrameReleaseCallback(iva_ctx.handle, rockiva_image_release_callback);

    load_face_lib(FACE_DATABASE_PATH, FACE_LIBRARY_NAME);

    DIR* d = opendir(recog_image_dir);
    struct dirent* entry;
    if (d == NULL) {
        printf("register image dirent open fail!");
        return -1;
    }
    char image_path[PATH_MAX];

    // 遍历文件夹下的所有图片
    while ((entry = readdir(d)) != NULL) {
        char* image_name = entry->d_name;
        if (strcmp(image_name, ".") == 0 || strcmp(image_name, "..") == 0) {
            continue;
        }
        memset(image_path, 0, PATH_MAX);
        snprintf(image_path, PATH_MAX, "%s/%s", recog_image_dir, image_name);
        printf("recog image: %s\n", image_path);
        // 触发识别
        triger_recog(&iva_ctx, image_path, image_name);
    }
    closedir(d);

    usleep(3 * 1000 * 1000);

    ROCKIVA_Release(iva_ctx.handle);

    DestroyImageBufferManager(iva_ctx.imgBufManager);

    printf("iva app exit\n");
    return 0;
}
