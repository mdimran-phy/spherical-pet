#!/usr/bin/env python3
"""Run a real OpenGL viewer under Xvfb and verify display-only track sampling."""
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

exe = str(Path(sys.argv[1]).resolve())
env = os.environ.copy()
for key in ('G4UI_USE_QT', 'G4UI_USE_XM', 'G4UI_USE_WIN32'):
    env.pop(key, None)
env['G4UI_USE_TCSH'] = '1'  # Terminal command input, native OpenGL window.
env['LIBGL_ALWAYS_SOFTWARE'] = '1'
common = ['-e', '1', '-x', 'e-', '-E', '100', '--source-z-mm', '28',
          '--lyso-yield', '2', '--track-photons', '2', '--mesh-steps', '12', '-seed', '1234']
with tempfile.TemporaryDirectory() as temp:
    root = Path(temp)
    for label, batch, limit in [('batch', 1, 4), ('visible', 0, 4), ('hidden', 0, 0)]:
        run = subprocess.run([exe, *common, '-b', str(batch), '--vis-photons', str(limit),
                              '-o', str(root/label)], input='exit\n', text=True,
                             capture_output=True, env=env, timeout=45)
        assert run.returncode == 0, run.stdout + '\n' + run.stderr
        assert 'Geant4 command failed' not in run.stdout + run.stderr
        if not batch:
            match = re.search(r'PET_DISPLAY_EVENT event=0 trajectories=(\d+) optical_paths=(\d+) SiPM_hits=(\d+)', run.stdout)
            assert match, run.stdout
            tracks, optical, hits = map(int, match.groups())
            assert optical == limit and tracks > optical and hits > 0
            summary = json.loads((root/label/'summary.json').read_text())
            assert summary['optical_created'] > limit
    for label in ['visible', 'hidden']:
        for filename in ['events.csv', 'sipm_hits.csv', 'gamma_steps.csv', 'optical_steps.csv', 'summary.json']:
            assert (root/label/filename).read_bytes() == (root/'batch'/filename).read_bytes(), (label, filename)
print('PASS: native OpenGL viewer, limited/hidden optical paths, SiPM hits, clean exit, unchanged batch physics and CSVs')
