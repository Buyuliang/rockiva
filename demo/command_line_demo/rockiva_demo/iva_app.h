#ifndef __IVA_APP_H__
#define __IVA_APP_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaApp(IvaAppContext* iva_ctx);

int ReleaseIvaApp(IvaAppContext* iva_ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif