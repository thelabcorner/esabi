#include "../include/esabi/externalobject.h"

static const char esabi_compile_signature[] =
    ESABI_SIGNATURE(esabi_compile_smoke, ESABI_SIG_I32);

ESABI_DIRECT_FUNCTION(esabi_compile_smoke)
{
    (void)argv;
    (void)argc;
    (void)esabi_compile_signature;
    if (retval != NULL) {
        retval->payload.raw_bits = UINT64_C(0);
        retval->type = ESABI_TYPE_UNDEFINED;
        retval->reserved = 0;
    }
    return ESABI_OK;
}
