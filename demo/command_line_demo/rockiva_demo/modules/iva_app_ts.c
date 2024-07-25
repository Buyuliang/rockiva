#include "iva_app_ts.h"

#if IVA_APP_TS_EN

#include "rockiva_ts_api.h"
#include "utils/image_utils.h"

#include <limits.h>
#include <stdlib.h>

void TsResultCallback(const RockIvaTsResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    unsigned long cur_timestamp_ms = GetTimeStampMS();
    printf("%lu TsResultCallback frameId %d status=%d\n", cur_timestamp_ms, result->frameId, status);

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
        snprintf(text, 32, "Id%d-Type%d-Score%d", obj->objId, obj->type, obj->score);
        DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1 + 2, obj_rect_y1 + 2,
                    10, COLOR_BLUE);
    }

    RockIvaTsTaskParams* tsParams = (RockIvaTsTaskParams*)iva_ctx->tsData;
    for (int i = 0; i < ROCKIVA_TS_MAX_RULE_NUM; i++) {
        if (tsParams->lines[i].ruleEnable == 1) {
            RockIvaPoint start = tsParams->lines[i].line.head;
            RockIvaPoint end = tsParams->lines[i].line.tail;
            DrawLine(&drawImage, start, end);
            char text[32] = {0};
            snprintf(text, 32, "count:%d", result->traffic.lines[i].count);
            int start_x = ROCKIVA_RATIO_PIXEL_CONVERT(drawImage.info.width, start.x);
            int start_y = ROCKIVA_RATIO_PIXEL_CONVERT(drawImage.info.height, start.y);
            DrawTextRGB(drawImage.dataAddr, drawImage.info.width, drawImage.info.height, text, start_x, start_y,
                    10, COLOR_BLUE);
        }
        if (tsParams->areas[i].ruleEnable == 1) {
            DrawArea(&drawImage, &(tsParams->areas[i].area));
            char text[32] = {0};
            snprintf(text, 32, "count:%d in:%d out:%d", result->traffic.areas[i].count, result->traffic.areas[i].in, result->traffic.areas[i].out);
            int start_x = ROCKIVA_RATIO_PIXEL_CONVERT(drawImage.info.width, tsParams->areas[i].area.points[0].x);
            int start_y = ROCKIVA_RATIO_PIXEL_CONVERT(drawImage.info.height, tsParams->areas[i].area.points[0].y);
            DrawTextRGB(drawImage.dataAddr, drawImage.info.width, drawImage.info.height, text, start_x, start_y,
                    10, COLOR_BLUE);
        }
    }

    char save_path[PATH_MAX] = {0};
    snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
    SaveImage(save_path, &(drawImage));
    printf("save detect result to: %s\n", save_path);

    ROCKIVA_IMAGE_Release(&drawImage);
}

/* 初始化人流统计 */
int InitIvaTS(IvaAppContext* ctx)
{
    RockIvaTsTaskParams* tsParams = malloc(sizeof(RockIvaTsTaskParams));
    memset(tsParams, 0, sizeof(RockIvaTsTaskParams));
    tsParams->lines[0].ruleEnable = 1;
    tsParams->lines[0].ruleID = 1;
    tsParams->lines[0].objType = ROCKIVA_OBJECT_TYPE_HEAD;
    tsParams->lines[0].line.head.x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 100);
    tsParams->lines[0].line.head.y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 500);
    tsParams->lines[0].line.tail.x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1800);
    tsParams->lines[0].line.tail.y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 500);
    tsParams->lines[0].direct = ROCKIVA_LINE_DIRECT_CW;

    tsParams->areas[0].ruleEnable = 1;
    tsParams->areas[0].ruleID = 2;
    tsParams->areas[0].area.pointNum = 4;
    tsParams->areas[0].objType = ROCKIVA_OBJECT_TYPE_HEAD;
    tsParams->areas[0].area.points[0].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 440);
    tsParams->areas[0].area.points[0].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 170);
    tsParams->areas[0].area.points[1].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 100);
    tsParams->areas[0].area.points[1].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 340);
    tsParams->areas[0].area.points[2].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1200);
    tsParams->areas[0].area.points[2].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 340);
    tsParams->areas[0].area.points[3].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1200);
    tsParams->areas[0].area.points[3].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 170);

    RockIvaRetCode ret = ROCKIVA_TS_Init(ctx->handle, tsParams, TsResultCallback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_TS_Init error %d\n", ret);
        return -1;
    }
    ctx->tsData = tsParams;
    return 0;
}

int ReleaseIvaTs(IvaAppContext* ctx)
{
    if (ctx->tsData != NULL) {
        free(ctx->tsData);
        ctx->tsData = NULL;
    }
    return 0;
}

#endif