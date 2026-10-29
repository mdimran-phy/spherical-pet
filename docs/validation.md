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
