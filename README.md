# ELF Security Analyzer

A C++17-based ELF binary security analysis tool focused on safe parsing, hardening inspection, and reproducible validation.

- Current implementation: ELF64 little-endian file-header parsing and basic validation.
- Planned security analysis: NX, PIE and RELRO hardening inspection.
- Long-term direction: binary vulnerability analysis and fuzzing research.

> **Status:** M1 (ELF64 header parsing) completed on 24 November 2026.
> The analyzer currently reads and checks the 64-byte ELF64 file header and prints its fields.
> It does not yet parse program headers, section headers or the dynamic segment, and it does
> not yet report NX, PIE or RELRO. Everything marked *planned* below describes intended behavior,
> not existing features.

## What works today

- Reads the input as untrusted data in binary mode and never executes it.
- Rejects files shorter than a full 64-byte ELF64 header.
- Checks the ELF magic number `7F 45 4C 46`.
- Validates the identification bytes and reports two kinds of rejection:
  - `invalid`: the value is not allowed by the ELF format;
  - `unsupported`: the value is valid ELF, but this tool cannot process it yet (ELF32, big-endian).
- Decodes all 13 numeric header fields with explicit little-endian byte assembly
  (no casting the raw buffer to a struct, so results do not depend on host byte order or struct padding).
- Checks `e_version` and `e_ehsize` before printing a report.
- Prints a readable report with basic names for `e_type` and `e_machine`.
- Builds with CMake; an optional build enables AddressSanitizer and UndefinedBehaviorSanitizer.
- Ships automated regression tests run through CTest.

### Limits of the current version

- Only the file header is validated. Offsets such as `e_phoff` and `e_shoff` are printed but not yet
  checked against the file size. Bounds and integer-overflow checks for them will start in M2.
- `e_type` = `DYN` does not mean the file is a PIE: shared libraries are `DYN` too. PIE detection
  needs dynamic-segment evidence and is planned for M3.
- Only ELF64 little-endian files are supported.

## What it will do (v1.0, planned)

- Parse program headers, section headers and the dynamic segment, with strict bounds and integer-overflow checks.
- Report three hardening properties (NX, PIE and RELRO) together with the evidence behind each finding.
- Report `unknown` when the file does not contain enough evidence, instead of guessing.

### Not in scope for v1.0

Exploitation, payloads, disassembly, decompilation, dynamic tracing, fuzzing, automatic vulnerability discovery, a GUI, and support for every ELF class / byte order / architecture.

These are left out on purpose, to keep v1.0 small enough to test thoroughly.

## Roadmap

| Milestone | Focus | Status |
|---|---|---|
| M0: Foundations | Linux executables, bytes and hex, ELF basics, magic-number check | ✅ Completed |
| M1: ELF header | Safe file access, `e_ident` validation, ELF64 header parsing, CMake, automated tests | ✅ Completed |
| M2: Structures and dynamic segment | Program headers, section headers, dynamic segment (`DT_FLAGS`, `DT_FLAGS_1`, `DT_BIND_NOW`), bounds checks | ⬜ Planned |
| M3: Hardening analysis | NX, PIE, RELRO with evidence and `unknown` states | ⬜ Planned |
| M4: Quality and release | Wider tests, validation against `readelf`, documentation, tagged v1.0 | ⬜ Planned |

## Build and run

Requirements: Linux x86-64, a C++17 compiler (tested with GCC 15), CMake ≥ 3.16, and Python 3 for the tests.

```bash
# Normal build
cmake -S . -B build -DENABLE_SANITIZERS=OFF
cmake --build build
(cd build && ctest --output-on-failure)

# Build with AddressSanitizer + UndefinedBehaviorSanitizer
cmake -S . -B build-sanitize -DENABLE_SANITIZERS=ON
cmake --build build-sanitize
(cd build-sanitize && ctest --output-on-failure)
```

```bash
./build/elf-analyzer /bin/ls
```

The program takes exactly one argument: the path of the file to inspect.

### Exit codes

| Exit code | Meaning |
|---|---|
| `0` | Supported ELF64 little-endian header passed the implemented checks; report printed |
| `1` | Input was read, but rejected: too short, magic mismatch, `invalid` or `unsupported` |
| `2` | The tool could not run: wrong arguments, file cannot be opened, or file cannot be read (e.g. a directory) |

All rejection and error messages are written to stderr; the report is written to stdout.

### Example output

Real output on the author's machine for `labs/m0/hello` (built from `labs/m0/hello.cpp`).
Exact values depend on the compiler and system that built the sample.

```text
ELF Header:
Class: ELF64
Data: Little endian
e_type      = 3
Type: DYN (3)
e_machine   = 62
Machine: AMD x86-64 (62)
e_version   = 1
e_entry     = 0x1060
e_phoff     = 64
e_shoff     = 14136
e_flags     = 0
e_ehsize    = 64
e_phentsize = 56
e_phnum     = 14
e_shentsize = 64
e_shnum     = 31
e_shstrndx  = 30
```

Before the report, the tool currently also prints the 64 raw header bytes and `ELF magic: match`
(kept from the learning stage).

## Validation rules

Checks run in this order; the first failing check stops the program.

| # | Check | Result if it fails |
|---|---|---|
| 1 | Exactly one file argument | `usage: elf-analyzer <file>`, exit 2 |
| 2 | File can be opened | `error: cannot open ...`, exit 2 |
| 3 | File can be read (not a directory, etc.) | `error: cannot read ...`, exit 2 |
| 4 | At least 64 bytes | `ELF header: too short (N bytes)`, exit 1 |
| 5 | Magic `7F 45 4C 46` | `ELF magic: mismatch`, exit 1 |
| 6 | `EI_CLASS` (byte 4) = 2 | `1` → `unsupported: ELF32`; other → `invalid`; exit 1 |
| 7 | `EI_DATA` (byte 5) = 1 | `2` → `unsupported: big-endian`; other → `invalid`; exit 1 |
| 8 | `EI_VERSION` (byte 6) = 1 | `invalid`, exit 1 |
| 9 | `e_version` = 1 | `invalid`, exit 1 |
| 10 | `e_ehsize` = 64 | `invalid`, exit 1 |

The identification bytes (4–6) are checked before the numeric fields are decoded,
so the decoder only ever runs on input that it is meant to understand.

## Architecture

```text
src/main.cpp        CLI: arguments, file reading, validation, exit codes
   │
   ├── src/elf_parser.h/.cpp   Elf64Header struct, read_u16/u32/u64, parse_elf64_header()
   │                           (decodes bytes into data; prints nothing)
   │
   └── src/elf_report.h/.cpp   print_elf_header(const Elf64Header&)
                               (turns parsed data into readable text)
```

The parser and the report share only the `Elf64Header` structure. Validation is currently in
`main.cpp`; it will only be moved into its own function or module when M2 adds enough checks to justify it.

## Testing

Tests are black-box tests of the command-line tool, written with Python's standard `unittest`
and registered with CTest. The analyzer binary built by CMake is passed to the tests through the
`ELF_ANALYZER` environment variable, so each build (normal or sanitizer) tests its own binary.

To stay reproducible on any Linux x86-64 machine, the tests use the newly built analyzer itself
as the valid ELF64 sample, and create every malformed input in a temporary directory.

8 test methods cover 14 scenarios:

| Area | Scenarios | Expected |
|---|---|---|
| Valid input | the analyzer's own ELF64 binary | exit 0, report printed |
| Identification bytes | ELF32, unknown class, big-endian, unknown byte order, bad `EI_VERSION` (one byte changed each) | exit 1, matching `invalid` / `unsupported` message |
| Header fields | bad `e_version`, bad `e_ehsize` | exit 1, `invalid` message |
| Length | 4-byte file (magic only), 63-byte file (one byte short) | exit 1, `too short` |
| Not ELF | 64 bytes of `A` | exit 1, `mismatch` |
| CLI errors | no arguments, missing file, directory as input | exit 2 |

Changing a single identification byte checks that the rejection path works; it does not build
a genuine ELF32 or big-endian file.

All 8 tests pass in both the normal and the sanitizer build, with no sanitizer reports.
In addition, all 13 numeric fields printed for `labs/m0/hello` were cross-checked against `readelf -h`
and matched.

## Repository layout

```text
CMakeLists.txt     Build, sanitizer option, CTest registration
src/               Analyzer source code (main, parser, report)
tests/             Automated CLI tests (Python unittest, run by CTest)
labs/              Learning experiments, one folder per milestone (not part of the tool)
  m0/              ELF magic-number check and hand-made samples
  m1/              Single-file prototype of the ELF64 header parser
docs/              Design notes and decisions (planned)
```

## Learning labs

The `labs/` folder keeps the small programs written while learning, so the history of how the
analyzer grew stays visible. They are not used by the build.

### M0 lab: ELF magic-number check

`labs/m0/elf_magic.cpp` reads the first 4 bytes of a file and compares them with the ELF magic number `7F 45 4C 46` (`\x7FELF`).
A match only means the first four bytes are correct. It does not mean the file is a valid ELF.

```bash
cd labs/m0
g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined elf_magic.cpp -o elf_magic
./elf_magic hello.cpp    # replace hello.cpp with the path of any file to check
```

Results verified on 3 Nov 2026 with a sanitizer build:

| Input | Purpose | Output | Exit |
|---|---|---|---|
| `hello` (compiled from `hello.cpp`) | Real ELF executable | `ELF magic: match` | 0 |
| `hello.cpp` | Text file | `ELF magic: mismatch` | 1 |
| `empty.bin` (0 bytes) | Empty file | `ELF magic: too short (0 bytes)` | 1 |
| `short.bin` (`7F 45 4C`) | Truncated magic | `ELF magic: too short (3 bytes)` | 1 |
| `fake.bin` (`7F 45 4C 46`) | Correct magic, no ELF header | `ELF magic: match` | 0 |
| `mismatch.bin` (`7F 45 4C 58`) | Only the last byte differs | `ELF magic: mismatch` | 1 |

`fake.bin` shows the limit of a magic-number check. The M1 analyzer rejects it as
`too short (4 bytes)`, because a full ELF64 header needs 64 bytes.

The samples can be regenerated (`hello` is built from `hello.cpp` and is not committed):

```bash
cd labs/m0
: > empty.bin                    # 0 bytes
printf '\x7fEL'  > short.bin     # 3 bytes: truncated magic
printf '\x7fELF' > fake.bin      # 4 bytes: correct magic only
printf '\x7fELX' > mismatch.bin  # 4 bytes: last byte wrong
g++ -std=c++17 hello.cpp -o hello  # real ELF sample (not committed)
```

### M1 lab: single-file header parser

`labs/m1/elf_header.cpp` is the single-file prototype in which the header struct, the little-endian
readers and the validation order were first worked out, before being split into `src/`.

## Design principles

1. Separate parsing from presentation: parsers return data or explicit errors; they do not print.
2. Validate before reading: check file size, offsets, entry sizes, counts and integer overflow first.
3. Be honest about uncertainty: insufficient evidence produces `unknown`, not yes/no.
4. Keep modules small: no speculative abstractions before a working prototype.
5. Never execute analyzed files: inspection is static and read-only.

## Development environment

Developed on Ubuntu under WSL2 (Windows), with:

- g++ 15.2.0, clang 21.1.8
- CMake 4.2.3
- Python 3 (for the tests)
- GNU Binutils 2.46 (`readelf`, used as a cross-checking reference)
- AddressSanitizer and UndefinedBehaviorSanitizer enabled in the sanitizer build

## Responsible use

This project is intended for static analysis of ELF binaries you built yourself or are authorized to examine.
It is not a substitute for a professional security audit.

## About this project

An open-source C++17 project developed by a cybersecurity undergraduate, focusing on ELF internals, defensive parsing and binary security analysis.
The repository documents both the implementation and the engineering decisions behind the tool.

## Development and AI Assistance

AI assistance was used for technical guidance, code examples, implementation suggestions and code review. The author is responsible for the implementation, design decisions and validation of the tool.

**How results are verified**

- Technical claims are not accepted from AI output alone. The project's verification sources are the ELF gABI, GNU `readelf` and local tests.
- Header values are cross-checked against `readelf -h`; in M0 by reading bytes by hand, in M1 by comparing the analyzer's report field by field.
- Tests are run locally by the author through CTest in both the normal and the sanitizer build.

## License

[MIT](LICENSE)

## References

- [ELF Object File Format (generic ABI)](https://gabi.xinuos.com/)
- [GNU Binutils: `readelf`](https://sourceware.org/binutils/docs/binutils/readelf.html)
- [CMake documentation](https://cmake.org/documentation/)
- [Clang AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
- [Clang UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
