#include "iva_app_ba.h"

#if IVA_APP_BA_EN

#include "rockiva_ba_api.h"
#include <limits.h>
#include <stdlib.h>

static void GetObjectColor(const RockIvaObjectInfo* obj_info, uint8_t color[3])
{
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
    if (obj_info->type == ROCKIVA_OBJECT_TYPE_PERSON) {
        color[0] = 255;
        color[1] = 255;
    } else if (obj_info->type == ROCKIVA_OBJECT_TYPE_VEHICLE) {
        color[1] = 255;
        color[2] = 255;
    } else {
        color[0] = 255;
        color[2] = 255;
    }
}

static void GetObjectTrigerColor(const RockIvaBaObjectInfo* ba_obj_info, uint8_t color[3])
{
    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
    if (ba_obj_info->triggerRules != 0) {
        printf("object %d triger rule %d\n", ba_obj_info->objInfo.objId, ba_obj_info->triggerRules);
        color[0] = 255;
    } else {
        GetObjectColor(&ba_obj_info->objInfo, color);
    }
}

static int DrawBaRules(RockIvaImage* image, RockIvaBaTaskParams* params)
{
    for (int i = 0; i < ROCKIVA_BA_MAX_RULE_NUM; i++) {
        RockIvaBaWireRule* rule = &(params->baRules.tripWireRule[i]);
        if (rule->ruleEnable == 1) {
            DrawLine(image, rule->line.head, rule->line.tail);
        }
    }
    for (int i = 0; i < ROCKIVA_BA_MAX_RULE_NUM; i++) {
        RockIvaBaAreaRule* rule = &(params->baRules.areaInBreakRule[i]);
        if (rule->ruleEnable == 1) {
            DrawArea(image, &(rule->area));
        }
    }
    for (int i = 0; i < ROCKIVA_BA_MAX_RULE_NUM; i++) {
        RockIvaBaAreaRule* rule = &(params->baRules.areaInRule[i]);
        if (rule->ruleEnable == 1) {
            DrawArea(image, &(rule->area));
        }
    }
    for (int i = 0; i < ROCKIVA_BA_MAX_RULE_NUM; i++) {
        RockIvaBaAreaRule* rule = &(params->baRules.areaOutRule[i]);
        if (rule->ruleEnable == 1) {
            DrawArea(image, &(rule->area));
        }
    }
    return 0;
}

void BaResultCallback(const RockIvaBaResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    printf("BaResultCallback frame %d\n", result->frameId);
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;
    RockIvaBaTaskParams* baParams = (RockIvaBaTaskParams*)iva_ctx->baData;

    if (status == ROCKIVA_SUCCESS) {
        const RockIvaImage* frame = &(result->frame);

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

        int has_triger = 0;

        for (int i = 0; i < result->objNum; i++) {
            if (result->triggerObjects[i].firstTrigger.ruleID >= 0) {
                RockIvaImage roiImage;
                ROCKIVA_IMAGE_Crop(&drawImage, &result->triggerObjects[i].objInfo.rect, 4, &roiImage, NULL);
                char out_img_path[PATH_MAX] = {0};
                snprintf(out_img_path, PATH_MAX, "%s/%d-%d.jpg", OUT_CAPTURE_PATH, result->frameId, result->triggerObjects[i].objInfo.objId);
                SaveImage(out_img_path, &roiImage);
                ROCKIVA_IMAGE_Release(&roiImage);
                has_triger = 1;
            }
        }

        if (result->objNum > 0) {

        for (int i = 0; i < result->objNum; i++) {
            uint8_t color[3];
            uint8_t text_color[3];
            char text[32];
            snprintf(text, 32, "%d - %d", result->triggerObjects[i].objInfo.objId, result->triggerObjects[i].objInfo.score);
            GetObjectColor(&(result->triggerObjects[i].objInfo), color);
            GetObjectTrigerColor(&(result->triggerObjects[i]), text_color);
            const RockIvaObjectInfo* obj = &(result->triggerObjects[i].objInfo);
            int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.topLeft.x);
            int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.topLeft.y);
            int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, obj->rect.bottomRight.x);
            int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, obj->rect.bottomRight.y);
            DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height,
                obj_rect_x1, obj_rect_y1, obj_rect_x2-obj_rect_x1+1, obj_rect_y2-obj_rect_y1+1, color, 2);
            DrawTextRGB(drawImage.dataAddr, drawImage.info.width, drawImage.info.height, text, obj_rect_x1, obj_rect_y1-22, 10, text_color);
        }

        DrawBaRules(&drawImage, baParams);

        char out_img_path[PATH_MAX] = {0};
        snprintf(out_img_path, PATH_MAX, "%s/%d.jpg", OUT_FRAMES_PATH, result->frameId);
        printf("write img to %s\n", out_img_path);
        SaveImage(out_img_path, &drawImage);
        }

        ROCKIVA_IMAGE_Release(&drawImage);
    }
}

int InitIvaBa(IvaAppContext* ctx)
{
    RockIvaBaTaskParams* baParams = malloc(sizeof(RockIvaBaTaskParams));
    memset(baParams, 0, sizeof(RockIvaBaTaskParams));

    baParams->aiConfig.detectResultMode = 1;
    baParams->aiConfig.filterPersonMode = 1;

    // 构建一个区域入侵规则
    baParams->baRules.areaInBreakRule[0].ruleEnable = 1;
    baParams->baRules.areaInBreakRule[0].alertTime = 1000;  // 1000ms
    baParams->baRules.areaInBreakRule[0].event = ROCKIVA_BA_TRIP_EVENT_STAY;
    baParams->baRules.areaInBreakRule[0].ruleID = 1;
    baParams->baRules.areaInBreakRule[0].objType |= ROCKIVA_OBJECT_TYPE_BITMASK(ROCKIVA_OBJECT_TYPE_PERSON);
    baParams->baRules.areaInBreakRule[0].area.pointNum = 4;
    baParams->baRules.areaInBreakRule[0].area.points[0].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 900);
    baParams->baRules.areaInBreakRule[0].area.points[0].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 400);
    baParams->baRules.areaInBreakRule[0].area.points[1].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 600);
    baParams->baRules.areaInBreakRule[0].area.points[1].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 600);
    baParams->baRules.areaInBreakRule[0].area.points[2].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1800);
    baParams->baRules.areaInBreakRule[0].area.points[2].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 600);
    baParams->baRules.areaInBreakRule[0].area.points[3].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1500);
    baParams->baRules.areaInBreakRule[0].area.points[3].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 400);

    RockIvaRetCode ret = ROCKIVA_BA_Init(ctx->handle, baParams, BaResultCallback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_BA_Init error %d\n", ret);
        return -1;
    }
    ctx->baData = baParams;
    return 0;
}

int ReleaseIvaBa(IvaAppContext* ctx)
{
    if (ctx->baData != NULL) {
        free(ctx->baData);
        ctx->baData = NULL;
    }
    return 0;
}

#endif