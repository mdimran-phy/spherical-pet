# Complete parameter reference

Values use the units named in the option. Boolean options require 0 or 1.

| Option | Alias | Default | Meaning |
|---|---|---|---|
| `--mode` | `-r` | `simulation` | simulation (0) or geometry (1); geometry performs overlap checks |
| `--events` | `-e` | `10` | Number of primary events |
| `--output` | `-o` | `results/run` | New output directory; existing paths are never overwritten |
| `--seed` | `-seed` | `20261029` | Positive random seed, at most 2147483646 |
| `--particle` | `-x` | `gamma` | Primary: gamma, e-, or e+ |
| `--energy-kev` | `-E` | `511` | Primary kinetic energy [keV], including for positrons |
| `--theta` | `-theta` | `0` | Polar direction from +z [degrees, 0..180] |
| `--phi` | `-phi` | `0` | Azimuth from +x toward +y [degrees] |
| `--angular` | `` | `fixed` | fixed or isotropic directions |
| `--source-x-mm` | `-gx` | `0` | Source x position [mm] |
| `--source-y-mm` | `-gy` | `0` | Source y position [mm] |
| `--source-z-mm` | `-gz` | `0` | Source z position [mm] |
| `--shell-thickness-mm` | `` | `10` | LYSO shell thickness [mm]; outer radius remains 34.5 mm |
| `--guide-shape` | | `pentagon-hexagon` | `pentagon-hexagon` (supplied) or `pentagon-octagon` (derived bevels); see [geometry](geometry.md) |
| `--guide-bevel-fraction` | | `0.25` | Fraction along adjacent edges cut at two opposite hexagon corners; strictly between 0 and 0.5; used only by octagon option |
| `--sipm-pitch-mm` | `` | `4.4` | Sensor center pitch in each 4x4 array [mm] |
| `--sipm-model` | `` | `HamamatsuS1036233100` | Built-in G4SiPM model name; package must fit array pitch |
| `--lyso-yield` | `` | `32` | Scintillation photons per keV |
| `--lyso-decay-ns` | `` | `41` | Scintillation decay time [ns] |
| `--lyso-index` | `` | `1.81` | Constant LYSO refractive index |
| `--lyso-absorption-mm` | `` | `413` | Constant LYSO optical absorption length [mm] |
| `--scint-energy-ev` | `` | `2.8` | Center of provisional flat scintillation spectrum [eV] |
| `--scint-width-ev` | `` | `0.02` | Full width of provisional flat spectrum [eV] |
| `--spectrum` | `` | `none` | Optional CSV: energy_eV,intensity; overrides flat spectrum |
| `--epoxy-index` | `` | `1.5` | Shared epoxy index: guide, gaskets and reference sensor window |
| `--epoxy-absorption-mm` | `` | `0` | Shared epoxy absorption [mm]; 0 leaves bulk absorption unset |
| `--pde-filter` | `` | `1` | Enable upstream SiPM photon-detection-efficiency filter [0/1] |
| `--cell-filter` | `` | `1` | Enable SiPM cell-geometry filter [0/1] |
| `--range-cut-mm` | `` | `0.1` | Electromagnetic production range cut [mm] |
| `--max-steps` | `` | `10000` | Per-track guard; killed tracks are reported and cause exit code 1 |
| `--track-photons` | `` | `0` | Save full steps for first N optical tracks/event (0 disables) |
| `--export-geometry` | `` | `1` | Export geometry OBJ, inventory and sensor centers [0/1] |
| `--mesh-steps` | `` | `96` | Angular tessellation steps for display only [12..192] |
| `--check-overlaps` | `` | `0` | Check placed volumes before simulation [0/1] |
| `--overlap-samples` | `` | `1000` | Sampled points per volume for overlap check |
| `--overlap-tolerance-mm` | `` | `0.001` | Overlap-check tolerance [mm] |
| `--macro` | `-m` | `none` | Optional source macro; /gps/* commands only |
| `--batch` | `-b` | `1` | 1 batch; 0 opens Geant4 UI (requires WITH_GEANT4_UIVIS=ON) |
| `--verbose` | `-v` | `0` | Application and SiPM verbosity [0..2] |

Additional switches: `--help` / `-h`, `--version`, `--config FILE` / `-c FILE`, and `--print-config`.

Geometry dimensions not listed here remain fixed in the supplied construction. See README.md and physics.md for model assumptions.
