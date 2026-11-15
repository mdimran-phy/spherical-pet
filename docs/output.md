# Output reference

Coordinates are world coordinates in millimeters. Energies are labeled in each header. Times are Geant4 global times in nanoseconds relative to the event origin. IDs are zero-based. Output is plain CSV/JSON so ROOT, Python, R, or other tools can read it without a custom dictionary.

## Event and hit records

`events.csv`: `event, edep_scintillator_keV, optical_created, scintillation_created, cherenkov_created, sipm_accepted_hits, guard_killed_tracks`.

Energy deposition counts non-optical particle deposits in the selected scintillator, avoiding optical-energy double counting. Optical production counts actual tracked optical photons. Accepted hits are photon records that passed the upstream sensor filter, not digitized pulses.

`sipm_hits.csv`: `event, sipm_id, track_id, time_ns, energy_eV, x_mm, y_mm, z_mm`.

The sensor ID divided by 16 gives the array index. Arrays 0–11 are on pentagonal guide regions; arrays 12–31 are on hexagonal or beveled octagonal regions according to `guide-shape`. Hit positions are actual photon positions, whereas `sensor_positions.csv` stores sensor centers. Event/track ID pairs uniquely identify accepted photons within a run. Arrival times include emission delay and optical propagation; no electronics timing resolution has been added.

## Trajectories

`gamma_steps.csv`: `event, track_id, parent_id, x0_mm, y0_mm, z0_mm, x1_mm, y1_mm, z1_mm, edep_keV, process`.

This includes primary and secondary gamma steps. A gamma step's energy deposit does not include the energy later deposited by its secondary electrons; use `events.csv` for total scintillator energy deposition.

`optical_steps.csv`: `event, track_id, parent_id, step, x0_mm, y0_mm, z0_mm, x1_mm, y1_mm, z1_mm, t0_ns, t1_ns, process`.

Enabled by `--track-photons N`. It records all steps of the first N optical tracks started in each event. The process defining a boundary step may be Transportation; direction changes across consecutive segments reveal reflected/refracted paths. This file does not currently store individual boundary-process status codes.

## Summary and termination accounting

`summary.json` stores event count, total scintillator energy deposition, total optical photons, accepted hits, guard kills, and mutually exclusive optical termination categories. Their counts should sum to the number of optical photons tracked.

For Geant4 11.1.2, `boundary_status_10` is surface Detection and `boundary_status_14` is NoRINDEX. The baseline outer absorber uses zero detection efficiency, so it reports absorption rather than Detection. Historical runs used a different absorber setting; neither is a SiPM hit. `sensor_sensitive_volume` includes all optical terminations in sensitive volumes; some photons are rejected by the sensor filters and do not create hit rows. `OpAbsorption` is bulk absorption; `world_escape` is escape from the world.

A nonzero track-guard count causes exit code 1. Inspect the geometry and transport before treating such a run as usable. Geometry checks are stochastic surface sampling; zero reported overlaps is not a mathematical proof that a complex shape is valid.

## Reproducibility

Keep `resolved.conf`, `source_commands.mac`, `spectrum.csv`, `build.txt`, the repository commit ID, and any custom source inputs with your data. The random engine is reseeded after initialization and any overlap sampling. Same-version repeatability is tested; bitwise equality across different compilers or Geant4 versions is not promised.

`resolved.conf` records requested configuration values; a source macro may override the source. `source_commands.mac` records the actual sequence applied. To replay a run from another directory, use its saved spectrum and source-command files explicitly and select a new output directory.

Geometry mode writes configuration, spectrum, build information, optional geometry exports, and `geometry_check.json`; it does not write event records or a transport summary.

`guide_sections.csv` records each selected shoulder polygon: `array_id,shape,vertex,x_mm,y_mm,z_mm`. It is written in geometry and simulation modes, including when mesh export is disabled. See [geometry](geometry.md) for how these outlines relate to the solid.

## Schema 2 and CNN preparation

Starting with version 0.2.0, `events.csv` and `summary.json` use
`edep_scintillator_keV` instead of `edep_lyso_keV`. The summary also records
`schema_version: 2` and `scintillator` (`GAGG` or `LYSO`). Bundled plotting
scripts accept both schemas; external consumers must update their field names.

`sipm_counts.csv`: `event,sipm_id,photon_count`.
Exactly 512 rows are written per event, including zero-hit channels. Counts
include only accepted hits and sum to `sipm_accepted_hits` in `events.csv`.
They are raw counts, without normalization or conversion into an annular image.

`gamma_interactions.csv`:
`event,track_id,parent_id,interaction_index,process,time_ns,x_mm,y_mm,z_mm,pre_energy_keV,post_energy_keV,local_edep_keV,material`.
Records gamma steps inside the selected scintillator whose defining process is
not Transportation. Includes photoelectric, Compton, and Rayleigh interactions,
and secondary gammas. Indexing starts at 1 **per event/track**, counting only
interactions in scintillator. Position is the post-step interaction point.
`local_edep_keV` is only this gamma step's deposit; energy later deposited by
secondary electrons belongs in the event total. An interaction may have zero
local deposit and still generate electrons and scintillation downstream.

For the single-gamma baseline, select `parent_id == 0` for the primary's history.
Rayleigh scattering does not create a scintillation site; retain it as history
but exclude it when constructing energy-deposit-site labels. First/second
Compton or photoelectric sites can be selected offline. No restriction to two
sites or energy threshold is imposed during output. Events without interactions
still have event/count rows; they have no interaction rows. Saving these records
does not train a CNN or define a spherical image layout.
