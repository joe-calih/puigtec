# Blue Steel EQ

A 64-bit Windows VST3 equalizer and standalone application, with a custom
blue-steel interface, Bahnschrift instrument lettering, musical filters,
analog-style soft clipping and 13 editable factory presets. The original
plugin is branded for [LOOPS.CO.KE](https://loops.co.ke), a curated,
royalty-free sample catalog and music-production platform. EQ presets and
hardware-inspired controls are original creative starting points; this
product is not affiliated with or endorsed by SSL or any third-party
manufacturer.

## Download and install

Open [Releases](https://github.com/joe-calih/puigtec/releases/latest) and
download the Windows installer, portable **VST3** bundle or self-contained
standalone package. The installer places the VST3 plugin in its shared
Windows directory and includes the standalone application. In FL Studio,
open **Options > Manage plugins**, choose **Find plugins**, and load **Blue
Steel EQ v2** from the Effects list. The installer offers to open
loops.co.ke when installation completes.

The GitHub Actions workflow builds the installer automatically when a
version tag (for example `v2.0.4`) is pushed and publishes the verified
Windows installer as a downloadable Release asset.

## v2 presets and controls

- **Galaxy Tape** and **Tape Vocal Warmth** for character and tape-style
  soft clipping.
- **SSL Bus EQ**, **SSL Mix Bus**, **SSL Drum Punch**, and **SSL Vocal
  Lift** for original mix, drum, and vocal starting points.
- **Afro House Master** for a polished Afro-house starting point.
- Six versatile original presets: Initial Preset, Warm Low End, Vocal Focus,
  Air and Clarity, Smooth Mix Bus and Saturated Presence.
- Choose any control-panel size from 70% to 200%. The setting is saved
  with the FL Studio project.
- **About** menu with LOOPS.CO.KE company details, complete license
  information and direct links to the website and manual.

See the [version 2 release notes](./docs/V2-RELEASE-NOTES.md) for all changes.
The SSL-style presets apply original EQ curves; this is not a compressor,
tape emulation or official SSL product.

## Build on Windows

Install Visual Studio 2022 Build Tools with the **Desktop development with
C++** workload and CMake 3.22 or later. Run the provided Windows build
script to configure the 64-bit build, compile the plugin, standalone
application and DSP tests, run the tests, and package the installer:

```powershell
.\build_windows.ps1
```

The first CMake configure downloads the pinned JUCE 8.0.6 source dependency.
Inno Setup 6 is required to create the compressed Windows installer.
The portable VST3 and standalone packages are built automatically without
the installer compiler.

## FL Studio and other DAWs

The Windows installer registers a 64-bit **VST3 effect** in the shared
`C:\Program Files\Common Files\VST3` directory. FL Studio users can open
**Options > Manage plugins > Find plugins**, then load **Blue Steel EQ v2**
from the effect list. The same standard VST3 format works in other
64-bit VST3-compatible DAWs. Use the included standalone application if
you want to audition presets without opening a DAW.

The plugin supports host automation and project-state recall, the host
preset/program list, stereo and mono buses, and 44.1 kHz through modern
hi-resolution sample rates. Use FL Studio's plugin wrapper to fit or
resize the panel; in the plugin, select the desired saved 70–200% size.
Scanning paths and whether to show the host wrapper are DAW settings.

## Controls

- Independent input and output gain, 100% dry bypass, and switchable soft
  clipping with an adjustable clipping ceiling.
- A low shelf with selectable corner frequency, boost, and separate
  attenuation frequency and amount.
- Independently tunable mid boost and dip, with continuously adjustable
  bandwidth for each band.
- A tunable high-frequency peak with bandwidth and a separate high-frequency
  attenuation shelf.
- Switchable high-pass and low-pass filters, plus a live frequency-response
  plot and input/output level meters.
- Thirteen factory presets; host automation, program recall and saved plugin state are supported.

All frequency selections, gains, bandwidths, filters, meters and the clipper
are connected to the same parameters that the plugin processes.

## License

This project and its source code are licensed under GNU AGPL-3.0-only; the
complete text is in [LICENSES/AGPL-3.0.txt](./LICENSES/AGPL-3.0.txt).
JUCE is used under AGPLv3; its upstream license and third-party notices
are included with the Windows downloads and installer. JUCE source is
fetched at the pinned 8.0.6 release tag rather than vendored in this repo.
