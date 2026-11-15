# Wento

**Wento** is a terminal-first music programming language and synthesis engine created by **Fructose**.
Designed for minimal composition, live terminal playback, and community-shared instrument definitions, Wento turns plain-text `.wento` scripts into synthesized audio and HTTPS media playback from any Unix/Linux, macOS, or Windows terminal.

## Author & Attribution

- **Language Name**: Wento
- **Creator**: Fructose
- **Implementation**: C++17 (Zero external library dependencies required for core synthesis)
- **License**: MIT License (see `LICENSE`)

## Core Language Commands

Inside any `.wento` file or via `wento command`:

```wento
# Set playback tempo in beats per minute
set tempo = 120

# Play a synthesized note: play note <note_name> <pitch> <duration>
play note Piano C4 1/4
play note CrystalBell E4 500ms
play note SubBass A2 1s

# Stream or play an external HTTPS audio file
play audio https://example.com/samples/kick.wav

# Pause execution before the next instruction
wait 250ms
wait 1/2
```

## CLI Usage

```text
wento readme                     Display project overview and quickstart guide
wento license                    Display the MIT License text
wento manual                     Display the full Wento language reference manual
wento agents                     Display AGENTS.md contributor & agent guide
wento run <file.wento>           Execute a Wento music script file
wento command "<statement>"      Execute a single inline Wento statement
wento upload-note <file.wnote>   Publish a custom note instrument for all Wento users
```

## Building from Source

### Unix / Linux / macOS

Run the `configure` script to detect your C++17 compiler and system audio backend and generate a tailored `Makefile`:

```sh
chmod +x configure
./configure
make
sudo make install
```

Optional flags for `./configure`:
- `--prefix=/usr/local` — Set installation prefix (default `/usr/local`)
- `--cxx=clang++` — Override the C++17 compiler

### Windows (MSVC or MinGW-w64)

Windows builds use the pre-configured `config.h` header (`-DWENTO_USE_WIN32_CONFIG`):

**MSVC Developer Command Prompt:**
```bat
cl /std:c++17 /O2 /EHsc /DWENTO_USE_WIN32_CONFIG /I. src\main.cpp src\interpreter.cpp src\synth.cpp src\registry.cpp /Fe:wento.exe winmm.lib winhttp.lib
```

**MinGW-w64 (g++):**
```bat
g++ -std=c++17 -O2 -DWENTO_USE_WIN32_CONFIG -I. src/main.cpp src/interpreter.cpp src/synth.cpp src/registry.cpp -o wento.exe -lwinmm -lwinhttp
```

## Quick Example

Run the included sample score:

```sh
./wento run examples/melody.wento
```
