#include "../include/esabi/value.h"

static char esabi_export_signatures[] =
    ESABI_SIGNATURE0(ping);

ESABI_INITIALIZE_FUNCTION
{
    (void)argv;
    (void)argc;
    return esabi_export_signatures;
}

ESABI_VERSION_FUNCTION
{
    return 1;
}

ESABI_FREE_FUNCTION
{
    (void)pointer;
}

ESABI_TERMINATE_FUNCTION
{
}

ESABI_DIRECT_FUNCTION(ping)
{
    (void)argv;
    (void)argc;
    esabi_value_set_i32(retval, 42);
    return ESABI_OK;
}