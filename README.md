# Neon Dust Lasso

**Today’s Instrument (2026-11-14):** Neon Dust Lasso – An experimental cyber-western FM synthesizer that lassos dusty neon harmonics across the frontier with glitchy modulation character and raw twang.

A date-seeded monophonic 2-operator FM instrument with low-pass filter, ADSR envelope, and safety limiter. Designed for dark cinematic, cyber-western atmospheres blending neon glitch and dusty trail textures.

## Features
- 2-operator FM (carrier + modulator ratio & index)
- State-variable low-pass filter with resonance
- Full ADSR envelope
- Parameter smoothing for zipper-free automation
- MIDI note handling
- Full state save/load via APVTS
- Safety limiter to prevent hard clipping
- Dark cinematic UI with copper/neon amber rotary controls
- VST3 + Standalone formats

## Build Instructions

Requirements: CMake 3.22+, C++20 compiler, Git.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE 8.0.10 is fetched automatically via FetchContent on first configure.

### Artefacts
- VST3: `build/NeonDustLasso_artefacts/Release/VST3/Neon Dust Lasso.vst3`
- Standalone: `build/NeonDustLasso_artefacts/Release/Standalone/Neon Dust Lasso` (or .exe on Windows)

## GitHub Actions
The included workflow builds Release VST3 + Standalone on macOS and Windows, packages them as zips, and attaches them to a GitHub Release.

## License
GPL-3.0 (JUCE dual-license compatible). Company: MPJRecords.
