# Complete parameter reference

Units are named in each option. Booleans require 0 or 1.
See [baseline assumptions](ray_baseline.md) before interpreting results.

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
| `--shell-thickness-mm` | `` | `10` | Scintillator shell thickness [mm]; outer radius remains 34.5 mm |
| `--guide-shape` | `` | `pentagon-hexagon` | pentagon-hexagon (supplied) or pentagon-octagon (beveled variant) |
| `--guide-bevel-fraction` | `` | `0.25` | Octagon corner cut as fraction of adjacent hexagon edges (0 < f < 0.5) |
| `--sipm-pitch-mm` | `` | `4.4` | Sensor center pitch in each 4x4 array [mm] |
| `--sipm-model` | `` | `HamamatsuS1036233100` | Built-in G4SiPM model name; package must fit array pitch |
| `--scintillator` | `` | `GAGG` | Scintillator material: GAGG (baseline) or LYSO |
| `--gagg-yield` | `` | `57` | Provisional GAGG scintillation photons per keV |
| `--gagg-decay-ns` | `` | `88` | Provisional single-component GAGG decay [ns] |
| `--gagg-index` | `` | `1.9` | Constant GAGG refractive index |
| `--gagg-absorption-mm` | `` | `1000` | Assumed GAGG absorption length [mm], unmeasured |
| `--lyso-yield` | `` | `32` | Scintillation photons per keV |
| `--lyso-decay-ns` | `` | `41` | Scintillation decay time [ns] |
| `--lyso-index` | `` | `1.81` | Constant LYSO refractive index |
| `--lyso-absorption-mm` | `` | `413` | Constant LYSO optical absorption length [mm] |
| `--scint-energy-ev` | `` | `auto` | Flat spectrum center [eV]; auto: GAGG 520 nm, LYSO 2.8 eV |
| `--scint-width-ev` | `` | `0.02` | Full width of provisional flat spectrum [eV] |
| `--spectrum` | `` | `none` | Optional CSV: energy_eV,intensity; overrides flat spectrum |
| `--epoxy-index` | `` | `1.5` | Reference sensor-window and outer-mask epoxy index (not guide/coupling) |
| `--epoxy-absorption-mm` | `` | `0` | Reference window/mask absorption [mm]; 0 disables bulk absorption |
| `--guide-index` | `` | `1.5` | Provisional guide resin refractive index |
| `--guide-absorption-mm` | `` | `1000` | Assumed guide absorption [mm]; 0 disables |
| `--coupling-index` | `` | `1.43` | Provisional Encapso-surrogate refractive index |
| `--coupling-absorption-mm` | `` | `1000` | Assumed coupling absorption [mm]; 0 disables |
| `--coupling-thickness-mm` | `` | `0.05` | Radial coupling layer [mm], taken from guide volume (0 < t <= 0.5) |
| `--inner-surface` | `` | `bare` | Ground inner boundary: bare, reflector, or absorber |
| `--inner-sigma-alpha-rad` | `` | `0.1` | Assumed ground-surface microfacet roughness [rad] |
| `--inner-reflectivity` | `` | `0.98` | Reflector-mode inner reflectivity [0..1] |
| `--guide-boundary` | `` | `absorber` | Exposed guide/mask boundary: bare, reflector, or absorber |
| `--guide-reflectivity` | `` | `0.98` | Reflector-mode guide reflectivity [0..1] |
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
| `--batch` | `-b` | `1` | 1 batch; 0 opens native geometry, tracks and SiPM hits (UIVIS build) |
| `--vis-photons` | `` | `50` | First N optical paths/event in native display; 0 hides optical paths |
| `--verbose` | `-v` | `0` | Application and SiPM verbosity [0..2] |

Additional switches: `--help` / `-h`, `--version`, `--config FILE` / `-c FILE`, `--print-config`.

`gagg-*` options affect GAGG only; `lyso-*` options affect LYSO only.
A custom `--spectrum` overrides `scint-energy-ev` and the provisional flat spectrum.
Explicit CLI options override all configuration files.
