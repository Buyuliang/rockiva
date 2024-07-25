#ifndef __IVA_APP_CTX_H__
#define __IVA_APP_CTX_H__

#include "rockiva_common.h"
#include "utils/image_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MODEL_DATA_PATH "/data/rockiva_data"
#define LICENSE_PATH "/data/key.lic"
#define FACE_DATABASE_PATH "/data/face.db"

#define OUT_DIR_PATH "./out"
#define OUT_FRAMES_PATH "./out/frames"
#define OUT_CAPTURE_PATH "./out/captures"

// 为输入图像分配的内存类型：0:CPU 1:DMA
#define ALLOC_IMAGE_BUFFER_TYPE 1
#define ALLOC_IMAGE_BUFFER_NUM 3

#define IVA_APP_DET_EN 0    /* 目标检测 */
#define IVA_APP_BA_EN 0     /* 周界 */
#define IVA_APP_FACE_EN 1   /* 人脸 */
#define IVA_APP_PLATE_EN 0  /* 车牌 */
#define IVA_APP_TS_EN 0     /* 客流统计 */
#define IVA_APP_OBJECT_EN 0 /* 目标检测（非机动车） */
#define IVA_APP_POSE_EN 0   /* 摔倒检测 */

typedef struct
{
    RockIvaHandle handle;
    RockIvaInitParam commonParams;
    void* detData;
    void* baData;
    void* faceData;
    void* plateData;
    void* tsData;
    void* poseData;
    void* objectData;
    ImageBufferManager imgBufManager;
    void* extData;
} IvaAppContext;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif