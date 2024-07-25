#include "iva_app_object.h"

#if IVA_APP_OBJECT_EN

#include "rockiva_object_api.h"
#include "utils/image_utils.h"
#include <limits.h>
#include <stdlib.h>

void NonvehicleResultCallback(const RockIvaObjectNonvehicleAttrResult* result, const RockIvaExecuteStatus status,
                              void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    unsigned long cur_timestamp_ms = GetTimeStampMS();
    printf("%lu NonvehicleResultCallback frameId %d status=%d\n", cur_timestamp_ms, result->frameId, status);

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
        RockIvaObjectNonvehicleInfo* obj = &result->objectInfo[i];
        unsigned char* color = COLOR_BLUE;
        if (obj->alert == 1) {
            color = COLOR_RED;
        }
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->objectRect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->objectRect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->objectRect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->objectRect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                         obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, color, 2);
        char text[32] = {0};
        snprintf(text, 32, "frame%d-ID%d-Alert%d-Type%d", frame->frameId, obj->objId, obj->alert, obj->attr.type);
        DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1, obj_rect_y1 + 2, 10,
                    color);
    }
    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
    SaveImage(save_path, &(drawImage));
    printf("save nonvehicle result to: %s\n", save_path);

    ROCKIVA_IMAGE_Release(&drawImage);
    return;
}

void FireResultCallback(const RockIvaObjectFireResult* result, const RockIvaExecuteStatus status,
                              void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    unsigned long cur_timestamp_ms = GetTimeStampMS();
    printf("%lu FireResultCallback frameId %d status=%d\n", cur_timestamp_ms, result->frameId, status);

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
        RockIvaObjectInfo* obj = &result->objectInfo[i];
        int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.topLeft.x);
        int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.topLeft.y);
        int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.bottomRight.x);
        int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.bottomRight.y);
        DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                         obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, COLOR_BLUE, 2);
        char text[32] = {0};
        snprintf(text, 32, "frame%d-score%d", frame->frameId, obj->score);
        DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1, obj_rect_y1 + 2, 10, COLOR_BLUE);
    }
    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
    SaveImage(save_path, &(drawImage));
    printf("save fire result to: %s\n", save_path);

    ROCKIVA_IMAGE_Release(&drawImage);
    return;
}

int InitIvaObject(IvaAppContext* ctx)
{
    RockIvaObjectTaskParams* objectParams = malloc(sizeof(RockIvaObjectTaskParams));
    memset(objectParams, 0, sizeof(RockIvaObjectTaskParams));

    objectParams->nonvehicleRule.enable = 1;
    objectParams->nonvehicleRule.detScene = 0;  // 0：平视角，1：俯视角

    objectParams->fireRule.enable = 0;

    RockIvaObjectResultCallback callback;
    callback.nonvehicleCallback = NonvehicleResultCallback;
    callback.fireCallback = FireResultCallback;
    RockIvaRetCode ret = ROCKIVA_OBJECT_Init(ctx->handle, objectParams, callback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_OBJECT_Init error %d\n", ret);
        return -1;
    }
    ctx->objectData = objectParams;
    return 0;
}

int ReleaseIvaObject(IvaAppContext* ctx)
{
    if (ctx->objectData != NULL) {
        free(ctx->objectData);
        ctx->objectData = NULL;
    }
    return 0;
}

#endif