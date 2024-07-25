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

sqlite3* db;

void rockiva_face_analyse_callback(const RockIvaFaceCapResults* result, const RockIvaExecuteStatus status,
                                   void* userdata)
{
    printf("rockiva_face_analyse_callback frameId=%d status=%d num=%d\n",
        result->frameId, status, result->num);

    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;

    IvaImageBuf* image_buf = (IvaImageBuf*)result->frame.extData;
    if (status == ROCKIVA_SUCCESS && result->num > 0) {
        // 正常注册图像只有一个人脸结果
        RockIvaFaceCapResult* face_result = &(result->faceResults[0]);
        if (face_result->qualityResult == ROCKIVA_FACE_QUALITY_OK) {
            face_data face;
            // 设置人脸特征数据
            face.feature = (void*)face_result->faceAnalyseInfo.feature;
            face.size = face_result->faceAnalyseInfo.featureSize;
            // 设置的人脸ID信息
            snprintf(face.id, FACE_ID_MAX_SIZE, image_buf->extraInfo);
            printf("insert face %s to database\n", face.id);
            // 插入人脸数据库
            insert_face(db, &face);
        } else {
            printf("import face fail(%d) %s\n", face_result->qualityResult, image_buf->extraInfo);
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
            free(img_buf->extraInfo);
            img_buf->extraInfo = NULL;
            free(img_buf);
        } else {
            printf("FrameReleaseCallback: can not find image buffer for frameId=%d\n", releaseFrames->frames[i].frameId);
        }
    }
}

int triger_recog(IvaAppContext* iva_ctx, const char* image_path, const char* faceId)
{
    RockIvaRetCode ret;
    int reti;
    int width = 0;
    int height = 0;
    int channel = 0;
    static int frame_index = 0;
    frame_index++;

    IvaImageBuf* image_buf = malloc(sizeof(IvaImageBuf));
    memset(image_buf, 0, sizeof(IvaImageBuf));

    reti = ReadImage(image_path, &image_buf->image);
    if (reti != 0) {
        free(image_buf);
        return -1;
    }

    // 设置faceId
    image_buf->extraInfo = calloc(strlen(faceId)+1, 1);
    strncpy(image_buf->extraInfo, faceId, strlen(faceId));

    image_buf->image.frameId = frame_index;
    image_buf->image.extData = image_buf;

    ret = ROCKIVA_PushFrame(iva_ctx->handle, &(image_buf->image), NULL);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_PushFrame fail ret=%d\n", ret);
        return -1;
    }

    return 0;
}

int main(int argc, char** argv)
{
    char* reg_image_dir = argv[1];

    printf("reg_image_dir: %s\n", reg_image_dir);

    RockIvaRetCode ret;

    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));

    open_db(FACE_DATABASE_PATH, &db);

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

    // 通用初始化
    ROCKIVA_Init(&iva_ctx.handle, ROCKIVA_MODE_PICTURE, commonParams, &iva_ctx);

    if (commonParams->license.memAddr != NULL) {
        free(commonParams->license.memAddr);
    }

    // 配置人脸回调
    RockIvaFaceCallback callback;
    callback.detCallback = NULL;
    callback.analyseCallback = rockiva_face_analyse_callback;

    // 配置为导库模式，人脸识别使能
    RockIvaFaceTaskParams faceParams;
    memset(&faceParams, 0, sizeof(RockIvaFaceTaskParams));
    faceParams.mode = ROCKIVA_FACE_MODE_IMPORT;
    faceParams.faceTaskType.faceRecognizeEnable = 1;

    // 人脸功能初始化
    ret = ROCKIVA_FACE_Init(iva_ctx.handle, &faceParams, callback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_FACE_Init error %d\n", ret);
        return -1;
    }

    // 配置帧释放回调
    ROCKIVA_SetFrameReleaseCallback(iva_ctx.handle, rockiva_image_release_callback);

    DIR* d = opendir(reg_image_dir);
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
        snprintf(image_path, PATH_MAX, "%s/%s", reg_image_dir, image_name);
        char* faceId = strtok(image_name, ".");
        printf("register %s image: %s\n", faceId, image_path);
        // 触发识别并注册（这里使用图像文件名作为注册人脸ID）
        triger_recog(&iva_ctx, image_path, faceId);
    }
    closedir(d);

    usleep(3 * 1000 * 1000);

    ROCKIVA_Release(iva_ctx.handle);

    close_db(db);

    printf("iva app exit\n");
    return 0;
}
