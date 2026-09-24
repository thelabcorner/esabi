#include "../include/esabi/value.h"

/*
 * MSVC-ABI objects that execute floating-point operations reference this
 * runtime sentinel even when no CRT functionality is otherwise required.
 * Keep it test-local so the export smoke DLL can remain /NODEFAULTLIB.
 */
#if defined(_WIN32)
int _fltused = 0;
#endif

static char esabi_text_result[] = "esabi";
static char esabi_script_result[] = "40 + 2";
static esabi_long esabi_initializer_argc = 0;

static char esabi_export_signatures[] =
    ESABI_SIGNATURE0(ping)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE(add_one, ESABI_SIG_I32)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE(invert_bool, ESABI_SIG_BOOL)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE(half_value, ESABI_SIG_DOUBLE)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE0(text_value)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE0(script_value)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE0(init_argc);

ESABI_INITIALIZE_FUNCTION
{
    (void)argv;
    esabi_initializer_argc = argc;
    return esabi_export_signatures;
}

ESABI_VERSION_FUNCTION
{
    return 1;
}

/*
 * The smoke-test string/script payloads live in static storage, so there is
 * intentionally nothing to release. Production code must pair ESFreeMem with
 * the allocator used for returned dynamic strings.
 */
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
    if (argc != 0) return ESABI_ERR_BAD_ARGUMENTS;
    esabi_value_set_i32(retval, 42);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(add_one)
{
    esabi_i32 value;

    if (argc != 1 || !esabi_arg_get_i32(argv, argc, 0, &value)) {
        return ESABI_ERR_BAD_ARGUMENTS;
    }
    if (value == INT32_MAX) {
        return ESABI_ERR_RANGE;
    }

    esabi_value_set_i32(retval, value + 1);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(invert_bool)
{
    int value;

    if (argc != 1 || !esabi_arg_get_bool(argv, argc, 0, &value)) {
        return ESABI_ERR_BAD_ARGUMENTS;
    }

    esabi_value_set_bool(retval, !value);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(half_value)
{
    double value;

    if (argc != 1 || !esabi_arg_get_double(argv, argc, 0, &value)) {
        return ESABI_ERR_BAD_ARGUMENTS;
    }

    esabi_value_set_double(retval, value * 0.5);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(text_value)
{
    (void)argv;
    if (argc != 0) return ESABI_ERR_BAD_ARGUMENTS;
    esabi_value_set_string(retval, esabi_text_result);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(script_value)
{
    (void)argv;
    if (argc != 0) return ESABI_ERR_BAD_ARGUMENTS;
    esabi_value_set_script(retval, esabi_script_result);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(init_argc)
{
    (void)argv;
    if (argc != 0) return ESABI_ERR_BAD_ARGUMENTS;

    if (esabi_initializer_argc < 0 ||
        esabi_initializer_argc > (esabi_long)INT32_MAX) {
        return ESABI_ERR_RANGE;
    }

    esabi_value_set_i32(retval, (esabi_i32)esabi_initializer_argc);
    return ESABI_OK;
}
