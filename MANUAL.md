# Wento Language Reference Manual

**Version 1.0.0 — Created by Fructose**

## 1. Lexical Structure

- Wento scripts are UTF-8 plain text files, conventionally using the `.wento` extension.
- Each statement occupies a single line. Leading and trailing whitespace is ignored.
- Blank lines and lines beginning with `#` are treated as comments and skipped.
- Keywords are case-insensitive (`set`, `play`, `wait`), while note instrument names and pitch identifiers follow standard casing.

## 2. Tempo Control

```wento
set tempo = <bpm>
```

Sets the global beat clock in Beats Per Minute (BPM). Valid range: `20` to `400` BPM.
The default tempo when a session or script starts is `120` BPM (one quarter beat = `500ms`).

Examples:
```wento
set tempo = 90
set tempo = 128.5
```

## 3. Note Synthesis

```wento
play note <note_name> <pitch> <duration>
```

Synthesizes and plays a musical tone using the named instrument timbre (`<note_name>`), scientific pitch notation (`<pitch>`), and time value (`<duration>`).

### Built-in Note Instruments
- `Piano` — Warm triangle-sine blend with gentle decay
- `Sine` — Pure fundamental sinusoidal wave
- `Saw` — Rich sawtooth harmonic lead
- `Square` — Hollow retro square wave
- `Triangle` — Soft flute-like odd-harmonic tone
- `SubBass` — Deep low-passed fundamental bass
- `CrystalBell` — Shimmering FM-inspired harmonic bell

Any custom note uploaded via `wento upload-note <file.wnote>` is also resolved automatically by `<note_name>` across all Wento installations connected to the registry.

### Pitch Notation
Scientific pitch notation `[A-G][#|b]?[0-8]` (e.g., `C4`, `F#3`, `Bb5`, `A4 = 440 Hz`), or direct frequency in Hertz (`440Hz`, `523.25Hz`). Use `REST` or `-` for a silent beat.

### Duration Formats
- **Musical fractions** (relative to current tempo, where `1/4` = 1 beat): `1/1`, `1/2`, `1/4`, `1/8`, `1/16`, `1/32`
- **Beat counts**: `1b`, `2b`, `0.5b`
- **Milliseconds**: `250ms`, `500ms`
- **Seconds**: `1s`, `1.5s`

## 4. HTTPS Audio Playback

```wento
play audio <https_url>
```

Downloads or streams an external audio resource over `https://` and plays it through the system audio device. Only `https://` URLs are accepted for transport security.

## 5. Timing & Rests

```wento
wait <duration>
```

Pauses script execution for the specified duration using the same duration formats as `play note` (`500ms`, `1s`, `1/4`, `2b`).

## 6. Custom Note Definition Format (`.wnote`)

To create and publish a custom instrument note for all Wento users:

```ini
name=GlassHarp
author=Fructose
waveform=sine
attack_ms=18
decay_ms=220
sustain=0.35
release_ms=340
harmonics=1.0,0.45,0.2,0.08
description=Airy glass harmonic resonator
```

Publish it globally with:
```sh
wento upload-note examples/crystal_bell.wnote
```

## 7. Terminal CLI Reference

| Command | Description |
| :--- | :--- |
| `wento readme` | Print README overview and build summary |
| `wento license` | Print the MIT license |
| `wento manual` | Print this language reference manual |
| `wento agents` | Print `AGENTS.md` architecture and contributor rules |
| `wento run <file>` | Parse and execute a `.wento` file |
| `wento command <stmt>` | Parse and execute a single inline Wento statement |
| `wento upload-note <file>` | Validate and upload a `.wnote` definition to the shared registry |
