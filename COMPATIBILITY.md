# esabi compatibility and validation matrix

Evidence recorded: **2026-09-23**.

| Surface | Profile | Toolchain / target | Result | Evidence level |
|---|---|---|---|---|
| Windows x64 record layout | LONG32 | Clang 22.1.8, x86_64-pc-windows-msvc | 16-byte value; type@8; reserved@12 | compile + executable smoke |
| Windows x86 record layout | LONG32 | Clang 22.1.8, i686-pc-windows-msvc | compile passes under hostile default calling convention | compile + symbol inspection |
| Windows x64 exports | LONG32 | Clang 22.1.8 + PE linker | exact standard entry points + direct method | linked DLL + PE export inspection |
| Windows x86 exports | LONG32 | Clang 22.1.8 + PE linker, `-mrtd` | exact standard entry points + direct method | linked DLL + PE export inspection |
| Windows x86 exports | LONG32 | MSVC 19.44.35228 (VS 2022 Build Tools) | exact standard entry points + direct method | `cl.exe` + `link.exe` + `dumpbin` |
| C language floor | LONG32 | Clang C99 | passes | compile |
| C11 | LONG32 / LONG64 | Clang C11 | passes | compile + executable layout/helper smoke |
| MSVC frontend compatibility | LONG32 | clang-cl C11, `/W4 /WX` | passes | compile |
| C++ compatibility | LONG32 | Clang C++11 | passes | compile + executable smoke |
| Installed CMake package | LONG32 | Visual Studio 2022 consumer project | C99 + C++11 consumers resolve package, compile, and run | install + `find_package` consumer smoke |
| LONG64 structural profile | LONG64 | Clang C11 on 64-bit target | 24-byte value; type@8; reserved@16; 32-bit semantic range checks pass | compile + executable smoke |
| Illustrator 2026 Windows x64 direct ABI | LONG32 | Illustrator 2026 | esabi-only DLL loaded; `ping() = 42`; `ExternalObject.version = 1` | direct host runtime |
| Modern Adobe host on macOS LP64 | LONG64 | — | **not yet runtime verified** | explicit unknown |

## What “verified” means here

There are three different evidence classes and they should not be conflated:

- **Compile/layout verified** — the C/C++ compiler agrees with the declared sizes, offsets, alignment, and types.
- **Binary/export verified** — a linked DLL has the exact symbol names and calling-convention-sensitive shape expected.
- **Host runtime verified** — an Adobe host actually loads the library and exercises the ABI.

LONG32 on the current Windows x64 development path has all three categories represented across the existing ExternalObject work and this package's binary tests.

LONG64 currently has the first category only. esabi therefore requires explicit selection instead of advertising it as automatically compatible.

## Illustrator live-smoke note

A live Illustrator 2026 x64 smoke test on 2026-09-23 loaded a DLL built only from the esabi headers, returned `42` from an integer direct method, and surfaced `ESGetVersion() == 1` through `ExternalObject.version`.

The host retained the DLL image mapping after both `unload()` and `terminate()` in that session. This does not invalidate the ABI call test, but it means automated live-smoke tooling should compile/load from a disposable user-temp path rather than from the source tree and should not assume the file can be deleted until the host process naturally releases it.

## Permanent test command

    powershell -ExecutionPolicy Bypass -File tests/run.ps1

When Visual Studio Build Tools are installed, the script additionally invokes Microsoft's x86 compiler/linker and checks its export table. When they are absent, that optional lane is reported as skipped.
