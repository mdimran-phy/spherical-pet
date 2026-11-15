#!/usr/bin/env python3
"""Render the exported Geant4 mesh and diagnostic event output."""
from pathlib import Path
import argparse
import csv
import json
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

parser = argparse.ArgumentParser()
parser.add_argument('--input', type=Path, default=Path('results/run'))
parser.add_argument('--output', type=Path, default=None)
args = parser.parse_args()
if args.output is None: args.output = args.input / "figures"
args.output.mkdir(parents=True, exist_ok=True)
vertices, groups, name = [], {}, ''
for line in (args.input / 'geometry.obj').read_text().splitlines():
    fields = line.split()
    if not fields:
        continue
    if fields[0] == 'o':
        name = fields[1]
        groups.setdefault(name, [])
    elif fields[0] == 'v':
        vertices.append([float(v) for v in fields[1:4]])
    elif fields[0] == 'f':
        groups[name].append(np.array([vertices[int(v) - 1] for v in fields[1:]]))

plt.rcParams.update({'font.size': 10, 'axes.spines.top': False,
                     'axes.spines.right': False})
fig = plt.figure(figsize=(13, 6.5))
for panel in (1, 2):
    ax = fig.add_subplot(1, 2, panel, projection='3d')
    for name, faces in groups.items():
        if 'Scint' in name:
            color, alpha = '#873caf', 0.82
        elif 'chapeauPoofMinusHomeOrb' in name:
            color, alpha = '#65c9dd', 0.19
        elif 'Housing' in name:
            color, alpha = '#1e6661', 0.95
        else:
            continue
        selected = faces if panel == 1 else [f for f in faces if f[:, 1].mean() >= 0]
        if selected:
            collection = Poly3DCollection(selected, facecolor=color, alpha=alpha,
                                          edgecolor=color, linewidths=0.12)
            ax.add_collection3d(collection)
    ax.set(xlim=(-45, 45), ylim=(-45, 45), zlim=(-45, 45),
           xlabel='x [mm]', ylabel='y [mm]', zlabel='z [mm]')
    ax.set_box_aspect((1, 1, 1))
    ax.view_init(elev=20, azim=-65)
    ax.set_title('Complete detector' if panel == 1 else 'Rear half shown to expose the cavity')
fig.suptitle('Spherical PET: actual Geant4 geometry', fontsize=17, weight='bold')
fig.legend(handles=[Patch(color='#873caf', label='scintillator shell'),
                    Patch(color='#65c9dd', label='Faceted light guide'),
                    Patch(color='#1e6661', label='512 reference SiPM housings')],
           loc='lower center', ncol=3, bbox_to_anchor=(0.5, 0.055), frameon=False)
fig.text(0.5, 0.025, 'Display omits gaskets and outer mask for visibility. Cutaway affects this drawing only.',
         ha='center', fontsize=9)
fig.subplots_adjust(top=0.88, bottom=0.15, left=0.02, right=0.97, wspace=0.05)
fig.savefig(args.output / 'detector_geometry.png', dpi=180)
fig.savefig(args.output / 'detector_geometry.pdf')
plt.close(fig)

events = np.genfromtxt(args.input / 'events.csv', delimiter=',', names=True)
hits = np.genfromtxt(args.input / 'sipm_hits.csv', delimiter=',', names=True)
events, hits = np.atleast_1d(events), np.atleast_1d(hits)
summary = json.loads((args.input / 'summary.json').read_text())
assert len(events) == summary['events']
assert len(hits) == summary['sipm_accepted_hits'] == int(events['sipm_accepted_hits'].sum())
assert int(events['optical_created'].sum()) == summary['optical_created']
assert sum(summary['optical_endings'].values()) == summary['optical_created']
assert len(set(zip(hits['event'], hits['track_id']))) == len(hits)
with (args.input / 'sensor_positions.csv').open() as f:
    sensors = list(csv.DictReader(f))
assert len(sensors) == 512
assert len({s['sd_name'] for s in sensors}) == 512
fig, axs = plt.subplots(2, 2, figsize=(11, 7.5), constrained_layout=True)
axs[0, 0].bar(events['event'], events['edep_scintillator_keV' if 'edep_scintillator_keV' in events.dtype.names else 'edep_lyso_keV'], color='#873caf')
axs[0, 0].set(xlabel='Event', ylabel='scintillator energy deposit [keV]', xticks=range(len(events)))
axs[0, 1].bar(events['event'] - 0.19, events['optical_created'], width=0.38,
               label='Optical photons created', color='#65b8cd')
axs[0, 1].bar(events['event'] + 0.19, events['sipm_accepted_hits'], width=0.38,
               label='Accepted SiPM hits', color='#1e6661')
axs[0, 1].set(xlabel='Event', ylabel='Count', xticks=range(len(events)))
axs[0, 1].legend(frameon=False, fontsize=8)
axs[1, 0].hist(hits['time_ns'], bins=60, color='#1e6661')
axs[1, 0].set(xlabel='Accepted photon arrival time [ns, event origin]', ylabel='Hits')
counts = np.bincount(hits['sipm_id'].astype(int) // 16, minlength=32)
axs[1, 1].bar(np.arange(32), counts, color='#65b8cd')
axs[1, 1].set(xlabel='Array index (16 SiPMs per array)', ylabel='Accepted hits', xlim=(-1, 32))
fig.suptitle(f"Diagnostic run: {len(events)} primary events\n"
             f"{summary['optical_created']:,} optical photons; {len(hits):,} accepted SiPM hits\n"
             'Provisional spectrum and sensor model; no electronic timing response', fontsize=13)
fig.savefig(args.output / 'smoke_results.png', dpi=180)
fig.savefig(args.output / 'smoke_results.pdf')
print('PASS: event totals, photon endings, unique photon hits, and 512 sensor IDs agree.')
print('Hit-bearing arrays:', int(np.count_nonzero(counts)))
