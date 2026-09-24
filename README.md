<div align="center">

# ESABI: Modern C/C++ declarations for Adobe ExtendScript ExternalObject

### Header-only ABI definitions, checked value helpers, and verified Windows export contracts

[![ABI profiles](https://img.shields.io/badge/ABI%20profiles-2-success)](#compatibility)
[![Validation](https://img.shields.io/badge/validation-C99%20%7C%20C11%20%7C%20C%2B%2B11-purple)](#validation)
[![Exports](https://img.shields.io/badge/PE%20exports-x86%20%2B%20x64-success)](#validation)
[![Adobe](https://img.shields.io/badge/Adobe-Creative%20Suite-red?logo=adobe&logoColor=white)](https://extendscript.docsforadobe.dev/)
[![Windows ABI](https://img.shields.io/badge/Windows-LONG32%20verified-blue)](#compatibility)
[![Runtime](https://img.shields.io/badge/runtime%20library-none-orange)](#performance)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)

</div>

---

## Part Of The Same Toolkit

> Production-grade ExtendScript infrastructure for Illustrator-era JavaScript engines.

<table>
<tr>
<td width="50%" valign="top">

### Runtime Primitives

**[ESON](https://github.com/thelabcorner/eson)**<br>
Strict RFC 8259 JSON for ExtendScript.

**[ESB64](https://github.com/thelabcorner/es-b64)**<br>
Base64 and UTF-8 utilities.

**[ESARR](https://github.com/thelabcorner/es-arr)**<br>
ES5+ Array compatibility methods.

**[ESSTR](https://github.com/thelabcorner/es-str)**<br>
String whitespace and trim methods.

**[ESCHARS](https://github.com/thelabcorner/es-chars)**<br>
Native bulk byte operations.

**[ESHTTP](https://github.com/thelabcorner/es-http)**<br>
HTTP transport for ExtendScript automation.

**[ESTIMER](https://github.com/thelabcorner/es-timer)**<br>
Microsecond timing for ExtendScript automation.

**[ESRAND](https://github.com/thelabcorner/es-rand)**<br>
Deterministic random streams and sampling for ExtendScript.

</td>
<td width="50%" valign="top">

### Build & Integration Tools

**[ESPACK](https://github.com/thelabcorner/espack)**<br>
Self-extracting ExternalObject bundles.

**[ESMIN](https://github.com/thelabcorner/es-min)**<br>
Minification for shipped JSX bundles.

**[ESABI](https://github.com/thelabcorner/esabi)**<br>
Modern ExternalObject ABI declarations for native integrations.

**[VectorIPC](https://github.com/thelabcorner/vector-ipc)**<br>
Bounded local IPC for scripting hosts and native plug-ins.

**ESOBF** <sub>coming soon</sub><br>
Obfuscation for hardened JSX distribution.

</td>
</tr>
</table>

Also from the same team: **[ArcFit.dev](https://arcfit.dev)**, deterministic arc warp for Illustrator.

---

## Table of Contents

- [Why ESABI?](#why-esabi)
- [Features](#features)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [API](#api)
- [Validation](#validation)
- [Performance](#performance)
- [Security Model](#security-model)
- [Compatibility](#compatibility)
- [Engine quirks that shaped the design](#engine-quirks-that-shaped-the-design)
- [Development](#development)
- [Repository layout](#repository-layout)
- [Known limitations](#known-limitations)
- [Credits](#credits)
- [License](#license)

---

## Why ESABI?

Adobe's public ExternalObject C interface is historically expressed with native C types such as `long`, raw unions, host-required symbol names, and an 8-byte packing contract. That is enough to define interoperability, but it leaves modern consumers exposed to several easy mistakes:

- C `long` is 32 bits on Windows and 64 bits on LP64 platforms.
- Project-wide calling-convention flags can silently change x86 exports.
- A wrong packing state can produce a layout that compiles but is binary-incompatible.
- Directly assigning tags and union members makes stale payload bytes and filler fields easy to leave behind.
- String-return ownership is a host boundary, not ordinary C string ownership.
- The public sample header carries its own Adobe notice and provenance.

ESABI keeps the interoperability facts while providing an independently written, MIT-licensed declaration with explicit profiles, compile-time invariants, checked helpers, and reproducible binary tests.

Current version: **0.3.0**.

---

## Features

- **Two explicit ABI profiles**: LONG32 and LONG64. Non-Windows targets fail closed unless a profile is selected.
- **Verified Windows LONG32 layout**: 16-byte value record, type at byte 8, reserved field at byte 12.
- **Structurally tested LONG64 layout**: 24-byte value record, type at byte 8, reserved field at byte 16.
- **Compile-time ABI guards** for byte width, exact integer widths, IEEE-754 binary64, pointer width, field widths, offsets, packing, and alignment.
- **Explicit C linkage and cdecl** for host-facing Windows entry points and direct methods.
- **Exact export macros** for `ESInitialize`, `ESGetVersion`, `ESFreeMem`, `ESTerminate`, and direct methods.
- **Zero-allocation value helpers** that clear the complete payload and reserved field before assigning a tag.
- **Bounds-checked argument helpers** that validate `argv`, `argc`, index bounds, and type tags before access.
- **32-bit semantic integer safety** even when a wider ABI storage word is selected.
- **Signature builders** for ExternalObject initialization metadata, including unambiguous zero-argument signatures.
- **Opaque LiveObject handles** rather than an invented public pointee layout.
- **Header-only integration** with no ESABI runtime library and no ESABI allocator.

---

## Installation

### CMake with FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
    esabi
    GIT_REPOSITORY https://github.com/thelabcorner/esabi.git
    GIT_TAG v0.3.0
)

FetchContent_MakeAvailable(esabi)

target_link_libraries(my_externalobject PRIVATE esabi::esabi)
```

### Installed CMake package

```cmake
find_package(esabi 0.3 CONFIG REQUIRED)
target_link_libraries(my_externalobject PRIVATE esabi::esabi)
```

### Vendored headers

Copy `include/esabi/` into a pinned dependency directory and add that directory to the compiler include path:

```c
#include <esabi/esabi.h>
```

No source file or runtime library is required.

---

## Quick Start

```c
#include <esabi/esabi.h>

static char signatures[] =
    ESABI_SIGNATURE(add_one, ESABI_SIG_I32)
    ESABI_SIGNATURE_SEPARATOR
    ESABI_SIGNATURE0(ping);

ESABI_INITIALIZE_FUNCTION
{
    (void)argv;
    (void)argc;
    return signatures;
}

ESABI_VERSION_FUNCTION
{
    return 1;
}

ESABI_FREE_FUNCTION
{
    /* Match this to the allocator used for returned strings. */
    (void)pointer;
}

ESABI_TERMINATE_FUNCTION
{
}

ESABI_DIRECT_FUNCTION(add_one)
{
    esabi_i32 input;

    if (argc != 1 || !esabi_arg_get_i32(argv, argc, 0, &input)) {
        return ESABI_ERR_BAD_ARGUMENTS;
    }

    if (input == INT32_MAX) {
        return ESABI_ERR_RANGE;
    }

    esabi_value_set_i32(retval, input + 1);
    return ESABI_OK;
}

ESABI_DIRECT_FUNCTION(ping)
{
    (void)argv;
    (void)argc;
    esabi_value_set_i32(retval, 42);
    return ESABI_OK;
}
```

---

## API

### Core types

- `esabi_value`: the host value record.
- `esabi_payload`: the 8-byte payload slot.
- `esabi_long` / `esabi_ulong`: profile-selected ABI storage words.
- `esabi_i32` / `esabi_u32`: exact-width JavaScript integer semantics.
- `esabi_error`: host runtime status type.
- `esabi_live_object`: opaque LiveObject handle.

### Value helpers

`value.h` provides typed setters/getters for undefined, boolean, signed/unsigned 32-bit integers, double, string, script, and LiveObject values.

Every setter resets the value first, including the reserved field.

### Argument helpers

Typed `esabi_arg_get_*` helpers combine bounds checks with tag checks. Use them at direct-method boundaries instead of indexing `argv` before validating `argc`.

### Entry-point macros

- `ESABI_INITIALIZE_FUNCTION`
- `ESABI_VERSION_FUNCTION`
- `ESABI_FREE_FUNCTION`
- `ESABI_TERMINATE_FUNCTION`
- `ESABI_DIRECT_FUNCTION(name)`

### Type tags and runtime statuses

The documented ExternalObject type tags and runtime statuses are exposed under the `ESABI_` prefix. See [ABI.md](ABI.md) for the complete numeric contract.

---

## Validation

| Check | Command | Result |
|---|---|---|
| LONG32 record | `tests/run.ps1` | 16 bytes, type@8, reserved@12 |
| LONG64 record | `tests/run.ps1` | 24 bytes, type@8, reserved@16 |
| C language floor | `tests/run.ps1` | C99 compile passes |
| C11 helpers/layout | `tests/run.ps1` | compile + executable smoke passes |
| C++ boundary | `tests/run.ps1` | C++11 compile + executable smoke passes |
| MSVC frontend | `tests/run.ps1` | clang-cl `/W4 /WX` passes |
| Hostile x86 default convention | `tests/run.ps1` | direct method remains cdecl |
| Clang PE exports | `tests/run.ps1` | exact x86 + x64 entry-point names |
| Microsoft x86 exports | `tests/run.ps1` | `cl.exe` + `link.exe` + `dumpbin` pass when Build Tools are installed |
| Installed CMake package | CI + `tests/cmake-consumer` | C99 and C++11 consumers resolve `find_package`, compile, and run |
| Illustrator Windows x64 ABI behavior | prior live ExternalObject probes | LONG32 layout/tag behavior corroborated |

The permanent command is:

```powershell
powershell -ExecutionPolicy Bypass -File tests/run.ps1
```

See [COMPATIBILITY.md](COMPATIBILITY.md) for the evidence classes and exact toolchain matrix.

---

## Performance

ESABI has **no runtime library**. The low-level surface is declarations and constants; the helper layer is `static inline` and performs no allocation or I/O.

No throughput number is claimed because there is no independent ESABI execution engine to benchmark. Generated code quality remains the consumer compiler's responsibility.

---

## Security Model

ESABI does not open files, create sockets, spawn processes, evaluate scripts, allocate memory, or load shared libraries. It defines a native ABI boundary.

Security-sensitive responsibilities remain visible at the consumer boundary:

- validate argument counts and tags before use;
- bound application-owned payloads;
- ensure returned string pointers use the matching `ESFreeMem` deallocator;
- do not allow C++ exceptions to cross a C ABI boundary;
- treat LiveObject handles as opaque host-owned values.

---

## Compatibility

| Target | Status |
|---|---|
| Windows x64, LONG32 | layout, binary exports, and Illustrator 2026 host behavior verified |
| Windows x86, LONG32 | compile/link/export behavior verified with Clang and MSVC; Adobe host runtime not claimed |
| C99 | supported for the low-level/header helper surface |
| C11 | validated |
| C++11+ | validated |
| LONG64 on a 64-bit target | structurally validated; explicit opt-in |
| Modern Adobe host on macOS LP64 | not yet runtime verified |

Windows selects `ESABI_ABI_PROFILE_LONG32` automatically. Non-Windows builds must select a profile explicitly.

---

## Engine quirks that shaped the design

### Native `long` is not a portable ABI width

The historical C contract uses `long`. Windows uses LLP64, where `long` remains 32 bits on x64. LP64 platforms use a 64-bit `long`. ESABI therefore exposes ABI profiles instead of allowing a compiler target to silently redefine the record.

### Packing is part of the ABI

The value record requires 8-byte packing. ESABI wraps its record with `#pragma pack(push, 8)` / `#pragma pack(pop)` and tests inclusion from a hostile outer packing state.

### Calling convention can affect x86 exports

A project-wide non-default calling convention can alter x86 symbol decoration. ESABI explicitly pins cdecl and validates the resulting symbol and PE export table under a hostile default.

### Strings cross an ownership boundary

A returned string is not merely a pointer with a string tag. The library that allocates returned storage must implement `ESFreeMem` with the matching deallocator. ESABI intentionally does not hide that rule behind a generic allocator.

### Signature metadata does not replace runtime validation

Initialization signatures describe the exported surface, but direct methods must still validate `argc` and value tags. The checked argument helpers exist for that boundary.

---

## Development

Run the complete local verification matrix:

```powershell
powershell -ExecutionPolicy Bypass -File tests/run.ps1
```

Verify the installable CMake package:

```powershell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=install
cmake --install build
cmake -S tests/cmake-consumer -B consumer-build -DCMAKE_PREFIX_PATH=install
cmake --build consumer-build --config Release
```

The test runner uses a unique build directory per invocation so a DLL currently loaded by an Adobe host cannot block a later validation run.

---

## Repository layout

```text
esabi/
├── include/esabi/
│   ├── esabi.h
│   ├── externalobject.h
│   └── value.h
├── tests/
│   ├── compile_smoke.c
│   ├── cpp_smoke.cpp
│   ├── cmake-consumer/
│   ├── export_smoke.c
│   ├── layout_smoke.c
│   └── run.ps1
├── ABI.md
├── COMPATIBILITY.md
├── PROVENANCE.md
├── CHANGELOG.md
├── CMakeLists.txt
└── LICENSE
```

---

## Known limitations

- LONG64 is structurally modeled and tested, but no modern Adobe/macOS host runtime has been certified yet.
- The indirect object/client interface traditionally associated with the larger ExternalObject C API is outside the current scope. ESABI focuses on the shared value ABI and direct-access entry points.
- ESABI does not choose an allocator for returned strings.
- Historical Adobe sample material may still exist in research/reference trees; production consumers can depend on ESABI instead.

---

## Credits

The interoperability contract is grounded in Adobe's public ExtendScript documentation and the public Adobe CEP sample repository. See [PROVENANCE.md](PROVENANCE.md) for exact source links, the reference commit used during compatibility research, and the distinction between interface facts and ESABI-specific design.

---

## License

MIT. See [LICENSE](LICENSE).
