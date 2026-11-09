# Blue Steel EQ v2.0.6

## Windows build correction

- Generate JUCE's configuration header for both the production plugin and
  the DSP test app so the plugin sources compile in a clean Windows build.
- Rename the custom UI color palette to avoid a symbol collision with JUCE.
- Use supported JUCE color blending and keep component bounds mutable while
  splitting layout regions.
- Mark the atomically published response-curve cache as mutable for its
  thread-safe const refresh operation.
- Use JUCE's supported line drawing overloads and make state snapshots
  non-const to match the host-state API.

## Brand and design

- Branded for **LOOPS.CO.KE**, the royalty-free sample catalog and music
  production platform at [loops.co.ke](https://loops.co.ke).
- Refined the existing steel-blue design rather than replacing it.
- Upgraded prominent headings and control titles to the Windows
  **Bahnschrift** instrument-style typeface.
- Redesigned the knob pointer with a recessed dark groove, dimensional
  inner engraving and fine, directionally lit highlights.
- Added carefully layered metallic panels, sharper scale markings and
  clearer panel accents.

## Sound and presets

All 22 FL Studio-automatable parameters remain connected to their DSP. v2
ships 13 factory presets, selectable from both FL Studio's preset menu and
the plugin:

1. Initial Preset
2. Warm Low End
3. Vocal Focus
4. Air and Clarity
5. Smooth Mix Bus
6. Saturated Presence
7. Galaxy Tape
8. SSL Bus EQ
9. SSL Vocal Lift
10. SSL Drum Punch
11. SSL Mix Bus
12. Afro House Master
13. Tape Vocal Warmth

These are original, editable tonal starting points, not samples, official
hardware emulations or endorsements from SSL or any other third party.
The host's preset menu correctly recalls and displays all 13 programs.
The SSL-style presets apply original EQ curves; this is not a compressor,
tape emulation or official SSL product.

## Workflow and installation

- Added one-click window sizes from 70% to 200% in addition to window
  resizing. The selected size is saved in project and host state.
- Added an **About LOOPS.CO.KE** menu with company information, project and
  licence details, and one-click links to the company website and manual.
- Added a checkbox on the installer's completion page to open
  [loops.co.ke](https://loops.co.ke).
- The x64 VST3 installer places the plugin in the shared Windows VST3
  directory and includes the standalone Windows application.
- Kept a 100% dry bypass, VST host state and presets, input/output meters,
  live response plot and support for 44.1 kHz through modern hi-resolution
  sample rates.

## Real-time performance and validation

Filter coefficients and the response curve are updated only when an EQ
setting or sample rate changes. Meter levels use atomic peak-hold values;
the interface does not run spectrum analysis or allocate audio buffers
during playback.

`tests/dsp_tests.cpp` verifies flat frequency response, EQ bands, cut
filters, gain staging, bypass, the clipping ceiling, all 13 factory presets,
and saving/restoring parameter and window-scale state.

The Windows CI workflow builds the 64-bit Release VST3, standalone
application, DSP tests and compressed installer. The DSP tests cover
measured-frequency response, gain, clipping, presets, and host-state
restoration.
