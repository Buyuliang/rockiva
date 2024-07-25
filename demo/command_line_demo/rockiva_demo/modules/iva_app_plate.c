#include "iva_app_plate.h"

#if IVA_APP_PLATE_EN

#include <limits.h>
#include <stdlib.h>
#include "rockiva_plate_api.h"
#include "utils/image_utils.h"

char carType[][10] = {"未知", "客车", "轿车", "面包车", "SUV", "皮卡车", "货车"};
char carColor[][10] = {"未知", "黑", "蓝", "棕", "灰", "黄", "绿", "紫", "红", "白"};
char carOrient[][10] = {"未知", "车头", "侧面", "车尾"};
char plateType[][10] = {"未知", "大型", "小型", "使馆", "领馆", "挂车", "教练车", "警车", "香港", "澳门", "武警", "军车", "新能源", "其他"};
char plateColor[][10] = {"未知", "蓝", "黄", "绿", "黑", "白"};

void PlateResultCallback(const RockIvaPlateResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    printf("PlateResultCallback frame %d status %d\n", result->frameId, status);

    RockIvaImage* frame = &(result->frame);

    if (status == ROCKIVA_SUCCESS) {
        char carplate_str[128] = {0};
        char carplate_score_str[128] = {0};
        printf("PlateResultCallback objNum %d\n", result->objNum);
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
            // vehicle
            const RockIvaPlateInfo* plateInfo = &(result->plateInfo[i]);
            int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, plateInfo->vehicleRect.topLeft.x);
            int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, plateInfo->vehicleRect.topLeft.y);
            int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, plateInfo->vehicleRect.bottomRight.x);
            int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, plateInfo->vehicleRect.bottomRight.y);

            printf("frameId=%d vehicle id=%d box=(%d %d %d %d) type=%d color=%d orient=%d\n", result->frameId, plateInfo->vehicleId,
                plateInfo->vehicleRect.topLeft.x, plateInfo->vehicleRect.topLeft.y, plateInfo->vehicleRect.bottomRight.x, plateInfo->vehicleRect.bottomRight.y,
                plateInfo->vehicleAttr.type, plateInfo->vehicleAttr.color, plateInfo->vehicleAttr.orient);
            // plate
            printf("plate type=%d\n", plateInfo->plateType);
            if (plateInfo->plateType != PLATE_TYPE_NONE) {
                int plate_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, plateInfo->plateRect.topLeft.x);
                int plate_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, plateInfo->plateRect.topLeft.y);
                int plate_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, plateInfo->plateRect.bottomRight.x);
                int plate_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, plateInfo->plateRect.bottomRight.y);

                StrIntArray(carplate_score_str, 128, plateInfo->plateScore, plateInfo->plateLen);
                memset(carplate_str, 0, 128);
                for (int n = 0; n < plateInfo->plateLen; n++) {
                    strncat(carplate_str, ROCKIVA_PLATE_CODE[plateInfo->plateCode[n]], 128);
                }
                printf(" plate number=%s scores=(%s) len=%d type=%d color=%d\n", carplate_str, carplate_score_str, plateInfo->plateLen,
                    plateInfo->plateType, plateInfo->plateColor);
                // save car image
                RockIvaImage carImage;

                RockIvaRetCode ret = ROCKIVA_IMAGE_Crop(frame, &(result->plateInfo->vehicleRect), 4, &carImage, NULL);
                if (ret == ROCKIVA_RET_SUCCESS) {
                    char car_save_path[PATH_MAX] = {0};
                    snprintf(car_save_path, PATH_MAX, "%s/Frame%d-Obj%d-%s_%s_%s.jpg", OUT_CAPTURE_PATH, result->frameId, plateInfo->vehicleId,
                        carType[plateInfo->vehicleAttr.type], carColor[plateInfo->vehicleAttr.color], carOrient[plateInfo->vehicleAttr.orient]);
                    printf("%s\n", car_save_path);
                    SaveImage(car_save_path, &carImage);
                    ROCKIVA_IMAGE_Release(&carImage);
                }
                // save plate image
                RockIvaImage plateImage;
                ret = ROCKIVA_IMAGE_Crop(frame, &(result->plateInfo->plateRect), 4, &plateImage, NULL);
                if (ret == ROCKIVA_RET_SUCCESS) {
                    char plate_save_path[PATH_MAX] = {0};
                    snprintf(plate_save_path, PATH_MAX, "%s/Frame%d-%s_%s_%s_%s.jpg", OUT_CAPTURE_PATH, result->frameId, carplate_str,
                        plateType[plateInfo->plateType], plateColor[plateInfo->plateColor], carplate_score_str);
                    printf("%s\n", plate_save_path);
                    SaveImage(plate_save_path, &plateImage);
                    ROCKIVA_IMAGE_Release(&plateImage);
                }
                // draw plate result
                DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height,
                    plate_rect_x1, plate_rect_y1, plate_rect_x2-plate_rect_x1+1, plate_rect_y2-plate_rect_y1+1, COLOR_BLUE, 2);
                char carplateText[32] = {0};
                snprintf(carplateText, 32, "Id%d-%s-Type%d-Color%d", plateInfo->vehicleId, carplate_str, plateInfo->plateType, plateInfo->plateColor);
                DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, carplateText, plate_rect_x1, plate_rect_y1-22, 10, COLOR_BLUE);
            }
            // draw vehicle result
            DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height,
                obj_rect_x1, obj_rect_y1, obj_rect_x2-obj_rect_x1+1, obj_rect_y2-obj_rect_y1+1, COLOR_BLUE, 2);
            char text[32] = {0};
            snprintf(text, 32, "Id%d-Type%d-Color%d-Orient%d", plateInfo->vehicleId, plateInfo->vehicleAttr.type, plateInfo->vehicleAttr.color, plateInfo->vehicleAttr.orient);
            DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1 + 2, obj_rect_y1 + 2, 10, COLOR_BLUE);
        }


        char save_path[PATH_MAX] = {0};
        snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
        SaveImage(save_path, &(drawImage));
        printf("save detect result to: %s\n", save_path);

        ROCKIVA_IMAGE_Release(&drawImage);
    }
}

int InitIvaPlate(IvaAppContext* ctx)
{
    RockIvaPlateTaskParam* plateParams = malloc(sizeof(RockIvaPlateTaskParam));
    memset(plateParams, 0, sizeof(RockIvaPlateTaskParam));

    // 机动车车身最小宽度（这里设置为1920下200像素宽度）
    plateParams->vehicleMinSize = ROCKIVA_PIXEL_RATION_CONVERT(1920, 200);
    // 车牌最小像素宽度（这里设置为1920下80像素宽度）
    plateParams->plateMinSize = ROCKIVA_PIXEL_RATION_CONVERT(1920, 70);
    // 车牌识别字符最小分数
    plateParams->plateMinScore = 95;
    // 设置识别模式
    plateParams->mode = 1;

    // plateParams->detectAreas.areaNum = 1;
    // plateParams->detectAreas.areas->pointNum = 4;
    // plateParams->detectAreas.areas->points[0].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 980);
    // plateParams->detectAreas.areas->points[0].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 330);
    // plateParams->detectAreas.areas->points[1].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1030);
    // plateParams->detectAreas.areas->points[1].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 780);
    // plateParams->detectAreas.areas->points[2].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1800);
    // plateParams->detectAreas.areas->points[2].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 780);
    // plateParams->detectAreas.areas->points[3].x = ROCKIVA_PIXEL_RATION_CONVERT(1920, 1450);
    // plateParams->detectAreas.areas->points[3].y = ROCKIVA_PIXEL_RATION_CONVERT(1080, 330);

    RockIvaRetCode ret = ROCKIVA_PLATE_Init(ctx->handle, plateParams, PlateResultCallback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_PLATE_Init error %d\n", ret);
        return -1;
    }
    ctx->plateData = plateParams;
    return 0;
}

int ReleaseIvaPlate(IvaAppContext* ctx)
{
    if (ctx->plateData != NULL) {
        free(ctx->plateData);
        ctx->plateData = NULL;
    }
    return 0;
}

#endif