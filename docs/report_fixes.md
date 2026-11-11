# Spherical-code report: implemented fixes

Updated November 11, 2026. This records the completed cleanup in the `spherical-pet` application. The original `Medical_imaging/DetectorConstruction.cc` and the earlier diagnostic copy are preserved.

## What changed and why

| Report issue | Completed action | Reason |
|---|---|---|
| Dead and commented-out code | Removed abandoned annulus, water-column, test-box, individual pyramid, older guide/casting, collar, and alternate SiPM-placement branches. | They did not contribute to the placed detector. Keeping them obscured the live geometry and allocated unused objects. |
| Orphaned solids and facets | Retained only solids reachable from placed volumes, including Boolean operands; removed unused facet-generation loops and unplaced logical volumes. | Avoids unnecessary initialization and allocations. A regression test now traverses the placed geometry and checks both the logical-volume and solid stores for unused entries. |
| Primitive exception | Replaced `throw 1` in `createHousing()` with a descriptive `std::runtime_error`. Unsupported model selection also uses a standard exception. | Failures reach the application's existing exception handler with an explanatory message. |
| Disabled destructor cleanup | Converted housings and the shared sensor model to `std::unique_ptr` ownership. The shared model outlives the housings. | Housings delete their individual SiPM objects; the shared model is deleted exactly once. Constructor failures also release completed allocations. |
| Rotation ownership | Own shared placement matrices separately; clear housing rotations with the upstream `setRotation(nullptr)` API during destruction. | The upstream setter deletes a previous matrix, while its destructor does not. Following that contract avoids leaks and competing owners. |
| Empty housing vector | Added an explicit empty-vector check before selecting the first housing, a null-model check, and a compile-time positive sensor-count check. | Produces a clear failure before accessing a missing sensor. The detector still contains 512 SiPMs. |
| Hardcoded dimensions | Added [GeometryParameters.hh](../include/GeometryParameters.hh) for the fixed shell radius, guide outlet, world size, gasket thickness, liner thickness, mask extension, and sensor topology. Configuration validation uses the same shell and world dimensions. | Keeps retained dimensions named and consistent. Shell thickness, pitch, optical properties, and bevel settings remain runtime configurable. |
| Obsolete class members | Replaced the large list of raw geometry members with local construction handles and a small ownership-focused class interface. | Geant4 stores own solids and volumes; the detector owns its sensor wrappers and model. This makes those responsibilities explicit. |

The detector source decreased from **5322 to 1370 lines**, including formatting. Original copyright attribution is retained.

## Report recommendations resolved by checking the actual geometry

### File tail

The complete supplied source was not truncated. The maintained source has complete functions and builds successfully. There was no dangling active placement statement to repair.

### North/south hemisphere and gasket placement

No arbitrary Z shifts were introduced. Each scintillator and gasket pair is partitioned by Boolean cuts; the north pair is rotated into the opposite hemisphere. A common origin does not imply common material volume.

The new geometry regression test probes points on both sides of the equatorial gasket boundaries and verifies that exactly the expected hemisphere/scintillator or gasket contains each point. Full sampled overlap audits also pass for both guide shapes.

### Light-guide placement

The unused experimental `hexagon_A_LV` branch was removed. The actual guide, `chapeauPoofMinusHomeOrb_PV`, remains placed, with its spherical socket and optional bevel intact. The regression test requires exactly one main guide placement. Enabling the obsolete experimental guide would add a different volume to the existing detector.

## Earlier fixes retained

- SiPM centers remain displaced outward by half the housing depth, keeping the front face at the 39.5 mm outlet plane.
- LYSO uses an increasing energy grid or the configured spectrum, with Geant4 11 scintillation property names.
- The unavailable custom SiPM model is rejected explicitly; the reference sensor remains selectable.
- Sensor pitch and outlet-fit checks remain active.
- Both pentagon–hexagon and optional pentagon–octagon guide modes remain available.

## Verification performed

The application was rebuilt with Geant4 11.1.2. **All six CTest tests passed**, including two new detector-geometry tests, one per guide mode.

The new tests verify that all constructed solids and logical volumes participate in the detector, the main guide is placed, all 512 sensitive sensors are distinct, gasket/scintillator interiors are separated at the tested seam points, and housing destruction deactivates sensors and detaches both digitizer types. They also exercise the housing library's rotation-replacement behavior before teardown. CLI tests now cover missing sensor models, too-small pitch, and arrays too wide for the outlet.

Both guide modes passed sampled overlap checks:

| Guide mode | Volumes checked | Samples per volume | Tolerance | Volumes reporting overlaps |
|---|---:|---:|---:|---:|
| Pentagon–hexagon | 2,055 | 1,000 | 0.001 mm | 0 |
| Pentagon–octagon | 2,055 | 1,000 | 0.001 mm | 0 |

Before/after comparisons used a 100 keV electron at z = 28 mm, one event, seed 1234, and a reduced yield of 2 photons/keV. For each guide mode, these eight files were **byte-for-byte identical** to the pre-cleanup executable's output: `events.csv`, `sipm_hits.csv`, `optical_steps.csv`, `sensor_positions.csv`, `geometry_inventory.csv`, `geometry.obj`, `guide_sections.csv`, and `summary.json`.

The detector and new C++ regression test also passed `-Wall -Wextra -Werror -fsyntax-only`. `git diff --check` passed.

Evidence: [verification data and hashes](validation/report_cleanup_20261111/verification.json), [CTest log](validation/report_cleanup_20261111/ctest.log), and [build log](validation/report_cleanup_20261111/build.log).

## Reproduce the checks

From the project root, with the installed Geant4 environment loaded:

```bash
cmake --build build -j4
ctest --test-dir build --output-on-failure
./build/spherical-pet --mode geometry --guide-shape pentagon-hexagon \
    --overlap-samples 1000 -o results/report_hexagon_check
./build/spherical-pet --mode geometry --guide-shape pentagon-octagon \
    --overlap-samples 1000 -o results/report_octagon_check
```

Each output directory must be new. Sampled audits and deterministic transport comparisons establish consistency for the tested configurations; they do not prove exact geometry for every possible parameter combination. The provisional emission spectrum, optical inputs, and reference sensor still require experimental validation before quantitative detector-performance claims. No complete memory-leak audit of Geant4 or its vendored dependencies is claimed.
