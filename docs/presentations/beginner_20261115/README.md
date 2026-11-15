# Spherical PET beginner presentation

112 slides in 10 chapters, with actual code excerpts, labeled teaching code, detailed notes, schematics, flowcharts, native event displays, actual data plots and answered exercises. Prepared 2026-11-15 against the current GAGG baseline.

## Start here

- **presentation.html** — open locally in a browser; arrow keys navigate, N toggles notes, chapter selector and search are available. Slide images and notes are embedded, so the HTML works alone without network access.
- **spherical_pet_beginner.pptx** — editable PowerPoint with speaker notes. Install DejaVu Sans and DejaVu Sans Mono for the closest layout match; applications may substitute fonts.
- **spherical_pet_beginner.pdf** — 112-slide PDF.
- **study_guide.pdf / study_guide.md** — all explanations, figures, code walkthroughs, answers and source references.
- **glossary.md / references.md** — definitions and full source links.
- **teaching_examples.py** — executable, read-only data lesson; standard-library Python only.
- **data/** — portable copy of the small reference CSV/JSON/configuration files.
- **source_snapshot/** — the files supporting the extracted implementation examples; this is not a complete buildable checkout.
- **assets/** — scientific PNG/SVG diagrams and native screenshots.
- **previews/** — rendered slide images and contact sheets, generated locally during regeneration and excluded from Git.
- **verification.json** — checks and source hashes.

Run the portable data lesson:

```bash
python3 teaching_examples.py
```

## How to use the course

Start with chapters 1–2 even if the detector screenshot is your immediate interest. Chapters 3–5 build the detector and follow gamma/electron/optical histories. Chapter 6 connects these to C++ actions. Chapter 7 reads the actual event. Chapters 8–10 provide terminal practice, interpretation, literature and exercises. Cover this over several lessons, not one lecture.

Each slide includes a code block. ACTUAL C++ excerpts are extracted from identified files and line numbers. TEACHING PYTHON explains an idea and sometimes depends on variables introduced in the preceding data-loading lesson. FUTURE / PSEUDOCODE is explicitly not an implemented API. The companion script provides complete runnable versions of the saved-data checks.

## Scope and evidence

The reference is results/ray_baseline_20261113: ten single 511 keV gamma events, seed 42. Event 0 deposited 511 keV and created 29,002 optical photons (28,983 scintillation + 19 Cherenkov), with 1,099 accepted hits across 303 of 512 sensors. The 0/5/100-path native captures reproduce those totals. Drawing limits do not reduce optical transport.

Diagrams state when they are schematic or not to scale. The detector has provisional material/boundary inputs, a reference SiPM, and no application electronics digitization, coincidence analysis or trained CNN. The two papers motivate future material and reconstruction work; their results are not claimed as this model's performance.

## Regeneration

The packaged content.py uses source_snapshot for exact excerpts. assets.py can regenerate the scientific diagrams from data; the native screenshots and original event overview are preserved assets. build.py regenerates the presentation files. Dependencies: Python 3.9+, numpy, matplotlib, Pillow, python-pptx, reportlab, Poppler pdftoppm, and DejaVu fonts. The default build reads existing assets. Native capture additionally needs the original project, Geant4 11.1.2, Xvfb and ImageMagick; its paths are installation-specific.

```bash
python3 build.py
```

The matching PDF was rendered and inspected; PPTX was reopened and checked for slide/notes counts and canvas bounds. This host has no PowerPoint/LibreOffice renderer, so application-specific font substitution remains possible. All presentation data checks pass; they do not add physical calibration or performance validation.
