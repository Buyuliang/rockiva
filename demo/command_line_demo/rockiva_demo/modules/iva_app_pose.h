#ifndef __IVA_APP_POSE_H__
#define __IVA_APP_POSE_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaPose(IvaAppContext* ctx);

int ReleaseIvaPose(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif