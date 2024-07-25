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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/limits.h>
#include <sys/stat.h>

#include "rockiva_image.h"
#include "rockiva_one_api.h"

#include "iva_app_ctx.h"

#include "utils/common_utils.h"
#include "utils/face_db.h"
#include "utils/image_buffer.h"
#include "utils/image_utils.h"

#define MAX_IMAGE_BUFFER_SIZE 4

static void CreateDir(const char* path)
{
    if (access(path, R_OK) != 0) {
        if (mkdir(path, 0755) == -1) {
            printf("create dir %s fail", path);
        } else {
            printf("create dir %s\n", path);
        }
    }
}

static void CreateOutDir()
{
    if (access(OUT_DIR_PATH, R_OK) == 0) {
        rmdir(OUT_DIR_PATH);
    }
    CreateDir(OUT_DIR_PATH);
    CreateDir(OUT_FRAMES_PATH);
    CreateDir(OUT_CAPTURE_PATH);
}

void FrameReleaseCallback(const RockIvaReleaseFrames* releaseFrames, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    printf("FrameReleaseCallback count=%d\n", releaseFrames->count);
    for (int i = 0; i < releaseFrames->count; i++) {
        IvaImageBuf* img_buf = releaseFrames->frames[i].extData;
        printf("release img_buf=%p\n", img_buf);
        if (img_buf != NULL) {
            RetureImageBuffer(iva_ctx->imgBufManager, img_buf);
        } else {
            printf("FrameReleaseCallback: can not find image buffer for frameId=%d\n",
                   releaseFrames->frames[i].frameId);
        }
    }
}

void ProcessFaceDetResult(const char* result)
{
    RockIvaRetCode ret;
    RockIvaImage frame;
    ret = ROCKIVA_ONE_GetImage(result, &frame, "faceDetResult/frame");
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("get frame fail");
        return;
    }

    printf("frame: width=%d height=%d format=%d dataVirtual=%p\n", frame.info.width, frame.info.height,
           frame.info.format, frame.dataAddr);

    int width = frame.info.width;
    int height = frame.info.height;

    // 拷贝一份避免画框改写原图数据
    RockIvaImage drawImage;
    ROCKIVA_IMAGE_Clone(&frame, &drawImage);

    int faceNum = 0;
    ROCKIVA_ONE_GetArraySize(result, &faceNum, "faceDetResult/faceInfo");
    printf("faceNum=%d\n", faceNum);

    for (int i = 0; i < faceNum; i++) {
        RockIvaRectangle rect;
        int objId;
        int score, faceScore, clarity;
        int eyesScore, noseScore, mouthScore;
        int pitch, yaw, roll;
        ret = ROCKIVA_ONE_GetInt(result, &objId, 0, "faceDetResult/faceInfo/%d/objId", i);
        ret = ROCKIVA_ONE_GetInt(result, &score, 0, "faceDetResult/faceInfo/%d/faceQuality/score", i);
        ret = ROCKIVA_ONE_GetInt(result, &faceScore, 0, "faceDetResult/faceInfo/%d/faceQuality/faceScore", i);
        ret = ROCKIVA_ONE_GetInt(result, &clarity, 0, "faceDetResult/faceInfo/%d/faceQuality/clarity", i);
        ret = ROCKIVA_ONE_GetInt(result, &pitch, 0, "faceDetResult/faceInfo/%d/faceQuality/angle/pitch", i);
        ret = ROCKIVA_ONE_GetInt(result, &roll, 0, "faceDetResult/faceInfo/%d/faceQuality/angle/roll", i);
        ret = ROCKIVA_ONE_GetInt(result, &yaw, 0, "faceDetResult/faceInfo/%d/faceQuality/angle/yaw", i);
        ret = ROCKIVA_ONE_GetInt(result, &eyesScore, 0, "faceDetResult/faceInfo/%d/faceQuality/eyesScore", i);
        ret = ROCKIVA_ONE_GetInt(result, &noseScore, 0, "faceDetResult/faceInfo/%d/faceQuality/noseScore", i);
        ret = ROCKIVA_ONE_GetInt(result, &mouthScore, 0, "faceDetResult/faceInfo/%d/faceQuality/mouthScore", i);
        ret = ROCKIVA_ONE_GetRect(result, &rect, "faceDetResult/faceInfo/%d/faceRect", i);
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, width, height, obj_rect_x1, obj_rect_y1, obj_rect_x2 - obj_rect_x1 + 1,
                         obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);
        char text[256] = {0};
        snprintf(text, 32, "%d-%d_%d_%d_%d_%d_%d_%d_%d_%d", objId, score, faceScore, clarity, pitch, roll, yaw,
                 eyesScore, noseScore, mouthScore);
        DrawTextRGB(drawImage.dataAddr, width, height, text, obj_rect_x1, obj_rect_y1 - 22, 10, COLOR_BLUE);
    }
    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, frame.frameId);
    printf("%lu before write image\n", GetTimeStampMS());
    SaveImage(save_path, &(drawImage));
    printf("%lu after write image\n", GetTimeStampMS());
    printf("save detect result to: %s\n", save_path);
    ROCKIVA_IMAGE_Release(&drawImage);
}

void ProcessFaceCapResult(const char* result)
{
    RockIvaRetCode ret;
    int faceCapFrameType;

    if (faceCapFrameType != 6) {
        RockIvaRectangle faceRectOnCaptureImage;
        RockIvaImage captureImage;
        int objId;
        int score, faceScore, clarity;
        int eyesScore, noseScore, mouthScore;
        int pitch, yaw, roll;
        int mask;
        char save_path[PATH_MAX];
        memset(save_path, 0, PATH_MAX);

        ret = ROCKIVA_ONE_GetImage(result, &captureImage, "faceCapResult/captureImage");
        ret = ROCKIVA_ONE_GetInt(result, &objId, 0, "faceCapResult/faceInfo/objId");
        ret = ROCKIVA_ONE_GetInt(result, &score, 0, "faceCapResult/faceInfo/faceQuality/score");
        ret = ROCKIVA_ONE_GetInt(result, &faceScore, 0, "faceCapResult/faceInfo/faceQuality/faceScore");
        ret = ROCKIVA_ONE_GetInt(result, &clarity, 0, "faceCapResult/faceInfo/faceQuality/clarity");
        ret = ROCKIVA_ONE_GetInt(result, &pitch, 0, "faceCapResult/faceInfo/faceQuality/angle/pitch");
        ret = ROCKIVA_ONE_GetInt(result, &roll, 0, "faceCapResult/faceInfo/faceQuality/angle/roll");
        ret = ROCKIVA_ONE_GetInt(result, &yaw, 0, "faceCapResult/faceInfo/faceQuality/angle/yaw");
        ret = ROCKIVA_ONE_GetInt(result, &eyesScore, 0, "faceCapResult/faceInfo/faceQuality/eyesScore");
        ret = ROCKIVA_ONE_GetInt(result, &noseScore, 0, "faceCapResult/faceInfo/faceQuality/noseScore");
        ret = ROCKIVA_ONE_GetInt(result, &mouthScore, 0, "faceCapResult/faceInfo/faceQuality/mouthScore");
        ret = ROCKIVA_ONE_GetInt(result, &mask, 0, "faceCapResult/faceAnalyseInfo/faceAttr/mask");
        ret = ROCKIVA_ONE_GetRect(result, &faceRectOnCaptureImage, "faceCapResult/faceRectOnCaptureImage");

        snprintf(save_path, PATH_MAX, "%s/%d_%d_%d_%d_%d_%d_quality%d_%d_%d_%d_%d_%d_%d_%d_%d_type%d_mask%d_face.png", OUT_CAPTURE_PATH,
            objId, captureImage.frameId, 
            faceRectOnCaptureImage.topLeft.x, faceRectOnCaptureImage.topLeft.y, faceRectOnCaptureImage.bottomRight.x, faceRectOnCaptureImage.bottomRight.y, 
            score, faceScore, clarity, pitch, roll, yaw, eyesScore, noseScore, mouthScore,
            faceCapFrameType, mask);
        printf("save capture to %s\n", save_path);
        SaveImage(save_path, &captureImage);
        printf("capture image data=%p size=%d size=%dx%d fmt=%d\n", captureImage.dataAddr, captureImage.size,
            captureImage.info.width, captureImage.info.height, captureImage.info.format);
    }
}

void ProcessBaResult(const char* result)
{
    RockIvaRetCode ret;
    RockIvaImage frame;
    ret = ROCKIVA_ONE_GetImage(result, &frame, "baResult/frame");
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("get frame fail");
        return;
    }

    printf("frame: width=%d height=%d format=%d dataVirtual=%p\n", frame.info.width, frame.info.height,
           frame.info.format, frame.dataAddr);

    int width = frame.info.width;
    int height = frame.info.height;

    // 拷贝一份避免画框改写原图数据
    RockIvaImage drawImage;
    ROCKIVA_IMAGE_Clone(&frame, &drawImage);

    int triggerNum = 0;
    ROCKIVA_ONE_GetArraySize(result, &triggerNum, "baResult/triggerObjects");
    printf("triggerNum=%d\n", triggerNum);

    RockIvaRectangle rect;
    int objId, score, triggerRules, firstTriggerType;
    for (int i = 0; i < triggerNum; i++) {
        ret = ROCKIVA_ONE_GetInt(result, &objId, 0, "baResult/triggerObjects/%d/objInfo/objId", i);
        ret = ROCKIVA_ONE_GetInt(result, &score, 0, "baResult/triggerObjects/%d/objInfo/score", i);
        ret = ROCKIVA_ONE_GetInt(result, &triggerRules, 0, "baResult/triggerObjects/%d/triggerRules", i);
        ret = ROCKIVA_ONE_GetInt(result, &firstTriggerType, 0, "baResult/triggerObjects/%d/firstTrigger/triggerType", i);
        ret = ROCKIVA_ONE_GetRect(result, &rect, "baResult/triggerObjects/%d/objInfo/rect", i);
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.bottomRight.y);
        if (triggerRules > 0) {
            DrawRectangleRGB(drawImage.dataAddr, width, height, obj_rect_x1, obj_rect_y1, obj_rect_x2 - obj_rect_x1 + 1,
                         obj_rect_y2 - obj_rect_y1 + 1, COLOR_RED, 2);
        } else {
            DrawRectangleRGB(drawImage.dataAddr, width, height, obj_rect_x1, obj_rect_y1, obj_rect_x2 - obj_rect_x1 + 1,
                         obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);
        }
        char text[256] = {0};
        snprintf(text, 32, "Id%d-Score%d-TriggerRules%d", objId, score, triggerRules);
        DrawTextRGB(drawImage.dataAddr, width, height, text, obj_rect_x1, obj_rect_y1 - 22, 10, COLOR_BLUE);
    }

    // Draw Cross Line
    // DrawLineRGB(drawImage.dataAddr, width, height, 854, 592, 772, 1079, COLOR_BLUE, 2);

    // Draw BreakIn Area
    DrawLineRGB(drawImage.dataAddr, width, height, 616, 578, 1072, 612, COLOR_BLUE, 2);
    DrawLineRGB(drawImage.dataAddr, width, height, 616, 578, 32, 1079, COLOR_BLUE, 2);
    DrawLineRGB(drawImage.dataAddr, width, height, 1072, 612, 1358, 1079, COLOR_BLUE, 2);
    DrawLineRGB(drawImage.dataAddr, width, height, 32, 1079, 1358, 1079, COLOR_BLUE, 2);

    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, frame.frameId);
    printf("%lu before write image\n", GetTimeStampMS());
    SaveImage(save_path, &(drawImage));
    printf("%lu after write image\n", GetTimeStampMS());
    printf("save detect result to: %s\n", save_path);
    ROCKIVA_IMAGE_Release(&drawImage);
}

void ProcessPlateResult(const char* result)
{
    RockIvaRetCode ret;
    RockIvaImage frame;
    ret = ROCKIVA_ONE_GetImage(result, &frame, "plateResult/frame");
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("get frame fail");
        return;
    }

    printf("frame: width=%d height=%d format=%d dataVirtual=%p\n", frame.info.width, frame.info.height,
           frame.info.format, frame.dataAddr);

    int width = frame.info.width;
    int height = frame.info.height;

    // 拷贝一份避免画框改写原图数据
    RockIvaImage drawImage;
    ROCKIVA_IMAGE_Clone(&frame, &drawImage);

    int objNum = 0;
    ROCKIVA_ONE_GetInt(result, &objNum, 0, "plateResult/objNum");
    printf("objNum=%d\n", objNum);

    RockIvaRectangle rect;
    int objId, score, triggerRules, firstTriggerType;
    for (int i = 0; i < objNum; i++) {
        ret = ROCKIVA_ONE_GetRect(result, &rect, "plateResult/plateInfo/%d/vehicleRect", i);
        int vehicle_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.topLeft.x);
        int vehicle_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.topLeft.y);
        int vehicle_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.bottomRight.x);
        int vehicle_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, width, height, vehicle_rect_x1, vehicle_rect_y1, vehicle_rect_x2 - vehicle_rect_x1 + 1,
                        vehicle_rect_y2 - vehicle_rect_y1 + 1, COLOR_BLUE, 2);
        
        ret = ROCKIVA_ONE_GetRect(result, &rect, "plateResult/plateInfo/%d/plateRect", i);
        int plate_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.topLeft.x);
        int plate_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.topLeft.y);
        int plate_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(width, rect.bottomRight.x);
        int plate_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(height, rect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, width, height, plate_rect_x1, plate_rect_y1, plate_rect_x2 - plate_rect_x1 + 1,
                        plate_rect_y2 - plate_rect_y1 + 1, COLOR_BLUE, 2);

        char text[256] = {0};
        ROCKIVA_ONE_GetStr(result, text, 256, "plateResult/plateInfo/%d/plateStr", i);

        DrawTextRGB(drawImage.dataAddr, width, height, text, plate_rect_x1, plate_rect_y1 - 22, 10, COLOR_BLUE);
    }

    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, frame.frameId);
    printf("%lu before write image\n", GetTimeStampMS());
    SaveImage(save_path, &(drawImage));
    printf("%lu after write image\n", GetTimeStampMS());
    printf("save detect result to: %s\n", save_path);
    ROCKIVA_IMAGE_Release(&drawImage);
}


void ResultCallback(const char* result, const RockIvaExecuteStatus status, void* userdata)
{
    printf("result: %s\n", result);

    if (ROCKIVA_ONE_CheckExist(result, "faceDetResult") == ROCKIVA_RET_SUCCESS) {
        ProcessFaceDetResult(result);
    } else if (ROCKIVA_ONE_CheckExist(result, "faceCapResult") == ROCKIVA_RET_SUCCESS) {
        ProcessFaceCapResult(result);
    } else if (ROCKIVA_ONE_CheckExist(result, "baResult") == ROCKIVA_RET_SUCCESS) {
        ProcessBaResult(result);
    } else if (ROCKIVA_ONE_CheckExist(result, "plateResult") == ROCKIVA_RET_SUCCESS) {
        ProcessPlateResult(result);
    }
}

int main(int argc, char** argv)
{
    char* json_path = argv[1];
    char* image_dir = argv[2];
    int image_num = atoi(argv[3]);
    printf("json path: %s\n", json_path);
    printf("image dir: %s image_num: %d\n", image_dir, image_num);

    CreateOutDir();

    int ret = 0;

    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));

    char* json_config = NULL;
    ReadDataFile(json_path, &json_config);
    printf("json config:\n%s\n", json_config);

    RockIvaOneCallback callback;
    callback.resultCallback = ResultCallback;
    callback.releaseCallback = FrameReleaseCallback;
    RockIvaRetCode retcode = ROCKIVA_ONE_Init(&iva_ctx.handle, json_config, callback, &iva_ctx);
    free(json_config);
    if (retcode != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_Init error %d\n", retcode);
        return -1;
    }

    char(*image_path_list)[IMAGE_PATH_MAX] = malloc(image_num * IMAGE_PATH_MAX);
    memset(image_path_list, 0, image_num * IMAGE_PATH_MAX);

    int real_num = GetImagePathList(image_dir, image_path_list, image_num);

    if (real_num <= 0) {
        printf("can't get image from dir %d\n", image_dir);
        return -1;
    }

    int w = 0;
    int h = 0;
    ret = ReadImageInfo(image_path_list[0], &w, &h);
    if (ret != 0 || w <= 0 || h <= 0) {
        printf("get image info fail %d : %s\n", ret, image_path_list[0]);
        return -1;
    }

    iva_ctx.imgBufManager =
        CreateImageBufferManagerPreAlloc(5, w, h, ROCKIVA_IMAGE_FORMAT_RGB888, ROCKIVA_MEM_TYPE_DMA);

    for (int i = 0; i < real_num; i++) {
        printf("%d process image %s\n", i, image_path_list[i]);

        IvaImageBuf* img_buf = AcquireImageBuffer(iva_ctx.imgBufManager, 1000);

        ret = ReadImage(image_path_list[i], &img_buf->image);
        if (ret != 0) {
            printf("error get image fail!\n");
            goto next_frame;
        }

        img_buf->image.frameId = i;
        img_buf->image.extData = img_buf;
        printf("%lu PushFrame %d\n", GetTimeStampMS(), i);
        ret = ROCKIVA_PushFrame(iva_ctx.handle, &img_buf->image, NULL);

    next_frame:
        // about 10 fps
        // usleep(100 * 1000);
        continue;
    }
    free(image_path_list);

    ROCKIVA_ONE_Release(iva_ctx.handle);

    DestroyImageBufferManager(iva_ctx.imgBufManager);
    return 0;
}
