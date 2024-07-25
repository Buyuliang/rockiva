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

#include <sys/stat.h>

#include "iva_app.h"

#include "utils/common_utils.h"
#include "utils/image_buffer.h"
#include "utils/image_utils.h"

#define MAX_IMAGE_BUFFER_SIZE 4

int main(int argc, char** argv)
{
    char* image_dir = argv[1];
    int image_num = atoi(argv[2]);
    printf("image dir: %s image_num: %d\n", image_dir, image_num);

    int ret = 0;

    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));

    ret = InitIvaApp(&iva_ctx);
    if (ret != 0) {
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
        CreateImageBufferManagerPreAlloc(ALLOC_IMAGE_BUFFER_NUM, w, h, ROCKIVA_IMAGE_FORMAT_RGB888, ALLOC_IMAGE_BUFFER_TYPE);

    for (int i = 0; i < real_num; i++) {
        printf("%d process image %s\n", i, image_path_list[i]);

        IvaImageBuf* img_buf = AcquireImageBuffer(iva_ctx.imgBufManager, 5000);

        printf("%lu before read image\n", GetTimeStampMS());
        ret = ReadImage(image_path_list[i], &img_buf->image);
        printf("%lu after read image\n", GetTimeStampMS());
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
        usleep(100 * 1000);
        continue;
    }
    free(image_path_list);

    ReleaseIvaApp(&iva_ctx);

    DestroyImageBufferManager(iva_ctx.imgBufManager);

    return 0;
}
