FELTROOM REVERB - JUCE VST3 SOURCE

This is a NEW effect plugin, not a patch to the uploaded compiled binary.
Requires CMake, Git, Visual Studio 2022 C++ and internet for JUCE FetchContent.
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target FeltRoom_VST3

IMPORTANT: This is a starter DSP with JUCE Reverb. PREDELAY, LOWCUT, HIGHCUT currently appear in UI but do not yet process audio. DECAY is only approximate. Do not distribute as a complete reverb. Not compiled/tested here.
