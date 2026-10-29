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
    common=['-e','1','-x','e-','-E','100','--source-z-mm','28','--lyso-yield','2','--track-photons','2','--mesh-steps','12','-seed','1234']
    for name in ['a','b']:run([*common,'-o',root/name])
    for name in ['events.csv','sipm_hits.csv','optical_steps.csv']:
        assert (root/'a'/name).read_bytes()==(root/'b'/name).read_bytes(),f'Non-reproducible {name}'
    summary=json.loads((root/'a/summary.json').read_text())
    assert summary['events']==1 and summary['guard_killed_tracks']==0
    assert summary['optical_created']>0 and 90<summary['edep_lyso_keV']<=100.001
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
print('PASS: deterministic transport, photon accounting, saved optical steps, sensor inventory, parameterized geometry')
