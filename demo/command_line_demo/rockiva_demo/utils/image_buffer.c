#include <stdio.h>
#include <sys/time.h>

#include "image_buffer.h"

#include "rockiva_image.h"

typedef struct
{
    IvaImageBuf* buffer;
    int count;
    int isPrelloc;
} _ImageBufferManager;

static unsigned long GetTimeStampMS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static int CreateImageBuffer(_ImageBufferManager* m, int width, int height, RockIvaImageFormat fmt, int buf_type)
{
    RockIvaRetCode ret;
    for (int i = 0; i < m->count; i++) {
        IvaImageBuf* img_buf = &(m->buffer[i]);

        img_buf->image.info.width = width;
        img_buf->image.info.height = height;
        img_buf->image.info.format = fmt;

        ret = ROCKIVA_IMAGE_AllocMem(&(img_buf->image), (RockIvaMemType)buf_type);
        if (ret != ROCKIVA_RET_SUCCESS) {
            printf("alloc image memory fail\n");
            return -1;
        }

        img_buf->type = buf_type;
        img_buf->state = IMAGE_BUFFER_UNUSED;
        img_buf->index = i;
        img_buf->imageBufferManager = m;

        printf("create image buffer %d width=%d height=%d fmt=%d size=%d fd=%d virt_addr=%p\n", i, width, height,
               fmt, img_buf->image.size, img_buf->image.dataFd, img_buf->image.dataAddr);
    }
    return 0;
}

static int DestroyImageBuffer(_ImageBufferManager* m)
{
    if (m == NULL) {
        return -1;
    }
    for (int i = 0; i < m->count; i++) {
        IvaImageBuf* img_buf = &(m->buffer[i]);
        ROCKIVA_IMAGE_FreeMem(&img_buf->image);
    }
    return 0;
}

ImageBufferManager CreateImageBufferManager(int count)
{
    _ImageBufferManager* m = (_ImageBufferManager*)malloc(sizeof(_ImageBufferManager));
    m->isPrelloc = 0;
    m->count = count;
    m->buffer = (IvaImageBuf*)malloc(count * sizeof(IvaImageBuf));
    memset(m->buffer, 0, count * sizeof(IvaImageBuf));

    return (ImageBufferManager)m;
}

ImageBufferManager CreateImageBufferManagerPreAlloc(int count, int w, int h, int fmt, int buf_type)
{
    _ImageBufferManager* m = CreateImageBufferManager(count);

    // 图像缓冲区分配内存
    int ret = CreateImageBuffer(m, w, h, fmt, buf_type);
    if (ret != 0) {
        free(m->buffer);
        free(m);
        printf("CreateImageBuffer fail\n");
        return NULL;
    }
    m->isPrelloc = 1;

    return (ImageBufferManager)m;
}

int DestroyImageBufferManager(ImageBufferManager manager)
{
    if (manager == NULL) {
        return -1;
    }
    _ImageBufferManager* m = (_ImageBufferManager*)manager;
    if (m->buffer == NULL) {
        return -1;
    }
    if (m->isPrelloc) {
        DestroyImageBuffer(m);
    }
    free(m->buffer);
    m->buffer = NULL;
    free(m);
    return 0;
}

IvaImageBuf* GetImageBuffer(ImageBufferManager manager, int index)
{
    if (manager == NULL) {
        return NULL;
    }
    _ImageBufferManager* m = (_ImageBufferManager*)manager;
    if (index >= m->count) {
        return NULL;
    }
    return &(m->buffer[index]);
}

IvaImageBuf* AcquireImageBuffer(ImageBufferManager manager, unsigned long timeoutms)
{
    if (manager == NULL) {
        return NULL;
    }
    _ImageBufferManager* m = (_ImageBufferManager*)manager;
    
    unsigned long start_ts = GetTimeStampMS();

    while(1) {
        // printf("find unused imageBuf\n");
        for (int i = 0; i < m->count; i++) {
            IvaImageBuf* imgBuf = &(m->buffer[i]);
            // printf("imageBuf %d state=%d fd=%d virtAddr=%p\n", i, imgBuf->state, imgBuf->image.dataFd,
                // imgBuf->image.dataAddr);
            if (imgBuf->state != IMAGE_BUFFER_USING) {
                imgBuf->state = IMAGE_BUFFER_USING;
                return imgBuf;
            }
        }
        unsigned long cur_ts = GetTimeStampMS();
        if (cur_ts - start_ts > timeoutms) {
            printf("get free image buffer fail\n");
            break;
        }
        usleep(10*1000);
    }
    return NULL;
}

IvaImageBuf* FindImageBufferByFrameId(ImageBufferManager manager, int frameId)
{
    if (manager == NULL) {
        return NULL;
    }
    _ImageBufferManager* m = (_ImageBufferManager*)manager;
    for (int i = 0; i < m->count; i++) {
        if (m->buffer[i].image.frameId == frameId) {
            return &m->buffer[i];
        }
    }
    return NULL;
}

void RetureImageBuffer(ImageBufferManager manager, IvaImageBuf* imgBuf)
{
    if (manager == NULL) {
        return;
    }
    // printf("return imgBuf=%p", imgBuf);
    _ImageBufferManager* m = (_ImageBufferManager*)manager;
    if (m->isPrelloc == 0) {
        memset(&imgBuf->image, 0, sizeof(RockIvaImage));
    }
    imgBuf->extraInfo= NULL;
    imgBuf->state = IMAGE_BUFFER_UNUSED;
}
