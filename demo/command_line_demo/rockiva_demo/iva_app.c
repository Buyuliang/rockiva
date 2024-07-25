#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/limits.h>
#include <sys/stat.h>

#include "iva_app.h"

#if IVA_APP_DET_EN
#include "modules/iva_app_det.h"
#endif

#if IVA_APP_BA_EN
#include "modules/iva_app_ba.h"
#endif

#if IVA_APP_FACE_EN
#include "modules/iva_app_face.h"
#endif

#if IVA_APP_PLATE_EN
#include "modules/iva_app_plate.h"
#endif

#if IVA_APP_POSE_EN
#include "modules/iva_app_pose.h"
#endif

#if IVA_APP_TS_EN
#include "modules/iva_app_ts.h"
#endif

#if IVA_APP_OBJECT_EN
#include "modules/iva_app_object.h"
#endif

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
    // printf("FrameReleaseCallback count=%d\n", releaseFrames->count);
    for (int i = 0; i < releaseFrames->count; i++) {
        RockIvaImage* frame = &releaseFrames->frames[i];
        if (frame->info.format == ROCKIVA_IMAGE_FORMAT_JPEG) {
            // 编码JPEG图像直接释放
            ROCKIVA_IMAGE_Release(frame);
        } else {
            IvaImageBuf* img_buf = frame->extData;
            // printf("release img_buf=%p\n", img_buf);
            if (img_buf != NULL) {
                RetureImageBuffer(iva_ctx->imgBufManager, img_buf);
            } else {
                printf("FrameReleaseCallback: can not find image buffer for frameId=%d\n", frame->frameId);
            }
        }
    }
}

int Yuv2Jpeg(RockIvaImage *inputImg, RockIvaImage *outputImg, void *userdata)
{
    return EncodeImageJpeg(inputImg, outputImg, 95);
}

static int InitIvaCommon(IvaAppContext* ctx)
{
    RockIvaInitParam* commonParams = &(ctx->commonParams);

    // 配置模型路径
    snprintf(commonParams->modelPath, ROCKIVA_PATH_LENGTH, MODEL_DATA_PATH);

    // 配置授权信息
    if (access(LICENSE_PATH, R_OK) == 0) {
        char* license_key;
        int license_size;
        license_size = ReadDataFile(LICENSE_PATH, &license_key);
        if (license_key != NULL && license_size > 0) {
            commonParams->license.memAddr = license_key;
            commonParams->license.memSize = license_size;
        }
    }

    // TODO: 设置检测模型
    commonParams->detModel = ROCKIVA_DET_MODEL_PFP;  // 人脸抓拍
    // commonParams->detModel = ROCKIVA_DET_MODEL_CLS7; // 车牌识别
    // commonParams->detModel = ROCKIVA_DET_MODEL_PHS;     // 客流统计

    // 婴幼儿检测
    // commonParams->detClasses |= ROCKIVA_OBJECT_TYPE_BITMASK(ROCKIVA_OBJECT_TYPE_BABY);

    commonParams->mediaOps.ROCKIVA_Yuv2Jpeg = Yuv2Jpeg;

    RockIvaRetCode ret = ROCKIVA_Init(&ctx->handle, ROCKIVA_MODE_VIDEO, commonParams, ctx);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_Init error %d\n", ret);
        return -1;
    }

    ROCKIVA_SetFrameReleaseCallback(ctx->handle, FrameReleaseCallback);

    if (commonParams->license.memAddr != NULL && commonParams->license.memSize > 0) {
        free(commonParams->license.memAddr);
    }

    return 0;
}

int InitIvaApp(IvaAppContext* iva_ctx)
{
    int ret;

    ret = InitIvaCommon(iva_ctx);
    if (ret != 0) {
        return -1;
    }

#if IVA_APP_DET_EN
    ret = InitIvaDet(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_BA_EN
    ret = InitIvaBa(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_FACE_EN
    ret = InitIvaFace(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_PLATE_EN
    ret = InitIvaPlate(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_POSE_EN
    ret = InitIvaPose(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_TS_EN
    ret = InitIvaTS(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_OBJECT_EN
    ret = InitIvaObject(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

    CreateOutDir();

    return 0;
}

int ReleaseIvaApp(IvaAppContext* iva_ctx)
{
    int ret = 0;

    RockIvaRetCode retcode = ROCKIVA_Release(iva_ctx->handle);
    if (retcode != ROCKIVA_RET_SUCCESS) {
        return -1;
    }

#if IVA_APP_DET_EN
    ret = ReleaseIvaDet(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_BA_EN
    ret = ReleaseIvaBa(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_FACE_EN
    ret = ReleaseIvaFace(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_PLATE_EN
    ret = ReleaseIvaPlate(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_POSE_EN
    ret = ReleaseIvaPose(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_TS_EN
    ret = ReleaseIvaTs(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

#if IVA_APP_OBJECT_EN
    ret = ReleaseIvaObject(iva_ctx);
    if (ret != 0) {
        return -1;
    }
#endif

    return 0;
}