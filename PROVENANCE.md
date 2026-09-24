# esabi provenance

esabi is an independently organized and written interoperability declaration. It does not reproduce Adobe's sample comments, structure names, macro names, or license notice.

The numeric tags, status values, exported entry-point names, calling shapes, signature characters, and value-record requirements are interoperability facts needed to communicate with an ExternalObject host. esabi supplies its own naming, organization, comments, profile system, assertions, helper API, and tests around those facts.

## Public interoperability sources

Primary documentation:

- https://extendscript.docsforadobe.dev/integrating-external-libraries/defining-entry-points-for-direct-access/
- https://extendscript.docsforadobe.dev/integrating-external-libraries/defining-entry-points-for-indirect-access/
- https://extendscript.docsforadobe.dev/integrating-external-libraries/loading-and-using-shared-libraries/

Public Adobe sample repository:

- https://github.com/Adobe-CEP/CEP-Resources
- Reference snapshot used during compatibility research: `ab5e4e3e53a42fad08e1225a22a991bb1ffe73f6`
- Historical sample tree: `ExtendScript-Toolkit/Samples/cpp/`

Runtime observations from Illustrator 2026 on Windows x64 were also used to corroborate the modern LONG32 layout and tag behavior.

## Important width distinction

The public C interface is historically written using `long`. That is not a fixed-width type across modern ABIs.

esabi therefore does **not** copy the source-level `long` choice blindly. It represents ABI word width through explicit LONG32/LONG64 profiles and keeps JavaScript integer semantics separately typed as exact-width `int32_t` / `uint32_t`.

The LONG64 profile is a structural interoperability model, not a claim that a specific modern Adobe/macOS host has been runtime verified.

## esabi-specific design

The following are esabi design decisions rather than copied source expression:

- explicit LONG32/LONG64 ABI profiles;
- fail-closed non-Windows profile selection;
- exact-width semantic integer aliases;
- a raw 64-bit payload view for deterministic clearing;
- an opaque LiveObject handle type;
- explicit cdecl protection on Windows;
- GCC/Clang default-visibility support;
- compile-time machine-representation and layout assertions;
- non-throwing C++ boundary declarations;
- entry-point definition macros;
- signature construction macros;
- checked inline value access;
- bounds-checked argument extraction;
- range checks between LONG64 storage and 32-bit integer semantics;
- independent naming and documentation throughout.

## Repository hygiene

Adobe's public sample header can remain a research/reference artifact where its license and provenance are preserved. Production projects can depend on esabi instead, avoiding a need to redistribute that sample header as part of their native ABI surface.
