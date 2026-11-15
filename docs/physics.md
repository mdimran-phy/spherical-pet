# Physics and geometry assumptions

## Implemented transport

Sequential Geant4 with Livermore electromagnetic physics, decay physics, and optical physics. The default electromagnetic production range cut is 0.1 mm. The default source is one 511 keV gamma per event at the origin, directed along +z. This is not a coincidence PET source. Positron primary kinetic energy is specified separately with `-x e+ -E ...`; annihilation then follows electromagnetic transport.

Both scintillator hemispheres are placed. The outer radius is 34.5 mm and the
default inner radius is 24.5 mm. The default guide has 12 pentagonal and 20
hexagonal regions and fixed 39.5 mm outlet planes. The octagon variant remains
an optional derived geometry, outside Dr. Ray's baseline.

## Materials and surfaces

GAGG is the default; `--scintillator LYSO` enables a comparison material.
GAGG uses the Gd3Al2Ga3O12 host composition and density 6.63 g/cm³. Its provisional
optics are 57 photons/keV, a single 88 ns decay, refractive index 1.9, and an
**assumed**, unmeasured 1000 mm absorption length. Trace Ce is represented through
the scintillation response, not an explicit dopant mass fraction.
LYSO retains density 7.36 g/cm³, the prior elemental fractions, 32 photons/keV,
41 ns, index 1.81, and 413 mm absorption length.

`scint-energy-ev = auto` centers a provisional flat spectrum at 1239.841984/520 eV
for GAGG or 2.8 eV for LYSO, with full width 0.02 eV. This is a transport placeholder,
not a measured emission spectrum. `--spectrum` accepts energy_eV,intensity CSV
with at least two strictly increasing energies in 1–6 eV and finite nonnegative
intensities with positive total. A wavelength-density spectrum must be converted
with its Jacobian before use. Constant material RINDEX and ABSLENGTH span 1–6 eV
independently of the scintillation-emission interval.

Guide resin, coupling, and reference sensor-window epoxy are separate materials.
The guide surrogate is C15H16O2 at 1.2 g/cm³; coupling is a silicone-like C2H6OSi
surrogate at 1.0 g/cm³. Neither composition is a measured product specification.
Guide index is provisionally 1.5; coupling index 1.43; both assumed absorption
lengths are 1000 mm. Their optical parameters are independently configurable.
The existing equatorial 0.1 mm seam uses coupling material. A 0.05 mm radial
coupling layer occupies part of the former guide volume; scintillator and outlet
dimensions remain fixed. Sensor windows contact the guide directly; a separate
sensor-side adhesive thickness is not yet modeled.

A full air cavity replaces the historical one-sided epoxy liner. Both radial
inner interfaces use UNIFIED ground surfaces, with provisional sigma_alpha=0.1 rad
and specular-lobe weight 1 (reflection about sampled microfacet normals).
The baseline assumes **bare** inner surfaces. Optional reflector/absorber modes
are assumptions for studies, not confirmed coatings. Scintillator-to-coupling
outer interfaces are explicitly polished dielectric boundaries. Border surfaces
are installed in both directions.

Exposed guide-to-air and guide-to-mask boundaries default to absorption, retaining
the earlier black-mask intent and extending it to exposed guide regions. A bare
or reflective boundary can be selected. Coatings use zero detection efficiency;
only accepted sensor records count as SiPM hits. Ground finish does not imply a
perfect reflector. See [the maintained baseline record](ray_baseline.md) for
parameter sources, limitations and open inputs.

The reference sensor ceramic has no optical refractive index, so NoRINDEX
termination can still occur there. The `epoxy-*` options now affect only the
reference window and outer-mask material; use `guide-*` and `coupling-*` for
those separate components. The corrected surfaces and additional coupling layer
mean LYSO mode does not reproduce old reference runs bit for bit.

## Sensors

Default: upstream `HamamatsuS1036233100`, 900 cells with 100 μm pitch, 3×3 mm active extent, and default 0.3 mm housing depth. It fits the 4.4 mm sensor-center spacing. The missing custom `HamamatsuS141604050HS` is not included. Unsupported names fail explicitly; oversized packages also fail the array-fit checks.

Available factory names: `generic`, `HamamatsuS1036211100`, `HamamatsuS1036233100`, `HamamatsuS10985100`, `HamamatsuS12651050`, `HamamatsuS1036233050`, `HamamatsuS12573100C`, and `HamamatsuS12573100X`. Availability does not imply a model fits this geometry. The 6 mm S12573 variants cannot fit the fixed 18 mm outlet in a 4×4 array.

Upstream PDE and cell-geometry filters are enabled by default. The PDE filter includes fill-factor and Fresnel corrections from the reference library. The application does not invoke the electronics digitizer: dark pulses, crosstalk, afterpulsing, cell recovery, waveform formation, and electronic time smearing are not added.

## Interpretation limits

This application supplies detector geometry, radiation transport, scintillation transport and accepted optical-photon records. It does not yet provide calibrated interaction localization, coincidence sorting, time-of-flight reconstruction, image reconstruction, or validated scanner sensitivity/resolution. Before quantitative studies, supply the actual sensor specification, emission spectrum, guide optical properties, surface/coupling model, and intended geometry; then validate transport and reconstruction against data or benchmarks.

The shaded viewer displays only the upper hemisphere and can clip the guide. These are presentation settings. The simulation remains a full sphere. The rendering's transparency and shading are illustrative, not a rendering of the simulated optical transport.
