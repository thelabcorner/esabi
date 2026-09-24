#include "../include/esabi/esabi.h"

ESABI_DIRECT_FUNCTION(esabi_cpp_smoke)
{
    (void)argv;
    (void)argc;
    esabi_value_set_i32(retval, 42);
    return ESABI_OK;
}

int main()
{
    esabi_value value;
    esabi_i32 result = 0;
    esabi_value_reset(&value);
    esabi_value_set_i32(&value, 42);
    return esabi_value_get_i32(&value, &result) && result == 42 ? 0 : 1;
}
