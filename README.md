# TitanTune Metal V3 — experimental pitch-engine update

This source project replaces the prototype dual-delay-head pitch shifter with the open-source Signalsmith Stretch pitch/time engine. The goal is to reduce the repeated/chorusy artifacts heard on low guitar tunings. This is an experimental engineering build and must be auditioned in REAPER before it is considered production-ready.

## Build

The existing GitHub Actions workflow builds the Windows x64 VST3. GitHub Actions downloads the Signalsmith Stretch header and its Signalsmith Linear headers before configuring CMake. For a local Windows build, run `BUILD-WINDOWS.bat` with Git, CMake, and Visual Studio 2022 C++ tools installed.

1. Upload/replace the project files in the GitHub repository (including the updated `CMakeLists.txt` and both `Source/PluginProcessor.*` files).
2. Open **Actions** and wait for `Build TitanTune Metal VST3` to succeed.
3. Download the `TitanTune-Metal-Windows-VST3` artifact and inspect it before installing.

## Important audio notes

- The new engine has algorithmic latency; the plugin reports the latency reported by the engine.
- Test at unity, -5, -12, and -24 semitones using the same DI guitar performance.
- Compare transients, palm mutes, sustained low notes, chords, CPU load, and latency.
- No claim is made that all artifacts are eliminated; results depend on source material and require listening tests.

## Third-party dependency

Signalsmith Stretch is an open-source C++ pitch/time library under the MIT License:
https://github.com/Signalsmith-Audio/signalsmith-stretch

Its license and copyright notice are available in the upstream repository. JUCE is fetched according to the project's existing CMake configuration.
