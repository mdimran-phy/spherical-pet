# Dr. Ray spherical PET baseline — implementation record

Last updated: **2026-11-15**. Application version: **0.2.0**.
Project: `Medical_imaging/spherical-pet`; starting revision: `c09afe7`.
This is the maintained record for this baseline. Update the implementation,
assumptions, validation and next-actions sections together when the model changes.
The original supplied files in the parent directory and `pet_run_20261029` are
historical references and were not modified by this implementation.

## Confirmed instructions and implementation

| Dr. Ray's instruction | Implemented behavior |
|---|---|
| Use a full sphere | Both scintillator hemispheres remain placed; a full air cavity makes inner optical treatment symmetric. |
| Keep current dimensions for now | Outer scintillator radius 34.5 mm, default inner radius 24.5 mm, outlet planes 39.5 mm, outlet width 18 mm. |
| 12 pentagons and 20 hexagons | `guide-shape = pentagon-hexagon` remains the baseline; the optional octagon experiment is not used. |
| Use GAGG instead of LYSO | Default material is GAGG; host chemistry, density and optical properties are replaced, not just the material name. LYSO remains selectable for comparison. |
| Outer surface polished, inner surface ground | Explicit two-way polished outer dielectric interfaces and two-way UNIFIED ground inner interfaces on both hemispheres. |
| Use the reference sensor until array design is established | `HamamatsuS1036233100`, 32 arrays of 4×4, 512 channels; adjustable pitch and validated package fit. |
| Guide material undecided; representative coupling optics allowed | Separate guide, coupling and window materials with independent parameters and documented provisional values. |
| First milestone: geometry, light transport, SiPM collection | Single 511 keV gamma source, overlap auditing, photon accounting and per-channel collection. |
| Save data for later CNN interaction positioning | Per-photon hits, dense per-SiPM counts, gamma interaction history and event energy deposits. |

## Provisional assumptions — not supplied measurements

| Quantity | Baseline | Basis / limitation |
|---|---|---|
| GAGG host | Gd3Al2Ga3O12, 6.63 g/cm³ | Manufacturer reference [3]; no supplier/grade selected by Dr. Ray. Trace Ce is represented by optical response, not an explicit dopant fraction. |
| GAGG light yield | 57 photons/keV | Representative 57,000 photons/MeV from [3]; configurable. |
| GAGG decay | 88 ns, one component | Representative value from [3]; no slow component or calibrated timing model. |
| GAGG refractive index | 1.9, constant | Reference [3]; wavelength dispersion not modeled. |
| GAGG emission | Flat 0.02 eV interval centered at 1239.841984/520 eV | 520 nm reference peak; the flat shape and width are placeholders, not a measured spectrum. Replace through `--spectrum`. |
| GAGG absorption | 1000 mm, constant | Assumption only; not inferred from the papers or a supplier measurement. |
| Guide resin | C15H16O2 surrogate, 1.2 g/cm³; index 1.5; absorption 1000 mm | Index is consistent with the approximate resin indices in [1]; composition, density and absorption are generic placeholders, not a model of a named printing resin. |
| Coupling | `EncapsoSurrogate`: C2H6OSi, 1.0 g/cm³; index 1.43; absorption 1000 mm | Generic silicone-like surrogate; no claim of measured Encapso chemistry or optical constants. |
| Radial coupling thickness | 0.05 mm | Assumption; carved from the guide's inner volume. Scintillator radii and guide outlets remain unchanged. Configurable in (0, 0.5] mm. |
| Equatorial seam | Existing 0.1 mm total thickness, coupling material | Supplied two-hemisphere construction retained. |
| Sensor-side coupling | Direct guide/window contact | A separate adhesive layer at each SiPM is not yet modeled. Reference window optics remain separate from coupling/guide settings. |
| Ground roughness | sigma_alpha = 0.1 rad, microfacet specular-lobe weight 1 | UNIFIED-model assumption; ground finish alone does not provide a numerical roughness. |
| Inner coating | Bare dielectric boundary | No coating was specified in the email. `reflector` or `absorber` can be studied; reflector defaults to 0.98 reflectivity only when selected. |
| Guide boundaries | Absorbing exposed guide-to-air and guide-to-mask boundaries | Retains the prior black-mask intent, extends it to exposed guide regions, and is configurable as bare/reflector/absorber. Not confirmed for manufactured guides. |

All absorption lengths above describe bulk absorption only. Scattering lengths
and measured surface response are not calibrated. Do not convert the paper's
sample transmission directly into pure bulk absorption: interface losses and
scattering also contribute. Custom spectra must be specified as intensity per
photon energy; wavelength-density data require the appropriate conversion.

## How the two papers are used

1. [Raylman et al., light-guide materials study](https://pmc.ncbi.nlm.nih.gov/articles/PMC8639756/),
   DOI 10.1016/j.apradiso.2021.109979. Candidate printed and cast materials are
   Somos WaterClear Ultra 10122 and Polytek Poly-Optic 1730. Their reported
   1 cm transmission at 420 nm is 49% and 73%. The detector demonstration used
   LYSO and Sylgard 184, so it does not calibrate the proposed GAGG/Encapso model.
   Its curves can inform later material comparisons; no curve has been silently
   substituted for a measured GAGG-spectrum-weighted response.
2. [Jaliparthi et al., AnnPET simulation and CNN](https://pmc.ncbi.nlm.nih.gov/articles/PMC8908313/),
   DOI 10.1088/1361-6560/ac0d0c. Provides the simulation-to-CNN pattern, including
   single-gamma training, per-sensor distributions and first/second interaction
   labels. The annular 12×16 image layout is not imposed on the sphere. Save all
   channel counts and gamma interaction histories so spherical preprocessing can
   be chosen later. This implementation does not claim to reproduce AnnPET's
   reported resolution or validate against its original code.
3. [Hilger GAGG datasheet](https://www.hilger-crystals.co.uk/wp-content/uploads/2024/12/datasheet-GAGG.pdf)
   supplies representative host, density, light yield, decay and index values.
   [C&A GAGG product data](https://c-and-a.jp/products.html) also list a 520 nm peak.
   These are baseline references, not confirmation of the eventual crystal grade.
4. [Geant4 optical-process documentation](https://geant4.web.cern.ch/documentation/pipelines/master/bfad_html/ForApplicationDevelopers/TrackingAndPhysics/physicsProcess.html)
   describes ordered border surfaces, dielectric Fresnel transport, UNIFIED
   roughness and the distinction between reflectivity and detection efficiency.

## Code and output changes

- `src/Config.cc`: GAGG default, material-specific properties, auto emission center,
  independent guide/coupling settings, coating/roughness controls and validation.
- `src/DetectorConstruction.cc`: GAGG chemistry and optics; separate material tables;
  radial coupling; full air cavity; symmetric inner/outer surfaces. Removes the
  unconfirmed one-sided perfectly reflecting liner. Absorbers have zero detection
  efficiency, so coating absorption cannot be mistaken for a sensor hit.
- `src/main.cc`: energy scoring recognizes the selected scintillator; schema 2
  uses `edep_scintillator_keV`, not the old material-specific field. Adds
  `gamma_interactions.csv` and `sipm_counts.csv` with zero-hit sensors included.
  Detailed hits retain times, positions, sensor IDs and track IDs.
- Display readers accept both old and new energy columns and use material-neutral
  labels. No CNN training, coincidence processing or scanner performance claim
  is introduced.
- `config/ray_baseline.conf` selects the intended baseline;
  `config/lyso_comparison.conf` changes the material in the new geometry.

The old `lyso-*` parameters still affect LYSO only. `epoxy-*` now affects reference
windows and the mask, not the separate guide/coupling materials. Selecting LYSO
is not a replay of version 0.1.0: surfaces, coupling geometry and optical-index
coverage changed. Constant optical tables cover 1–6 eV independently of the
scintillation-emission interval. See [output schemas](output.md) and
[all parameters](parameters.md).

## Reproducible commands

Load the same Geant4 installation used by the build first, then from the project:

```bash
cmake --build build -j4
ctest --test-dir build --output-on-failure
./build/spherical-pet --config config/ray_baseline.conf \
    -e 10 -seed 42 --track-photons 5 -o results/ray_baseline_20261113
python3 scripts/plot_results.py --input results/ray_baseline_20261113
python3 scripts/event_display.py --input results/ray_baseline_20261113 --event 0
python3 scripts/photo_geometry_display.py --input results/ray_baseline_20261113
```

The output path must be new for each run. Site environment used for validation:
`source /u/group/halla/apps/Geant4/11.1.2/el9/bin/geant4.sh`.

## Validation

Build succeeded with Geant4 11.1.2. All seven CTest tests passed, including
native display under Xvfb. Tests cover GAGG density and elements, both ordered
inner/outer interfaces, both guide geometries, sampled overlaps, deterministic
transport, zero-hit channel preservation and count reconciliation, gamma
interaction sequence/positions/processes, south-hemisphere collection, alternate
coatings and LYSO comparison. See [test log](validation/ray_baseline_20261113/ctest.txt).
The full-yield baseline (`-e 10 -seed 42 --track-photons 5`) completed:

| Check | Result |
|---|---|
| Completed events | 10 |
| GAGG energy deposit | 1882.12381162 keV |
| Optical photons created | 107,474 |
| Accepted SiPM hits | 5,295 |
| Track-guard kills | 0 |
| Optical termination accounting | Exact agreement with photon count |
| Sampled overlaps | 0 across 2,056 volumes, 1,000 samples/volume, 0.001 mm tolerance |
| Offline displays | All three scripts completed; rendered cutaway visually inspected |

Run metadata and totals are saved in
[validation/ray_baseline_20261113](validation/ray_baseline_20261113/).
Full CSV/mesh data and generated figures are under the ignored local directory
`results/ray_baseline_20261113/`. Event 0 has 1,099 accepted hits across 303
sensors. All 32 arrays received hits over this small run. The static cutaway
shows the upper hemisphere only; the transport model is the full sphere.
Standalone HTML generation was verified; browser interaction was not manually
retested. Native rendering is covered by the passing Xvfb regression test.

These are execution and geometry checks. They do not establish measured optical
agreement, calibrated PDE, event-position resolution, or PET sensitivity.

## Open inputs and next actions

Proceed with this baseline without waiting for additional answers. Before
quantitative design comparisons, obtain the current CAD, intended GAGG grade and
emission/absorption data, and actual inner/guide coatings. Scan documented
roughness, bulk absorption and coupling assumptions to determine which affect
collection most strongly. Obtain the AnnPET source/configuration and a reference
output before claiming reproduction of that benchmark. Sensor-array optimization,
CNN training and paired annihilation photons remain later milestones.

## Change log

- 2026-11-13: Implemented Dr. Ray's confirmed baseline, separated provisional
  inputs, added interaction/count output, updated displays and regression checks.

- 2026-11-15: Added a [beginner event-display guide](event_display_guide.md) explaining construction, particle histories, colors, counts and the seed-42 example. No physics settings changed.

## Beginner course — 2026-11-15

Added the [complete beginner presentation](presentations/beginner_20261115/README.md):
112 slides, editable PPTX, matching slide PDF, offline HTML, a 113-page study guide,
Markdown source, runnable saved-data lessons, glossary and references. Every slide
has code and explanatory notes. The material distinguishes current implementation,
provisional assumptions and future CNN/coincidence work.

New native screenshots under `results/presentation_native_20261115_*` reproduce
event 0 with 0, 5 and 100 optical paths drawn. Each produces 29,002 optical photons
and 1,099 accepted hits. Presentation data/accounting checks pass; diagrams and
rendered slide contact sheets were visually reviewed. PPTX was structurally checked;
native PowerPoint rendering is unavailable on this host. This is educational
documentation, not new detector calibration or performance validation.
