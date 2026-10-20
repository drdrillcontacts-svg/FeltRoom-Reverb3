# FeltRoom Reverb 2.0 — GitHub Actions

1. Upload ALL files and folders from this ZIP into the root of your GitHub repository, replacing old files. Include `.github/workflows/build-windows.yml`.
2. Go to Actions > Build FeltRoom VST3 (Windows) > Run workflow.
3. Download the `FeltRoom-Windows-VST3` artifact after a successful build.

Changes: D.A Production, 16 factory presets, save/load XML presets, six algorithmic character settings, working pre-delay and wet high/low cut, improved numerical display, removed corrupt footer text.

Limitations: The six characters share JUCE's algorithmic reverb core with different tunings (not separate convolution engines). Decay is an approximate control, not a calibrated RT60. Build has not been verified on GitHub.
