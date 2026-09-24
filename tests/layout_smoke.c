#include <stddef.h>
#include <stdio.h>
#include <string.h>

#pragma pack(push, 1)
#include "../include/esabi/value.h"
typedef struct esabi_outer_pack_probe {
    char byte;
    int value;
} esabi_outer_pack_probe;
#pragma pack(pop)

static int check_int(const char *name, long long actual, long long expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: expected %lld, got %lld\n", name, expected, actual);
        return 1;
    }
    return 0;
}

static int check_helpers(void)
{
    esabi_value value;
    esabi_i32 i32 = 0;
    esabi_u32 u32 = 0;
    double real = 0.0;
    int boolean = 0;
    char text[] = "hello";

    memset(&value, 0xA5, sizeof(value));
    esabi_value_reset(&value);
    if (value.payload.raw_bits != 0 || value.type != ESABI_TYPE_UNDEFINED || value.reserved != 0) return 1;

    esabi_value_set_bool(&value, 99);
    if (!esabi_value_get_bool(&value, &boolean) || boolean != 1 || value.reserved != 0) return 1;

    esabi_value_set_i32(&value, -123456);
    if (!esabi_value_get_i32(&value, &i32) || i32 != -123456 || value.reserved != 0) return 1;
    if (!esabi_arg_get_i32(&value, 1, 0, &i32) || i32 != -123456) return 1;
    if (esabi_arg_at(&value, 1, 1) != NULL ||
        esabi_arg_at(&value, -1, 0) != NULL ||
        esabi_arg_at(NULL, 1, 0) != NULL) return 1;

    esabi_value_set_u32(&value, UINT32_C(4000000000));
    if (!esabi_value_get_u32(&value, &u32) || u32 != UINT32_C(4000000000)) return 1;

    esabi_value_set_double(&value, 123.25);
    if (!esabi_value_get_double(&value, &real) || real != 123.25) return 1;

    esabi_value_set_string(&value, text);
    if (esabi_value_get_string(&value) != text || esabi_value_get_script(&value) != NULL) return 1;

    esabi_value_set_script(&value, text);
    if (esabi_value_get_script(&value) != text || esabi_value_get_string(&value) != NULL) return 1;

    esabi_value_set_undefined(&value);
    if (!esabi_value_is(&value, ESABI_TYPE_UNDEFINED) ||
        !esabi_type_is_known(ESABI_TYPE_SCRIPT) ||
        esabi_type_is_known(9999)) return 1;

    if (strcmp(ESABI_SIGNATURE(example_name, ESABI_SIG_I32 ESABI_SIG_STRING), "example_name_ds") != 0 ||
        strcmp(ESABI_SIGNATURE0(example_name), "example_name_") != 0) return 1;

#if ESABI_ABI_PROFILE == ESABI_ABI_PROFILE_LONG64
    value.type = ESABI_TYPE_INTEGER;
    value.payload.signed_value = (esabi_long)INT32_MAX + 1;
    if (esabi_value_get_i32(&value, &i32)) return 1;
    value.type = ESABI_TYPE_UINTEGER;
    value.payload.unsigned_value = (esabi_ulong)UINT32_MAX + 1;
    if (esabi_value_get_u32(&value, &u32)) return 1;
#endif

    if (!esabi_error_is_ok(ESABI_OK) ||
        esabi_error_is_ok(ESABI_ERR_BAD_ARGUMENTS) ||
        !esabi_error_is_fatal(ESABI_ERR_OUT_OF_MEMORY) ||
        esabi_error_is_fatal(ESABI_ERR_BAD_ARGUMENTS)) return 1;

    return 0;
}

int main(void)
{
    int failures = 0;
    failures += check_int("sizeof(esabi_i32)", (long long)sizeof(esabi_i32), 4);
    failures += check_int("ABI long bits", (long long)(sizeof(esabi_long) * CHAR_BIT), ESABI_ABI_LONG_BITS);
    failures += check_int("sizeof(esabi_payload)", (long long)sizeof(esabi_payload), ESABI_PAYLOAD_SIZE);
    failures += check_int("sizeof(esabi_value)", (long long)sizeof(esabi_value), ESABI_VALUE_SIZE);
    failures += check_int("offsetof(payload)", (long long)offsetof(esabi_value, payload), 0);
    failures += check_int("offsetof(type)", (long long)offsetof(esabi_value, type), ESABI_VALUE_TYPE_OFFSET);
    failures += check_int("offsetof(reserved)", (long long)offsetof(esabi_value, reserved), ESABI_VALUE_RESERVED_OFFSET);
    failures += check_int("sizeof(reserved)", (long long)sizeof(((esabi_value *)0)->reserved), (long long)sizeof(esabi_long));
    failures += check_int("caller pack restored", (long long)sizeof(esabi_outer_pack_probe), 5);
    failures += check_int("ESABI_TYPE_STRING", ESABI_TYPE_STRING, 4);
    failures += check_int("ESABI_TYPE_INTEGER", ESABI_TYPE_INTEGER, 123);
    failures += check_int("ESABI_TYPE_SCRIPT", ESABI_TYPE_SCRIPT, 125);
    failures += check_int("ESABI_ERR_BAD_ARGUMENTS", ESABI_ERR_BAD_ARGUMENTS, 20);
    failures += check_int("ESABI_ERR_OUT_OF_MEMORY", ESABI_ERR_OUT_OF_MEMORY, -28);
    failures += check_int("helper contract", check_helpers(), 0);

    if (failures != 0) {
        fprintf(stderr, "esabi smoke test failed: %d mismatch(es)\n", failures);
        return 1;
    }

    printf("esabi %s OK: profile=%s(%d) long=%zu value=%zu payload=%zu pointer=%zu type@%zu reserved@%zu\n",
           ESABI_VERSION_STRING, ESABI_ABI_PROFILE_NAME, ESABI_ABI_PROFILE,
           sizeof(esabi_long), sizeof(esabi_value), sizeof(esabi_payload), sizeof(void *),
           offsetof(esabi_value, type), offsetof(esabi_value, reserved));
    return 0;
}
