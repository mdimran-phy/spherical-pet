#!/usr/bin/env python3
"""Read-only checks of the bundled GAGG reference data (standard library only)."""
import argparse, csv, json, math
from pathlib import Path
from collections import Counter
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--run',type=Path,default=Path(__file__).resolve().parent/'data')
a=p.parse_args()
def read(name):
    with (a.run/name).open() as f:return list(csv.DictReader(f))
events=read('events.csv')
hits=read('sipm_hits.csv')
counts=read('sipm_counts.csv')
history=read('gamma_interactions.csv')
event_hits=[r for r in hits if int(r['event'])==0]
event_counts=[r for r in counts if int(r['event'])==0]
primary=[r for r in history if r['event']=='0' and r['parent_id']=='0']
assert len(events)==10
assert len(event_hits)==1099
assert len(event_counts)==512
assert sum(int(r['photon_count']) for r in event_counts)==1099
assert len(set(r['sipm_id'] for r in event_hits))==303
assert [r['process'] for r in primary]==['compt']*4+['Rayl','phot']
assert int(events[0]['optical_created'])==29002
assert int(events[0]['scintillation_created'])==28983
assert int(events[0]['cherenkov_created'])==19
assert sum(int(r['optical_created']) for r in events)==107474
assert len(hits)==5295
assert all(int(r['guard_killed_tracks'])==0 for r in events)
with (a.run/'summary.json').open() as f: summary=json.load(f)
assert sum(summary['optical_endings'].values())==summary['optical_created']
for e in events:
    event=e['event']; ec=[r for r in counts if r['event']==event]
    eh=[r for r in hits if r['event']==event]
    assert len(ec)==512
    assert len({r['sipm_id'] for r in ec})==512
    assert sum(int(r['photon_count']) for r in ec)==len(eh)==int(e['sipm_accepted_hits'])
    assert len({r['track_id'] for r in eh})==len(eh)
first=primary[0]
r=math.sqrt(sum(float(first[k])**2 for k in ['x_mm','y_mm','z_mm']))
assert 24.5<r<34.5
print('Reference event 0: 511 keV deposited, 29002 optical photons, 1099 accepted hits, 303 hit sensors.')
print('Primary gamma history:', ' -> '.join(r['process'] for r in primary))
print('Optical acceptance fraction: {:.2%}'.format(1099/29002))
print('Expected scintillation mean for 511 keV:',511*57)
print('First interaction radius (mm):',r)
print('All data checks passed for ten events; this verifies accounting, not calibrated physical performance.')
