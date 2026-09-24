# Changelog

All notable changes to esabi are documented here.

## 0.3.1 - 2026-09-23

- Expanded the ExternalObject export smoke test to cover every documented direct-method shape and standard entry point, with exact export assertions on x86 and x64.
- Documented ESRAND in the shared toolkit listing.

## 0.3.0 - 2026-09-23

- Introduced explicit LONG32 and LONG64 ABI profiles instead of relying on platform-dependent C `long` width implicitly.
- Added compile-time checks for byte width, integer representation, IEEE-754 binary64, pointer width, record size, field offsets, packing, and alignment.
- Added exact host entry-point and direct-method declaration macros with explicit Windows cdecl behavior.
- Added zero-allocation typed value and argument helpers.
- Added C99, C11, C++11, clang-cl, x86/x64 PE export, hostile calling-convention, and native MSVC x86 validation.
- Added an installable header-only CMake package with C99 and C++11 installed-package consumer tests.
- Added ABI, compatibility, and provenance documentation.
