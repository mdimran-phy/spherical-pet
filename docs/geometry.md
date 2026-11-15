# Light-guide shapes

`--guide-shape` selects real Geant4 transport geometry, not a display setting. The default is `pentagon-hexagon`; use `pentagon-octagon` for the derived variant. Both retain the full spherical scintillator shell, 32 square outlets, and 512 SiPMs. The supplied source has pentagons and hexagons; the octagon variant is a new parameterized design, not an exact CAD reconstruction of the photograph.

| Value | Pentagonal shoulders | Other shoulders | Construction |
|---|---:|---:|---|
| `pentagon-hexagon` | 12 | 20 hexagons | Supplied guide solid |
| `pentagon-octagon` | 12 | 20 irregular octagons | Two opposite corners cut from each hexagonal shoulder |

## Bevel construction

The shoulder is the polygon where each original guide section begins its transition toward the square sensor outlet. Its vertices and ordering come from the supplied source. For hexagon vertices `v0..v5`, corners `v0` and `v3` are replaced by two points each:

```
a = vi + f * (v_previous - vi)
b = vi + f * (v_next     - vi)
```

The four untouched corners plus four new points form eight sides. `f` is `--guide-bevel-fraction`, default 0.25, strictly between 0 and 0.5. It is ignored for the default shape. These octagons are irregular; this is not a new tessellation of regular pentagons and octagons over a sphere.

At each removed corner, the triangular cut tapers inward to `0.85 * vi` (inside the outer scintillator radius), and outward to the nearest corner of the original square outlet. Each cut is one closed, six-triangle solid. The union of 40 cuts is subtracted from the original guide. This preserves the square outlet area and adds no guide material. The spherical scintillator itself is untouched. The removed guide volume becomes world air; The coupling seam and outer mask remain. The baseline now has a full air cavity, a thin radial coupling layer, and explicit configurable boundaries; see [the baseline record](ray_baseline.md). Consequently the response changes, and octagon and hexagon runs must not be treated as optically equivalent.

`guide_sections.csv` records the selected shoulder polygons in world coordinates, one row per vertex, with array ID and shape. It describes construction outlines; `geometry.obj` is the actual Geant4 solid mesh including curved interfaces and bevel faces. `resolved.conf` records both shape parameters. The photo-style viewer labels the selected shape from that file.

## Examples

```bash
./build/spherical-pet --config config/octagon.conf -r 1 -o results/octagon_geometry
./build/spherical-pet --guide-shape pentagon-octagon --guide-bevel-fraction 0.35 \
    --check-overlaps 1 -e 10 -seed 42 -o results/octagon35
python3 scripts/photo_geometry_display.py --input results/octagon35
```

Use a new output directory for each run. Run overlap checks after geometry changes. Very small bevels can be below a selected display-mesh resolution or geometry-check tolerance; the configurable fraction is not a machining specification. Detector dimensions and surface properties still need experimental/CAD validation before drawing detector-performance conclusions.
