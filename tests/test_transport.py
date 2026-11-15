#!/usr/bin/env python3
"""Small deterministic transport and geometry integration check."""
import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path
exe=str(Path(sys.argv[1]).resolve())
def run(args):
    p=subprocess.run([exe,*map(str,args)],text=True,capture_output=True,timeout=140)
    if p.returncode:
        raise AssertionError(p.stdout[-6000:]+'\n'+p.stderr[-3000:])
    return p
with tempfile.TemporaryDirectory() as temp:
    root=Path(temp)
    common=['-e','1','-x','e-','-E','100','--source-z-mm','28','--gagg-yield','2','--track-photons','2','--mesh-steps','12','-seed','1234']
    for name in ['a','b']:run([*common,'-o',root/name])
    for name in ['events.csv','sipm_hits.csv','optical_steps.csv','sipm_counts.csv','gamma_interactions.csv']:
        assert (root/'a'/name).read_bytes()==(root/'b'/name).read_bytes(),f'Non-reproducible {name}'
    summary=json.loads((root/'a/summary.json').read_text())
    assert summary['events']==1 and summary['guard_killed_tracks']==0
    assert summary['optical_created']>0 and 90<summary['edep_scintillator_keV']<=100.001
    assert sum(summary['optical_endings'].values())==summary['optical_created']
    with (root/'a/sipm_hits.csv').open() as f:hits=list(csv.DictReader(f))
    assert len(hits)==summary['sipm_accepted_hits']
    assert len({(h['event'],h['track_id']) for h in hits})==len(hits)
    with (root/'a/optical_steps.csv').open() as f:steps=list(csv.DictReader(f))
    assert steps and len({s['track_id'] for s in steps})<=2
    with (root/'a/sensor_positions.csv').open() as f:sensors=list(csv.DictReader(f))
    assert len(sensors)==512
    run(['--mode','geometry','--shell-thickness-mm','8','--sipm-pitch-mm','4','--mesh-steps','12','--overlap-samples','100','-o',root/'geometry'])
    check=json.loads((root/'geometry/geometry_check.json').read_text())
    assert check['volumes_reporting_overlaps']==0
    run([*common,'--guide-shape','pentagon-octagon','--check-overlaps','1',
         '--overlap-samples','100','-o',root/'octagon'])
    octagon=json.loads((root/'octagon/summary.json').read_text())
    assert octagon['guard_killed_tracks']==0 and octagon['sipm_accepted_hits']>0
    assert sum(octagon['optical_endings'].values())==octagon['optical_created']
    check=json.loads((root/'octagon/geometry_check.json').read_text())
    assert check['volumes_reporting_overlaps']==0
    assert 'o chapeauPoofMinusHomeOrb_PV\n' in (root/'octagon/geometry.obj').read_text()
    assert (root/'octagon/geometry.obj').read_bytes()!=(root/'a/geometry.obj').read_bytes()
    assert (root/'octagon/sensor_positions.csv').read_bytes()==(root/'a/sensor_positions.csv').read_bytes()
    with (root/'octagon/guide_sections.csv').open() as f:sections=list(csv.DictReader(f))
    assert len(sections)==12*5+20*8
    for array in range(32):
        vertices=[r for r in sections if int(r['array_id'])==array]
        assert len(vertices)==(5 if array<12 else 8)
        assert {r['shape'] for r in vertices}==({'pentagon'} if array<12 else {'octagon'})
    assert 'guide-shape = pentagon-octagon' in (root/'octagon/resolved.conf').read_text()
    # Preserve zero-hit channels and reconcile accepted hits with CNN-ready counts.
    with (root/'a/sipm_counts.csv').open() as f: counts=list(csv.DictReader(f))
    assert len(counts)==512 and {int(c['sipm_id']) for c in counts}==set(range(512))
    assert sum(int(c['photon_count']) for c in counts)==summary['sipm_accepted_hits']
    assert summary['scintillator']=='GAGG' and summary['schema_version']==2
    # Single gammas, including transport/Compton/photoelectric history in GAGG.
    run(['-e','20','-seed','42','--gagg-yield','2','--export-geometry','0','-o',root/'gamma'])
    with (root/'gamma/gamma_interactions.csv').open() as f: interactions=list(csv.DictReader(f))
    assert interactions and all(r['material']=='GAGG' for r in interactions)
    assert any(r['process']=='compt' for r in interactions)
    assert any(r['process']=='phot' for r in interactions)
    sequences={}
    for r in interactions:
        key=(r['event'],r['track_id'])
        sequences.setdefault(key,[]).append(int(r['interaction_index']))
        radius=sum(float(r[k])**2 for k in ('x_mm','y_mm','z_mm'))**0.5
        assert 24.5-1e-5<=radius<=34.5+1e-5
        assert 0<=float(r['post_energy_keV'])<=float(r['pre_energy_keV'])+1e-6
    assert all(v==list(range(1,len(v)+1)) for v in sequences.values())
    with (root/'gamma/sipm_counts.csv').open() as f: counts=list(csv.DictReader(f))
    with (root/'gamma/events.csv').open() as f: events=list(csv.DictReader(f))
    assert len(counts)==20*512
    for e in events:
        assert sum(int(c['photon_count']) for c in counts if c['event']==e['event'])==int(e['sipm_accepted_hits'])
    # Both hemispheres produce optical signals; alternative coatings remain runnable.
    run(['-e','1','-x','e-','-E','100','-gz','-28','--gagg-yield','2',
         '--inner-surface','reflector','--guide-boundary','bare','--export-geometry','0',
         '-o',root/'south'])
    south=json.loads((root/'south/summary.json').read_text())
    assert south['sipm_accepted_hits']>0 and south['guard_killed_tracks']==0
    run(['-e','1','-x','e-','-E','100','-gz','28','--scintillator','LYSO',
         '--lyso-yield','2','--export-geometry','0','-o',root/'lyso'])
    lyso=json.loads((root/'lyso/summary.json').read_text())
    assert lyso['scintillator']=='LYSO' and lyso['optical_created']>0
print('PASS: deterministic transport, photon accounting, saved optical steps, sensor inventory, parameterized geometry')
