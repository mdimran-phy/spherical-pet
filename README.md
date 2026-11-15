# spherical-pet

A standalone Geant4 simulation of a spherical scintillation detector with optical-photon transport and SiPM readout. It provides command-line runs, source macros, configuration files, CSV output, and native and offline event displays.

The default detector is a full GAGG shell with a 24.5 mm inner radius, a 34.5 mm outer radius, a faceted light guide, and 512 SiPMs arranged in 32 arrays of 4 × 4. The default source emits one 511 keV gamma per event from the center. Optical material parameters are configurable and provisional. Coincidence selection and PET image reconstruction are not implemented.

## Requirements

- Geant4 11.1 or newer with its physics datasets; tested with 11.1.2.
- A C++17 compiler and CMake 3.16 or newer.
- Boost development libraries: `date_time`, `program_options`, `filesystem`, `system`, and `regex`.
- Python 3.9 or newer for analysis scripts, with dependencies from `requirements.txt`.
- For the native event display: Geant4 built with visualization/UI support and a working graphical display.

G4SiPM and Jansson sources are included in `third_party/`. ROOT, Git submodules, and private libraries are not required. Geant4, Boost, and Python packages must be installed separately.

## Download and build

```bash
git clone https://github.com/mdimran-phy/spherical-pet.git
cd spherical-pet

# Load your Geant4 environment if needed:
# source /path/to/geant4/bin/geant4.sh

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./build/spherical-pet --help
```

If CMake cannot locate Geant4, supply `-DGeant4_DIR=/path/to/directory/containing/Geant4Config.cmake`. Use the same Geant4 installation and datasets when building and running.

Run the commands below from the repository root. Each run must use a **new output directory**; existing results are never overwritten.

## Quick start

Simulate ten 511 keV gammas from the center, directed along +z:

```bash
./build/spherical-pet --macro macros/gamma_center.mac \
    -e 10 -seed 42 -o results/gamma_center
```

Alternatively, specify the source directly on the command line:

```bash
./build/spherical-pet -x gamma -E 511 \
    -gx 0 -gy 0 -gz 0 --angular fixed -theta 0 -phi 0 \
    -e 10 -seed 42 -o results/gamma_cli
```

`./run.sh` is a convenience wrapper for `./build/spherical-pet` and accepts the same arguments.

## Source macros

| Macro | Source |
|---|---|
| [gamma_center.mac](macros/gamma_center.mac) | One 511 keV gamma at (0, 0, 0) mm, directed along +z |
| [isotropic_source.mac](macros/isotropic_source.mac) | One 511 keV gamma at (0, 0, 0) mm, with an isotropic direction |
| [gamma_offset.mac](macros/gamma_offset.mac) | One 511 keV gamma at (10, 0, 0) mm, directed along +z |

```bash
# Isotropic source
./run.sh --macro macros/isotropic_source.mac \
    -e 100 -seed 123 -o results/gamma_isotropic

# Off-center source
./run.sh --macro macros/gamma_offset.mac \
    -e 10 -seed 42 -o results/gamma_offset
```

Macros contain **only `/gps/*` source commands** and are applied after command-line source settings. The supplied macros explicitly select the particle, energy, position, direction distribution, and one primary per event. Edit or copy a macro to change these settings.

Set the event count, seed, output directory, and visualization options on the command line. Do not put `/run/beamOn`, `/run/initialize`, random-seed commands, or `/vis/*` commands in a source macro. The application initializes and starts one run per invocation. Applied source commands are saved in `source_commands.mac`.

## Configuration and common options

Use the commented default configuration and override selected values:

```bash
./build/spherical-pet --config config/default.conf \
    --macro macros/gamma_center.mac -e 10 -seed 42 \
    -o results/configured_run
```

| Option | Meaning |
|---|---|
| `-e`, `--events` | Number of events |
| `-o`, `--output` | New output directory |
| `-seed`, `--seed` | Random seed |
| `-x`, `--particle` | `gamma`, `e-`, or `e+` |
| `-E`, `--energy-kev` | Primary kinetic energy in keV |
| `-gx`, `-gy`, `-gz` | Source position in mm |
| `-theta`, `-phi` | Source direction angles in degrees |
| `--angular` | `fixed` or `isotropic` |
| `--macro` | Source macro file |
| `--scintillator` | `GAGG` or `LYSO` |
| `--shell-thickness-mm` | Crystal thickness; outer radius remains 34.5 mm |
| `--guide-shape` | `pentagon-hexagon` or the beveled `pentagon-octagon` variant |
| `--check-overlaps 1` | Audit geometry before transport |
| `-r 1` | Inspect geometry without transporting events |
| `-b 0` | Open the native event display |
| `--vis-photons` | Optical trajectories drawn per event |
| `--track-photons` | Optical trajectories saved to CSV per event |

Configuration files use `key = value`. Explicit command-line options override configuration values. Input paths inside a configuration file are relative to that file; output paths are relative to the working directory. A source macro is applied last and can override source settings.

```bash
# Inspect settings without running transport
./build/spherical-pet --config config/default.conf --print-config

# Check geometry only
./build/spherical-pet -r 1 -o results/geometry

# Save paths for the first 20 optical tracks in each event
./build/spherical-pet --macro macros/gamma_center.mac \
    -e 1 --track-photons 20 -o results/optical_tracks
```

Use `--help` for all supported parameters and units. `--print-config` prints configuration settings; macro commands are applied when the simulation initializes.

## Native event display

Build with visualization enabled:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DWITH_GEANT4_UIVIS=ON
cmake --build build -j4

# On Linux with a Qt-enabled Geant4 build, load the generated environment:
# source build/native-env.sh

./build/spherical-pet --macro macros/gamma_center.mac \
    -b 0 -e 1 -seed 42 --vis-photons 5 -o results/native_event
```

On Linux, source `build/native-env.sh` when generated to use the Qt libraries selected by CMake. Load it after your Geant4 environment. Remote sessions require X forwarding and a local X server, for example `ssh -Y user@host`.

The display uses red for gamma tracks, blue for electrons, magenta for positrons, cyan for optical photons, and green markers for accepted SiPM hits. Use **PET → Review events** or enter `/vis/reviewKeptEvents` in the Geant4 session to inspect retained events. Enter `continue` to advance and `exit` to close the session.

`--vis-photons 5` draws the first five optical trajectories per event. It does not reduce photon generation, transport, or hit processing. Use `--vis-photons 0` to hide optical paths or increase the number for a denser view. `--track-photons` independently controls CSV trajectory recording; both limits select diagnostic samples.

A single gamma can escape without producing optical hits. Increase `-e` to inspect additional events.

To view geometry without an event:

```bash
./build/spherical-pet -r 1 -b 0 -o results/native_geometry
```

## Offline plots and displays

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt

python scripts/plot_results.py --input results/gamma_center
python scripts/event_display.py --input results/gamma_center --event 0
python scripts/photo_geometry_display.py --input results/gamma_center
```

The scripts save figures under `results/gamma_center/figures/`:

- `event_000.png` / `.pdf`: gamma paths, sensor occupancy, and arrival times.
- `event_display.html`: interactive event selection and timing filters.
- `photo_style_detector.png` / `.pdf`: detector cutaway.
- `photo_style_event_display.html`: shaded detector and event viewer.

Open HTML files locally in a browser. No web server or network connection is required; the shaded viewer requires WebGL. These offline displays show gamma trajectories and SiPM occupancy. Sampled optical paths are available separately in CSV.

## Simulation output

| File | Contents |
|---|---|
| `events.csv` | Crystal energy deposit, optical production, and accepted hits per event |
| `sipm_hits.csv` | One row per accepted optical photon, with sensor ID, track ID, time, energy, and position |
| `sipm_counts.csv` | Counts for all 512 sensors per event, including zero-hit sensors |
| `gamma_interactions.csv` | Gamma interaction processes, positions, times, and energies in the crystal |
| `gamma_steps.csv` | Gamma trajectory segments |
| `optical_steps.csv` | Sampled optical trajectories when requested |
| `sensor_positions.csv` | Sensor centers |
| `summary.json` | Run totals and optical termination counts |
| `resolved.conf` | Resolved configuration |
| `source_commands.mac` | Applied source commands |
| `spectrum.csv` | Scintillation spectrum used |
| `build.txt` | Application, Geant4, and compiler versions |
| `geometry.obj`, `geometry_inventory.csv` | Geometry exports when enabled |
| `geometry_check.json` | Sampled overlap-audit results when requested |

Energy deposition is reported as `edep_scintillator_keV`. Accepted optical hits are distinct from primary events, illuminated sensors, and drawn trajectories.

## Test and install

```bash
ctest --test-dir build --output-on-failure
cmake --install build --prefix "$HOME/.local"
```

Tests cover configuration, geometry, command-line handling, deterministic transport, photon accounting, and native visualization when the required display tools are available. The installed executable is placed in `bin/`, with supporting files under `share/spherical-pet/`.

## License

The application and G4SiPM-derived code use GPL-3.0. Jansson retains its MIT license. See [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
