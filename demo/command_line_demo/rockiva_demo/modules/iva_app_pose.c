#include "iva_app_pose.h"

#if IVA_APP_POSE_EN

#include "rockiva_pose_api.h"
#include "utils/image_utils.h"
#include <limits.h>
#include <stdlib.h>

void PoseResultCallback(const RockIvaPoseResult* result, const RockIvaExecuteStatus status, void* userdata)
{
    printf("PoseResultCallback frame %d status %d\n", result->frameId, status);

    RockIvaImage* frame = &(result->frame);

    if (status == ROCKIVA_SUCCESS) {
        printf("PoseResultCallback objNum %d\n", result->objNum);
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

        unsigned char* color;
        for (int i = 0; i < result->objNum; i++) {
            // vehicle
            const RockIvaPoseInfo* poseInfo = &(result->poseInfo[i]);
            int obj_rect_x1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, poseInfo->poseRect.topLeft.x);
            int obj_rect_y1 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, poseInfo->poseRect.topLeft.y);
            int obj_rect_x2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.width, poseInfo->poseRect.bottomRight.x);
            int obj_rect_y2 = ROCKIVA_RATIO_PIXEL_CONVERT(frame->info.height, poseInfo->poseRect.bottomRight.y);
            if (poseInfo->poseType == POSE_TYPE_FALLDOWN) {
                color = COLOR_RED;
            } else {
                color = COLOR_GREEN;
            }

            DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                             obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, color, 2);
            printf("frameId=%d pose id=%d box=(%d %d %d %d) posetype=%d trigger=%d\n", result->frameId, poseInfo->poseId,
                   obj_rect_x1, obj_rect_y1, obj_rect_x2, obj_rect_y2, poseInfo->poseType, poseInfo->isFalldown);
            char text[32] = {0};
            snprintf(text, 32, "Id%d-PoseType%d-Trigger%d", poseInfo->poseId, poseInfo->poseType, poseInfo->isFalldown);
            DrawTextRGB(drawImage.dataAddr, frame->info.width, frame->info.height, text, obj_rect_x1 + 2,
                        obj_rect_y1 + 2, 10, color);
            DrawRectangleRGB(drawImage.dataAddr, frame->info.width, frame->info.height, obj_rect_x1, obj_rect_y1,
                             obj_rect_x2 - obj_rect_x1 + 1, obj_rect_y2 - obj_rect_y1 + 1, color, 2);
        }
        char save_path[PATH_MAX] = {0};
        snprintf(save_path, PATH_MAX, "%s/%04d.jpg", OUT_FRAMES_PATH, result->frameId);
        SaveImage(save_path, &(drawImage));
        printf("save detect result to: %s\n", save_path);
        ROCKIVA_IMAGE_Release(&drawImage);
    }
}

int InitIvaPose(IvaAppContext* ctx)
{
    RockIvaPoseTaskParam* poseParams = malloc(sizeof(RockIvaPoseTaskParam));
    memset(poseParams, 0, sizeof(RockIvaPoseTaskParam));
    poseParams->mode = 1;
    RockIvaRetCode ret = ROCKIVA_POSE_Init(ctx->handle, poseParams, PoseResultCallback);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("ROCKIVA_POSE_Init error %d\n", ret);
        return -1;
    }
    ctx->poseData = poseParams;
    return 0;
}

int ReleaseIvaPose(IvaAppContext* ctx)
{
    if (ctx->poseData != NULL) {
        free(ctx->poseData);
        ctx->poseData = NULL;
    }
    return 0;
}

#endif