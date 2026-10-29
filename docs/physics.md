# Physics and geometry assumptions

## Implemented transport

Sequential Geant4 with Livermore electromagnetic physics, decay physics, and optical physics. The default electromagnetic production range cut is 0.1 mm. The default source is one 511 keV gamma per event at the origin, directed along +z. This is not a coincidence PET source. Positron primary kinetic energy is specified separately with `-x e+ -E ...`; annihilation then follows electromagnetic transport.

Both LYSO hemispheres are placed. The scintillator outer radius is 34.5 mm; default inner radius is 24.5 mm. The guide is a faceted solid with 12 pentagonal and 20 hexagonal regions leading to square sensor-array outlets. The optical outlet plane is at radial distance 39.5 mm along each array normal. The default reference sensor housing is shifted outward by half its depth so its window entrance contacts the outlet. The source places one inner hemispherical liner, so optical boundaries are not symmetric between hemispheres. This behavior is retained and must be reviewed before interpreting hemispheric response differences.

## Default materials

LYSO retains the supplied density of 7.36 g/cm³ and elemental mass fractions. Default yield is 32 photons/keV, decay constant 41 ns, refractive index 1.81, and absorption length 413 mm. The original repeated energy grid was invalid; the default now uses a provisional flat 2.79–2.81 eV spectrum. This is not a measured LYSO spectrum.

A custom spectrum is a CSV with header `energy_eV,intensity` and strictly increasing energies within 1–6 eV. Intensities must be nonnegative with at least one positive value. The column represents the spectrum in photon energy; a measured spectrum given per wavelength requires the appropriate variable conversion before use. Example:

```csv
energy_eV,intensity
2.6,0
2.8,1
3.0,0
```

This example is illustrative, not calibration data. Refractive index and absorption length remain constant over the supplied energy grid.

The generic guide epoxy is modeled as C15H16O2 with density 1.2 g/cm³ and refractive index 1.5. The upstream nominal Bisphenol A formula contained silicon; the integration corrects that atom to carbon. There is no epoxy bulk absorption by default. `--epoxy-index` and `--epoxy-absorption-mm` affect the shared epoxy material, including gaskets and reference sensor windows, not just the guide. The photographed resin and coupling layers have not been measured or modeled independently.

The upstream ceramic sensor package has no optical refractive index, causing NoRINDEX termination at applicable boundaries. Idealized reflective/absorptive surfaces from the supplied detector are retained. In particular, the absorbing outer boundary can produce Geant4's surface Detection status; it is excluded from accepted SiPM hit counts.

## Sensors

Default: upstream `HamamatsuS1036233100`, 900 cells with 100 μm pitch, 3×3 mm active extent, and default 0.3 mm housing depth. It fits the 4.4 mm sensor-center spacing. The missing custom `HamamatsuS141604050HS` is not included. Unsupported names fail explicitly; oversized packages also fail the array-fit checks.

Available factory names: `generic`, `HamamatsuS1036211100`, `HamamatsuS1036233100`, `HamamatsuS10985100`, `HamamatsuS12651050`, `HamamatsuS1036233050`, `HamamatsuS12573100C`, and `HamamatsuS12573100X`. Availability does not imply a model fits this geometry. The 6 mm S12573 variants cannot fit the fixed 18 mm outlet in a 4×4 array.

Upstream PDE and cell-geometry filters are enabled by default. The PDE filter includes fill-factor and Fresnel corrections from the reference library. The application does not invoke the electronics digitizer: dark pulses, crosstalk, afterpulsing, cell recovery, waveform formation, and electronic time smearing are not added.

## Interpretation limits

This application supplies detector geometry, radiation transport, scintillation transport and accepted optical-photon records. It does not yet provide calibrated interaction localization, coincidence sorting, time-of-flight reconstruction, image reconstruction, or validated scanner sensitivity/resolution. Before quantitative studies, supply the actual sensor specification, emission spectrum, guide optical properties, surface/coupling model, and intended geometry; then validate transport and reconstruction against data or benchmarks.

The shaded viewer displays only the upper hemisphere and can clip the guide. These are presentation settings. The simulation remains a full sphere. The rendering's transparency and shading are illustrative, not a rendering of the simulated optical transport.
