#include <esabi/esabi.h>

int main(void)
{
    esabi_value value;
    esabi_i32 out = 0;

    esabi_value_set_i32(&value, 42);
    return esabi_value_get_i32(&value, &out) && out == 42 ? 0 : 1;
}
