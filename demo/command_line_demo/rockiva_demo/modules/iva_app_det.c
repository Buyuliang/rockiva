#include "iva_app_det.h"

#if IVA_APP_DET_EN

#include "rockiva_det_api.h"
#include "utils/image_utils.h"
#include <limits.h>
#include <stdlib.h>

void DetResultCallback(const RockIvaDetectResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    unsigned long cur_timestamp_ms = GetTimeStampMS();
    printf("%lu DetResultCallback frameId %d status=%d objNum=%d\n", cur_timestamp_ms, result->frameId, status,
           result->objNum);

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
        RockIvaObjectInfo* obj = &result->objInfo[i];
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                         obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);

        char text[32] = {0};
        snprintf(text, 32, "Frame%d-Id%d-Type%d-Score%d", obj->frameId, obj->objId, obj->type, obj->score);
        DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1 + 2, obj_rect_y1 + 2,
                    10, COLOR_BLUE);
    }

    RockIvaDetTaskParams* detParams = (RockIvaDetTaskParams*)iva_ctx->detData;
    for (int i = 0; i < detParams->roiAreas.areaNum; i++) {
        DrawArea(&drawImage, &detParams->roiAreas.areas[i]);
    }

    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
    SaveImage(save_path, &(drawImage));
    printf("save detect result to: %s\n", save_path);

    ROCKIVA_IMAGE_Release(&drawImage);
}

int InitIvaDet(IvaAppContext* ctx)
{
    RockIvaDetTaskParams* detParams = malloc(sizeof(RockIvaDetTaskParams));
    memset(detParams, 0, sizeof(RockIvaDetTaskParams));

    // TODO: 设置需要的检测类别
    // detParams->detObjectType |= ROCKIVA_OBJECT_TYPE_BITMASK(ROCKIVA_OBJECT_TYPE_HEAD);
    // detParams->detObjectType |= ROCKIVA_OBJECT_TYPE_BITMASK(ROCKIVA_OBJECT_TYPE_PERSON);

    // detParams->scores[ROCKIVA_OBJECT_TYPE_PERSON] = 30;

    RockIvaRetCode ret = ROCKIVA_DETECT_Init(ctx->handle, detParams, DetResultCallback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_DETECT_Init error %d\n", ret);
        free(detParams);
        return -1;
    }
    ctx->detData = detParams;
    return 0;
}

int ReleaseIvaDet(IvaAppContext* ctx)
{
    if (ctx->detData != NULL) {
        free(ctx->detData);
        ctx->detData = NULL;
    }
    return 0;
}

#endif