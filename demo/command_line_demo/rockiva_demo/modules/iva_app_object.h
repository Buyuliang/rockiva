#ifndef __IVA_APP_OBJECT_H__
#define __IVA_APP_OBJECT_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaObject(IvaAppContext* ctx);

int ReleaseIvaObject(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif