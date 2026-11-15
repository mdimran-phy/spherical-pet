# Validation record — 2026-10-29

## Environment

- Geant4 11.1.2 (sequential application using the installed MT-capable library).
- cmake version 3.31.8
- c++ (GCC) 11.5.0 20240719 (Red Hat 11.5.0-14)
- Python 3.9.25

## Completed checks

A separate local clone under a temporary directory was configured and built from the committed source. It did not use the original working directory's build tree or dependency checkout. All three CTest tests passed: configuration unit tests, CLI behavior, and deterministic transport/geometry integration. Installation to a separate prefix succeeded, and the installed executable reported its version.

The optional `WITH_GEANT4_UIVIS=ON` build compiled successfully. A native GUI window was not opened on this headless host. Python scripts generated both empty-event and nonempty-run plots. Standalone HTML data embedding and JavaScript syntax were checked; interactive WebGL behavior was not browser-tested on this host.

The default detector's overlap audit checked 2,055 placed volumes with 1,000 samples per volume and 0.001 mm tolerance, with zero overlap reports. The integration test additionally checked 8 mm shell thickness and 4 mm sensor pitch with 100 samples per volume. These are sampled checks.

## Reference transport run

```bash
./build/spherical-pet -e 10 -seed 42 --track-photons 5 -o results/example10
```

- Events completed: 10.
- Total LYSO energy deposition: 1592.07665036 keV.
- Optical photons: 50,845.
- Accepted SiPM hits: 3,819.
- Track-guard kills: 0.

This is a small transport check, not a detector-performance measurement. Some primary gammas escape without depositing energy; event 0 in this reference run is one such event. Output files and local builds are deliberately excluded from the source repository.

The independent configuration-only compile used by GitHub CI also passed. No CI result from GitHub itself is claimed before publication.

## Selectable guide shapes

The shape update adds a fourth CTest test for closed, navigable bevel solids at fractions 0.01, 0.25, and 0.49. The transport test now runs both guide options and checks optical-photon accounting, nonzero octagon-mode SiPM response, identical sensor positions, exported guide meshes, shoulder vertex counts, and zero sampled overlaps. All four tests pass locally.

```bash
./build/spherical-pet --config config/octagon.conf --check-overlaps 1 \
    -e 10 -seed 42 --track-photons 5 -o results/octagon_verified
python3 scripts/photo_geometry_display.py --input results/octagon_verified
```

The octagon run completed 10 events: 1543.62601098 keV deposited in LYSO, 49,258 optical photons, 4,033 accepted SiPM hits, and zero track-guard kills. Its overlap check reported zero overlaps across 2,055 volumes, using 1,000 samples per volume and 0.001 mm tolerance. The exported guide uses the actual sequential Boolean corner subtractions. Mesh export now fails explicitly if any top-level placed solid cannot produce a mesh, rather than silently omitting that component.

These results validate execution and sampled geometry consistency, not the optical performance of a manufactured detector. The octagon mode is the documented irregular bevel design.

## Detector report cleanup — 2026-11-11

The remaining applicable source-cleanup recommendations are implemented. See [the change record](report_fixes.md) for the report-to-code mapping and ownership details.

The application rebuilt successfully and all six local CTest tests passed. New tests cover geometry-store reachability, hemisphere/gasket interior separation, one placed main guide, 512 distinct sensitive sensors, rotation replacement, and sensor/digitizer teardown. Constructor-error paths are covered by the CLI test.

Both guide modes reported zero overlaps across 2,055 checked volumes at 1,000 samples per volume and 0.001 mm tolerance. For each guide mode, eight geometry and event-output files matched the pre-cleanup reference byte for byte. The detector and new regression test passed a warning-as-error syntax check. [Verification hashes and audit results](validation/report_cleanup_20261111/verification.json) and [test output](validation/report_cleanup_20261111/ctest.log) are retained.

## Native event display — 2026-11-11

The main `build` executable was rebuilt with `WITH_GEANT4_UIVIS=ON`. All seven CTest tests passed, including the new native OpenGL test under Xvfb. The native test checks stored particle trajectories, optical-path limits of four and zero, nonzero SiPM hits, and clean session exit. Event, hit, gamma-step, optical-step, and summary outputs match batch mode byte for byte. CLI tests cover a missing graphical display and reject the request before creating output files.

A separate Qt window was rendered under Xvfb for 10 events with seed 42 and 50 displayed optical paths per event. Its screenshot was visually inspected: detector geometry, gamma and optical tracks, SiPM markers, and the Qt controls are visible. The run reproduced 1592.07665036 keV deposited, 50,845 optical photons, 3,819 accepted SiPM hits, and zero guard kills. The Qt preview process was terminated after capture; clean session exit is covered by the terminal-driven OpenGL regression test. Manual Qt interaction and SSH forwarding to a user's desktop were not tested.

This farm requires matching Qt plugin and private-library versions. CMake now generates `build/native-env.sh` from the selected Qt target; source it after the Geant4 environment. The native regression test also passed with this environment loaded. See the README for the launch command, event review controls, and the distinction between displayed optical paths and simulated photons. Preview files remain under the ignored `results/native_display_preview_20261111_fixed/` directory.

## Dr. Ray GAGG baseline — 2026-11-13

Version 0.2.0 builds with Geant4 11.1.2; all seven CTest tests pass. New checks
cover GAGG host/density, symmetric polished/ground boundaries, independent
materials, gamma interaction records, per-sensor counts including zero-hit
channels, alternate coatings and LYSO comparison.

The 10-event single-511-keV-gamma run (seed 42, default GAGG yield) deposited
1882.12381162 keV, created 107,474 optical photons and recorded 5,295 accepted
SiPM hits. No guard kills occurred. Photon termination counts reconcile exactly.
The sampled overlap audit reported zero overlaps across 2,056 volumes at 1,000
samples per volume and 0.001 mm tolerance. All three offline display scripts
completed; the rendered cutaway was visually inspected. These are smoke checks,
not measured calibration or scanner-performance validation.

See the maintained [baseline record](ray_baseline.md) and
[test log](validation/ray_baseline_20261113/ctest.txt). Older validation entries
above describe the earlier LYSO model and are retained as history.
