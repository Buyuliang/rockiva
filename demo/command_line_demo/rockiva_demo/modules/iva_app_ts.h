#ifndef __IVA_APP_TS_H__
#define __IVA_APP_TS_H__

#include "iva_app_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

int InitIvaTs(IvaAppContext* ctx);

int ReleaseIvaTs(IvaAppContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif