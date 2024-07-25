#ifndef __IVA_APP_FACE_H__
#define __IVA_APP_FACE_H__

#include "iva_app_ctx.h"
#include "rockiva_face_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t objNum;                                         /* 图片数目 */
    char faceInfo[ROCKIVA_FACE_MAX_FACE_NUM][50];     /* 图片字符串数组 */
} PicResult;

typedef struct Person {
    uint32_t count;
    char first_time[ROCKIVA_FACE_INFO_SIZE_MAX];
    char last_time[ROCKIVA_FACE_INFO_SIZE_MAX];
    struct Person *next;
} Person;

typedef struct Node {
    uint32_t count;
    char first_time[ROCKIVA_FACE_INFO_SIZE_MAX];
    char last_time[ROCKIVA_FACE_INFO_SIZE_MAX];
    char faceInfo[ROCKIVA_FACE_MAX_FACE_NUM];
    struct Node* next;
} Node;

extern Node *node_head;



extern int update_pic;
extern PicResult picresult;
extern int update_pic_num;
extern int old_update_pic_num;
extern int update_time;
extern RockIvaFaceDetResult detresult;
extern RockIvaFaceCapResults capresult;
extern RockIvaFaceIdInfo faceinforesult;
extern Person *head;

int InitIvaFace(IvaAppContext* ctx);

int ReleaseIvaFace(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif