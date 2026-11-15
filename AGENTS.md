# AGENTS.md — Wento Architecture & Contributor Guide

This document defines the engineering conventions, module boundaries, and runtime guarantees for **Wento**, created by **Fructose**.

## 1. Repository Layout

```text
wento-cli/
├── configure          POSIX shell script generating Unix/Linux Makefile & config.unix.h
├── config.h           Windows (MSVC / MinGW) static configuration header
├── README.md          Project overview and quickstart
├── LICENSE            MIT License (Copyright Fructose)
├── MANUAL.md          Complete Wento syntax and CLI manual
├── AGENTS.md          This contributor & AI agent specification
├── examples/          Reference .wento scores and .wnote instrument definitions
└── src/
    ├── wento.hpp      Shared constants, types, and platform config inclusion
    ├── main.cpp       Terminal CLI router (readme, license, manual, agents, run, command, upload-note)
    ├── interpreter.hpp / interpreter.cpp   Lexer, parser, duration math, execution loop
    ├── synth.hpp / synth.cpp               16-bit PCM WAV synthesizer & OS audio backends
    └── registry.hpp / registry.cpp         Local + HTTPS community note registry client
```

## 2. Core Design Rules

1. **English-Only Internals**: All source identifiers, comments, error diagnostics, manual entries, and sample scripts must be written strictly in English.
2. **Zero Mandatory Third-Party Dependencies**: Core compilation must succeed with a standard C++17 compiler (`g++`, `clang++`, or `cl.exe`) and OS-provided libraries (`winmm`/`winhttp` on Windows, standard POSIX + `curl`/`afplay`/`aplay`/`paplay` on Unix).
3. **Platform Configuration Split**:
   - **Unix / Linux / macOS**: `./configure` probes the host environment and emits `Makefile` and `config.unix.h`.
   - **Windows**: `config.h` provides Win32 feature flags (`WENTO_OS_WINDOWS`), `WIN32_LEAN_AND_MEAN`, and Winsock/WinHTTP/WinMM linkage directives.
4. **Deterministic Timing**: Tempo changes (`set tempo = <bpm>`) immediately recalculate beat-fraction durations (`1/4`, `1/8`, `2b`) for all subsequent `play note` and `wait` statements.
5. **Community Note Registry Protocol**:
   - `wento upload-note <file.wnote>` validates required fields (`name`, `waveform`, `attack_ms`, `decay_ms`, `sustain`, `release_ms`, `harmonics`), persists a local copy in `~/.wento/notes/`, and dispatches an HTTPS/HTTP `POST /api/notes` request to `WENTO_REGISTRY_URL` so every Wento user can invoke `play note <name> ...`.

## 3. Adding a New Statement

1. Extend `StatementType` in `src/interpreter.hpp`.
2. Implement parsing and validation in `Interpreter::parseLine()` (`src/interpreter.cpp`).
3. Add execution handling in `Interpreter::executeStatement()`.
4. Document the new syntax in `MANUAL.md` and `README.md`.
5. Add a working snippet under `examples/`.
