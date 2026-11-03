# ELF Security Analyzer

A C++17-based ELF binary security analysis tool focused on safe parsing, hardening inspection, and reproducible validation.

- **Current scope:** ELF parsing and security hardening inspection.
- **Future direction:** binary vulnerability analysis and fuzzing research — a long-term goal, not a v1.0 commitment.

> **Status: M0 — Foundations (started 3 November 2026).**
> The analyzer itself has **not been implemented yet**. The only working code so far is a small
> learning lab that checks ELF magic bytes (see [`labs/m0`](labs/m0)).
> Everything below marked *planned* describes intended behavior, not existing features.

## What it will do (v1.0, planned)

- Treat every input file as **untrusted** and never execute it.
- Parse **ELF64 little-endian** headers, program headers, section headers and the dynamic segment, with strict bounds and integer-overflow checks.
- Report three hardening properties — **NX**, **PIE** and **RELRO** — together with the evidence behind each finding.
- Report **`unknown`** when the file does not contain enough evidence, instead of guessing.

### Not in scope for v1.0

Exploitation, payloads, disassembly, decompilation, dynamic tracing, fuzzing, automatic vulnerability discovery, a GUI, and support for every ELF class / byte order / architecture.

**Design rule:** a small, correct, well-tested tool over a long feature list.

## Roadmap

| Milestone | Focus | Status |
|---|---|---|
| **M0 — Foundations** | Linux executables, bytes and hex, ELF basics, magic-number check | 🟡 In progress |
| **M1 — ELF header** | Safe file access, `e_ident` validation, ELF64 header parsing | ⬜ Planned |
| **M2 — Structures & dynamic segment** | Program headers, section headers, dynamic segment (`DT_FLAGS`, `DT_FLAGS_1`, `DT_BIND_NOW`) | ⬜ Planned |
| **M3 — Hardening analysis** | NX, PIE, RELRO with evidence and `unknown` states | ⬜ Planned |
| **M4 — Quality & release** | Tests, sanitizers, validation against `readelf`, documentation, tagged v1.0 | ⬜ Planned |

There is no fixed release date; coursework comes first.

## Repository layout

```text
labs/      Learning experiments, one folder per milestone (not part of the final tool)
  m0/      ELF magic-number check and hand-made test samples
src/       Analyzer source code (planned, from M1)
tests/     Automated tests (planned)
docs/      Design notes and decisions (planned)
```

## M0 lab: ELF magic-number check

`labs/m0/elf_magic.cpp` reads the first 4 bytes of a file and compares them with the ELF magic number `7F 45 4C 46` (`\x7FELF`).
A match only means the **first four bytes** are correct — it does **not** mean the file is a valid ELF.

### Build and run

```bash
cd labs/m0
g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined elf_magic.cpp -o elf_magic
./elf_magic hello.cpp    # replace hello.cpp with the path of any file to check
```

| Exit code | Meaning |
|---|---|
| `0` | Magic bytes match |
| `1` | Input checked: file too short or magic mismatch |
| `2` | Tool could not run: wrong arguments or file cannot be opened |

### Test samples and results

Verified on 3 Nov 2026 with the sanitizer build below (all six cases matched; no sanitizer errors).

| Input | Purpose | Output | Exit |
|---|---|---|---|
| `hello` (compiled from `hello.cpp`) | Real ELF executable | `ELF magic: match` | 0 |
| `hello.cpp` | Text file | `ELF magic: mismatch` | 1 |
| `empty.bin` (0 bytes) | Empty file | `ELF magic: too short (0 bytes)` | 1 |
| `short.bin` (`7F 45 4C`) | Truncated magic | `ELF magic: too short (3 bytes)` | 1 |
| `fake.bin` (`7F 45 4C 46`) | Correct magic, no ELF header | `ELF magic: match` | 0 |
| `mismatch.bin` (`7F 45 4C 58`) | Only the last byte differs | `ELF magic: mismatch` | 1 |

`fake.bin` is intentionally included to show the limit of a magic-number check; full header validation starts in M1.

### Reproducing the M0 tests

The `.bin` samples are committed, but they can be regenerated with `printf`.
`hello` is built from `hello.cpp` and is not committed.

```bash
cd labs/m0

# Regenerate the hand-made samples
: > empty.bin                    # 0 bytes
printf '\x7fEL'  > short.bin     # 3 bytes: truncated magic
printf '\x7fELF' > fake.bin      # 4 bytes: correct magic only
printf '\x7fELX' > mismatch.bin  # 4 bytes: last byte wrong

# Build a real ELF executable and the checker (with sanitizers)
g++ -std=c++17 hello.cpp -o hello
g++ -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined elf_magic.cpp -o elf_magic

# Run every case and print its exit code
for f in hello hello.cpp empty.bin short.bin fake.bin mismatch.bin; do
    echo "== $f"; ./elf_magic "$f"; echo "exit: $?"
done
```

## Design principles

1. **Separate parsing from presentation** — parsers return data or explicit errors; they do not print.
2. **Validate before reading** — check file size, offsets, entry sizes, counts and integer overflow first.
3. **Be honest about uncertainty** — insufficient evidence produces `unknown`, not yes/no.
4. **Keep modules small** — no speculative abstractions before a working prototype.
5. **Never execute analyzed files** — inspection is static and read-only.

## Development environment

Developed on Ubuntu under WSL2 (Windows), with:

- g++ 15.2.0, clang 21.1.8
- CMake 4.2.3 (used from M1)
- GNU Binutils 2.46 (`readelf`, used as a cross-checking reference)
- AddressSanitizer and UndefinedBehaviorSanitizer enabled in test builds

## Responsible use

This project is for learning and for analyzing binaries you built yourself or are authorized to examine.
It is not a substitute for a professional security audit.

## About this project

This is my second substantial C++ project, after a hospital triage and queue-management system.
I am a cybersecurity undergraduate starting binary security from zero, so this repository also records the learning process.

## Development and AI Assistance

This project is developed and maintained by the repository owner, with AI tools
(ChatGPT and Claude) used as part of the engineering workflow.

**How AI is used**

- Learning: explanations of ELF internals, C++ and Linux tooling.
- Implementation: AI-generated code examples and implementation suggestions have been used, particularly during the early learning stages.
- Review: two assistants review plans and code independently and are asked to challenge each other's proposals.
- Documentation: drafting and editing.

**How results are verified**

- Technical claims are not accepted from AI output alone. The project's verification sources are the ELF gABI, GNU `readelf` and local tests; in M0, header values read by hand were cross-checked with `readelf`.
- The M0 test cases were executed locally by the author using a sanitizer-enabled build.
- The owner makes the final design decisions and is responsible for every change.

## References

- [ELF Object File Format (generic ABI)](https://gabi.xinuos.com/)
- [GNU Binutils: `readelf`](https://sourceware.org/binutils/docs/binutils/readelf.html)
- [Clang AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
- [Clang UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)
