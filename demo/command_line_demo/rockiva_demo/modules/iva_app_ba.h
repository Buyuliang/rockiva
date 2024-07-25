#ifndef __IVA_APP_BA_H__
#define __IVA_APP_BA_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaBa(IvaAppContext* ctx);

int ReleaseIvaBa(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif