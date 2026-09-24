# esabi ABI contract

This document is the normative design description for esabi's public binary boundary.

## 1. ABI word profiles

The historical ExternalObject interface is expressed in terms of C `long`. C `long` is not a portable-width type, so esabi makes its width an explicit ABI choice.

### LONG32

`ESABI_ABI_PROFILE_LONG32`

- `esabi_long`: signed 32-bit
- `esabi_ulong`: unsigned 32-bit
- payload: 8 bytes
- value: 16 bytes, alignment 8
- type: offset 8, width 4
- reserved: offset 12, width 4
- default on Windows

### LONG64

`ESABI_ABI_PROFILE_LONG64`

- `esabi_long`: signed 64-bit
- `esabi_ulong`: unsigned 64-bit
- payload: 8 bytes
- value: 24 bytes, alignment 8
- type: offset 8, width 8
- reserved: offset 16, width 8
- never selected implicitly by esabi

LONG64 is a structural model of the public native-`long` contract on an LP64 ABI. It is not currently marked as Adobe-host runtime verified.

## 2. Value record

The public type is:

    typedef struct esabi_value {
        esabi_payload payload;
        esabi_type    type;
        esabi_long    reserved;
    } esabi_value;

The payload is always exactly 8 bytes and can represent:

- ABI integer/boolean storage
- IEEE-754 binary64
- UTF-8 string/script pointer
- opaque LiveObject pointer
- raw 64-bit storage used internally for deterministic clearing

The record uses 8-byte packing. esabi pushes and restores packing so including it from a translation unit using another pack setting does not contaminate either side.

## 3. Semantic integer width

The ABI storage word and JavaScript integer semantic width are intentionally separate concepts.

`ESABI_TYPE_INTEGER` and `ESABI_TYPE_UINTEGER` represent signed/unsigned **32-bit** values. Therefore:

- `esabi_value_set_i32` and `esabi_value_set_u32` accept exact-width 32-bit values.
- LONG64 stores those values in the wider ABI slot.
- LONG64 getters reject host values outside the signed/unsigned 32-bit semantic range rather than truncating them.

## 4. Type tags

| esabi name | Value |
|---|---:|
| `ESABI_TYPE_UNDEFINED` | 0 |
| `ESABI_TYPE_BOOL` | 2 |
| `ESABI_TYPE_DOUBLE` | 3 |
| `ESABI_TYPE_STRING` | 4 |
| `ESABI_TYPE_LIVE_OBJECT` | 6 |
| `ESABI_TYPE_LIVE_OBJECT_RELEASE` | 7 |
| `ESABI_TYPE_INTEGER` | 123 |
| `ESABI_TYPE_UINTEGER` | 124 |
| `ESABI_TYPE_SCRIPT` | 125 |

Unknown tags are never silently coerced by the helper layer.

## 5. Direct method shape

A direct method has the ABI shape:

    esabi_error method(
        esabi_value *argv,
        esabi_long argc,
        esabi_value *retval
    );

Use `ESABI_DIRECT_FUNCTION(name)` to pin:

- C language linkage
- export visibility
- ABI word width
- Windows cdecl
- a non-throwing C++ boundary

The function-pointer alias is `esabi_function`.

## 6. Standard entry points

The host-facing names are fixed:

- `ESInitialize`
- `ESGetVersion`
- `ESFreeMem`
- `ESTerminate`

Use the corresponding `ESABI_*_FUNCTION` macros for declarations and definitions.

## 7. String ownership

String and script values are UTF-8, null-terminated pointers at the ABI boundary.

The setter helpers do **not** allocate, duplicate, retain, or transfer ownership. When returned storage is expected to be released by the host, the library's `ESFreeMem` must use the deallocator paired with the allocator that produced the pointer.

This rule is intentionally not hidden behind a convenience allocator.

## 8. Script values

`ESABI_TYPE_SCRIPT` is represented by the same pointer payload as a string but has distinct host semantics. esabi therefore exposes separate string and script setters/getters and does not merge the tags.

## 9. Errors

`ESABI_OK` is zero.

The documented runtime statuses are exposed under esabi-prefixed names. `esabi_error_is_fatal(error)` classifies negative statuses as fatal. The helper does not remap or synthesize host error numbers.

## 10. Initialization signatures

esabi exposes the documented signature characters and compile-time string builders:

    ESABI_SIGNATURE(method, ESABI_SIG_I32 ESABI_SIG_STRING)
    ESABI_SIGNATURE0(ping)

Multiple entries can be joined with `ESABI_SIGNATURE_SEPARATOR`.

These strings are metadata for the ExternalObject surface; they are not a replacement for validating `argc` and actual value tags inside native methods.

## 11. Argument helpers

`esabi_arg_at` checks:

1. `argv != NULL`
2. `argc >= 0`
3. `index >= 0`
4. `index < argc`

Only then does it perform pointer arithmetic.

Typed argument helpers compose that bounds check with a tag check.

## 12. Compile-time invariants

Compilation fails when any selected profile violates its required:

- 8-bit bytes
- 32-bit semantic integer types
- ABI word width
- IEEE-754 binary64 representation
- supported pointer width
- LiveObject handle pointer width
- payload size
- value size
- field offsets
- value alignment

This is intentional. A compile error is safer than emitting a shared library with a plausible-looking but incompatible record.

## 13. Platform selection

Windows automatically selects LONG32 because Windows uses a 32-bit C `long` on both x86 and x64.

On non-Windows platforms esabi does not guess. Define `ESABI_ABI_PROFILE` explicitly before including the header.

That policy is deliberate: the public Adobe documentation still expresses the ABI with native C `long`, while modern LP64 systems give `long` a different width. Until a modern host/platform pair is runtime-probed, silent auto-selection would turn uncertainty into an ABI claim.
