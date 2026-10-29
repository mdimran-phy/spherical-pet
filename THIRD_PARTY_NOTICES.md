# Third-party notices and provenance

## Detector construction

`src/DetectorConstruction.cc` and `include/DetectorConstruction.hh` derive from the supplied detector files, whose headers identify Tim Niggemann, III Phys. Inst. A, RWTH Aachen University, and GNU GPL v3. The files also contain later spherical-detector modifications with their original comments. Original attribution is preserved; no authorship of those geometric developments is claimed here. Original input hashes are in `docs/original_provenance.json`.

Integration changes: Geant4 11 scintillation properties; ascending provisional spectrum or configurable CSV spectrum; reference-model selection; housing entrance-plane alignment; validated CLI-configurable shell thickness, sensor pitch and optical properties; optional suppression of diagnostic prints. A configuration reference is passed into the original geometry rather than replacing its topology.

## G4SiPM

Upstream: https://github.com/c-dilks/g4sipm , branch `geant11-fixes`, commit `53d953ee2427309cfa4d6b94d60f09a28ba1fa20`.

The required `include/`, `src/`, and `resources/` are included under `third_party/g4sipm/`. License: GNU GPL v3, preserved at `third_party/g4sipm/LICENSE`. Original source copyright headers are retained. Integration changes add a water material accessor needed by unused legacy construction code, correct the nominal Bisphenol A elemental formula, and gate per-sensor event logging on verbosity. The reference sensor model and PDE implementation remain upstream code.

## Jansson

Upstream: https://github.com/akheron/jansson , commit `bc5741fb1ac730ead24e9bd08977fc6c248e04b0`.

Included under `third_party/jansson/` as the dependency pinned by the G4SiPM checkout. License: MIT, preserved at `third_party/jansson/LICENSE`. No Git submodule initialization is required. Application CMake disables its standalone tests and documentation build; its source and notices are preserved.

## External installations

Geant4, Boost, NumPy, Matplotlib and Pillow are not redistributed by this repository and retain their own licenses. The PNG in `docs/images/` was rendered from the exported simulation geometry; the user's reference photograph is not included.
