#ifndef __IVA_APP_PLATE_H__
#define __IVA_APP_PLATE_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaPlate(IvaAppContext* ctx);

int ReleaseIvaPlate(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif