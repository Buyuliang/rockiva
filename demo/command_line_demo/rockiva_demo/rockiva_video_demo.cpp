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
#include <unistd.h>

#include <linux/limits.h>
#include <sys/stat.h>

#include "iva_app.h"

#include "utils/common_utils.h"
#include "utils/image_buffer.h"
#include "utils/image_utils.h"
#include "utils/mpp_decoder.h"

#if defined(BUILD_VIDEO_RTSP)
#include "mk_mediakit.h"
#endif

#define VIDEO_FRAME_SKIP_NUM 2  // 表示对视频帧多少帧取一帧做推理

typedef struct
{
    MppDecoder* decoder;
    char* video_url;
    int video_type;
} VideoAppContext;

void VideoDecoderFrameCallback(void* userdata, int width_stride, int height_stride, int width, int height, int format,
                                int fd, void* data)
{
    IvaAppContext* iva_ctx = (IvaAppContext*)userdata;

    printf("VideoDecoderFrameCallback frame width=%d height=%d format=%d fd=%d data=%p\n", width_stride, height_stride,
           width, height, format, fd, data);

    static int video_frame_index = 0;
    video_frame_index++;
    if (video_frame_index % VIDEO_FRAME_SKIP_NUM != 0) {
        return;
    }

    static int frame_index = 0;
    frame_index++;

    if (iva_ctx->imgBufManager == NULL) {
        iva_ctx->imgBufManager = CreateImageBufferManagerPreAlloc(3, width, height, ROCKIVA_IMAGE_FORMAT_YUV420SP_NV12,
                                                                  ROCKIVA_MEM_TYPE_CPU);
    }

    IvaImageBuf* img_buf = AcquireImageBuffer(iva_ctx->imgBufManager, 5000);

    // TODO: avoid memcpy
    if (img_buf != NULL) {
        uint8_t* img_data = img_buf->image.dataAddr;
        for (int i = 0; i < height; i++) {
            memcpy(img_data + i * width, (char*)data + width_stride * i, width);
        }
        for (int i = 0; i < height / 2; i++) {
            memcpy(img_data + width * height + i * width, (char*)data + width_stride * height_stride + i * width_stride,
                   width);
        }
    }

    img_buf->image.frameId = frame_index;
    img_buf->image.extData = img_buf;
    printf("%lu PushFrame %d\n", GetTimeStampMS(), frame_index);
    RockIvaRetCode ret = ROCKIVA_PushFrame(iva_ctx->handle, &img_buf->image, NULL);
    if (ret != ROCKIVA_RET_SUCCESS) {
        printf("PushFrame %d fail\n", frame_index);
    }
}

int ProcessVideoFile(IvaAppContext* ctx, const char* path)
{
    VideoAppContext* video_ctx = (VideoAppContext*)ctx->extData;

    char* video_data;
    int video_size = ReadDataFile(path, &video_data);
    char* video_data_end = video_data + video_size;
    printf("read video size=%d\n", video_size);

    const int SIZE = 8192;
    char* video_data_ptr = video_data;

    do {
        int pkt_eos = 0;
        int size = SIZE;
        if (video_data_ptr + size >= video_data_end) {
            pkt_eos = 1;
            size = video_data_end - video_data_ptr;
        }

        video_ctx->decoder->Decode((uint8_t*)video_data_ptr, size, pkt_eos);

        video_data_ptr += size;

        if (video_data_ptr >= video_data_end) {
            printf("reset decoder\n");
            break;
        }

        // LOGD("video_data_ptr=%p video_data_end=%p", video_data_ptr, video_data_end);
        // usleep(10*1000);
    } while (1);

    return 0;
}

#if defined(BUILD_VIDEO_RTSP)
void API_CALL OnTrackFrameOut(void* user_data, mk_frame frame)
{
    IvaAppContext* ctx = (IvaAppContext*)user_data;
    VideoAppContext* video_ctx = (VideoAppContext*)ctx->extData;
    if (video_ctx == NULL || video_ctx->decoder == NULL) {
        printf("decoder has not init");
        return;
    }

    const char* data = mk_frame_get_data(frame);
    size_t size = mk_frame_get_data_size(frame);

    video_ctx->decoder->Decode((uint8_t*)data, size, 0);
}

void API_CALL OnMkPlayEventFunc(void* userdata, int err_code, const char* err_msg, mk_track tracks[],
                                    int track_count)
{
    if (err_code == 0) {
        // success
        printf("play success!");
        int i;
        for (i = 0; i < track_count; ++i) {
            if (mk_track_is_video(tracks[i])) {
                log_info("got video track: %s", mk_track_codec_name(tracks[i]));
                // 监听track数据回调
                mk_track_add_delegate(tracks[i], OnTrackFrameOut, userdata);
            }
        }
    } else {
        printf("play failed: %d %s", err_code, err_msg);
    }
}

void API_CALL OnMkShutDownFunc(void* user_data, int err_code, const char* err_msg, mk_track tracks[],
                                  int track_count)
{
    printf("play interrupted: %d %s", err_code, err_msg);
}

int ProcessVideoRtsp(IvaAppContext* ctx, const char* url)
{
    mk_config config;
    memset(&config, 0, sizeof(mk_config));
    config.log_mask = LOG_CONSOLE;
    mk_env_init(&config);
    mk_player player = mk_player_create();
    mk_player_set_on_result(player, OnMkPlayEventFunc, ctx);
    mk_player_set_on_shutdown(player, OnMkShutDownFunc, ctx);
    mk_player_play(player, url);

    printf("enter any key to exit\n");
    getchar();

    if (player) {
        mk_player_release(player);
    }
    return 0;
}
#endif

int InitVideoCtx(IvaAppContext* ctx, char* video_url, int video_type) {
    // init video decoder
    MppDecoder* decoder = new MppDecoder();
    decoder->Init(video_type, 30, ctx);
    decoder->SetCallback(VideoDecoderFrameCallback);

    VideoAppContext* video_ctx = (VideoAppContext*)malloc(sizeof(VideoAppContext));
    video_ctx->decoder = decoder;
    video_ctx->video_url = video_url;
    video_ctx->video_type = video_type;
    ctx->extData = video_ctx;

    return 0;
}

int ReleaseVideoCtx(IvaAppContext* ctx) {
    VideoAppContext* video_ctx = (VideoAppContext*)ctx->extData;
    if (video_ctx == NULL) {
        return -1;
    }
    if (video_ctx->decoder != NULL) {
        delete (video_ctx->decoder);
        video_ctx->decoder = NULL;
    }

    free(ctx->extData);
    ctx->extData = NULL;

    return 0;
}

int main(int argc, char** argv)
{
    char* video_url = argv[1];
    int video_type = atoi(argv[2]);
    printf("video_url: %s video_type: %d\n", video_url, video_type);

    int ret = 0;

    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));

    ret = InitIvaApp(&iva_ctx);
    if (ret != 0) {
        return -1;
    }

    ret = InitVideoCtx(&iva_ctx, video_url, video_type);
    if (ret != 0) {
        return -1;
    }

    if (strncmp(video_url, "rtsp", 4) == 0) {
#if defined(BUILD_VIDEO_RTSP)
        ProcessVideoRtsp(&iva_ctx, video_url);
#else
        printf("rtsp no support\n");
#endif
    } else {
        ProcessVideoFile(&iva_ctx, video_url);
    }

    ReleaseVideoCtx(&iva_ctx);

    ReleaseIvaApp(&iva_ctx);

    DestroyImageBufferManager(iva_ctx.imgBufManager);
    return 0;
}
