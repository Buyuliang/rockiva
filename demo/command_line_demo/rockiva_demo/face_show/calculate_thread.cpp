#include "calculate_thread.h"

CALCULATE_Thread::CALCULATE_Thread(QWidget *parent):QThread(parent){
     this->parent = (videoshow*)parent;
}

void CALCULATE_Thread::run(){
    int ret = 0;
    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));
    ret = InitIvaApp(&iva_ctx);
    if (ret != 0) {
        return;
    }

    iva_ctx.imgBufManager =
        CreateImageBufferManagerPreAlloc(ALLOC_IMAGE_BUFFER_NUM, PIXWIDTH, PIXHEIGHT, ROCKIVA_IMAGE_FORMAT_RGB888, ALLOC_IMAGE_BUFFER_TYPE);

    for (int i = 0; ;i++) {
        IvaImageBuf* img_buf = AcquireImageBuffer(iva_ctx.imgBufManager, 5000);
        // printf("%lu before read image\n", GetTimeStampMS());
        memcpy(img_buf->image.dataAddr, (char *)this->parent->img.bits(), this->parent->img.byteCount());
        img_buf->image.frameId = i;
        img_buf->image.extData = img_buf;
        // printf("%lu PushFrame %d\n", GetTimeStampMS(), i);
        ret = ROCKIVA_PushFrame(iva_ctx.handle, &img_buf->image, NULL);
    next_frame:
        // about 1 fps
        usleep(10 * 1000);
        continue;
    }
    ReleaseIvaApp(&iva_ctx);
    DestroyImageBufferManager(iva_ctx.imgBufManager);
    // while(1) {
    //     process((char *)this->parent->img.bits(), this->parent->img.byteCount());
    // }
}

CALCULATE_Thread::~CALCULATE_Thread(){
}

int CALCULATE_Thread::process(char *buf, int len) {
    int ret = 0;
    IvaAppContext iva_ctx;
    memset(&iva_ctx, 0, sizeof(IvaAppContext));
    ret = InitIvaApp(&iva_ctx);
    if (ret != 0) {
        return -1;
    }

    iva_ctx.imgBufManager =
        CreateImageBufferManagerPreAlloc(ALLOC_IMAGE_BUFFER_NUM, PIXWIDTH, PIXHEIGHT, ROCKIVA_IMAGE_FORMAT_RGB888, ALLOC_IMAGE_BUFFER_TYPE);

    for (int i = 0; i < 1; i++) {
        IvaImageBuf* img_buf = AcquireImageBuffer(iva_ctx.imgBufManager, 5000);
        printf("%lu before read image\n", GetTimeStampMS());
        memcpy(img_buf->image.dataAddr, buf, len);
        img_buf->image.frameId = i;
        img_buf->image.extData = img_buf;
        printf("%lu PushFrame %d\n", GetTimeStampMS(), i);
        ret = ROCKIVA_PushFrame(iva_ctx.handle, &img_buf->image, NULL);
    next_frame:
        usleep(80 * 1000);
        continue;
    }
    ReleaseIvaApp(&iva_ctx);
    DestroyImageBufferManager(iva_ctx.imgBufManager);
    return 0;
}