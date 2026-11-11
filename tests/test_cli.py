#!/usr/bin/env python3
"""Exercise parameter validation without initializing Geant4 transport."""
import os
import subprocess
import sys
import tempfile
from pathlib import Path
exe = str(Path(sys.argv[1]).resolve())
def run(*args, ok=True):
    p = subprocess.run([exe,*map(str,args)],text=True,capture_output=True)
    assert (p.returncode == 0) == ok, (args,p.returncode,p.stdout,p.stderr)
    return p
assert '--shell-thickness-mm' in run('--help').stdout
assert '0.1.0' in run('--version').stdout
for args in [('-e','-1'),('--energy-kev','nan'),('--seed','0'),('--theta','181'),('--cell-filter','2'),('--typo','1'),('--energy-kev',),('--particle','proton'),('--source-z-mm','100')]:
    run(*args,ok=False)
with tempfile.TemporaryDirectory() as temp:
    root=Path(temp)
    config=root/'settings.conf';config.write_text('events = 4\nenergy-kev = 200\n')
    result=run('-e','2','--config',config,'--print-config').stdout
    assert 'events = 2\n' in result and 'energy-kev = 200\n' in result
    spectrum=root/'spectrum.csv';spectrum.write_text('energy_eV,intensity\n2.7,0\n2.8,1\n2.9,0\n')
    config.write_text('spectrum = spectrum.csv\n')
    assert str(spectrum) in run('--config',config,'--print-config').stdout
    spectrum.write_text('2.8,1\n2.7,1\n')
    run('--spectrum',spectrum,'--print-config',ok=False)
    macro=root/'bad.mac';macro.write_text('/run/beamOn 100\n')
    run('--macro',macro,'-o',root/'not-created',ok=False)
    assert not (root/'not-created').exists()
    # Fail before creating output when interactive mode has no display.
    env = os.environ.copy()
    env.pop('DISPLAY', None)
    env.pop('WAYLAND_DISPLAY', None)
    failure = subprocess.run([exe, '-b', '0', '-o', str(root/'no-display')],
                             env=env, text=True, capture_output=True, timeout=20)
    assert failure.returncode == 1 and not (root/'no-display').exists()
    assert ('graphical display' in failure.stderr or 'WITH_GEANT4_UIVIS' in failure.stderr)
    # Exercise detector-constructor failures and unwinding of its owned sensors.
    for label, args, message in [
        ('missing-model', ['--sipm-model', 'HamamatsuS141604050HS'], 'Unavailable SiPM model'),
        ('narrow-pitch', ['--sipm-pitch-mm', '2'], 'SiPM package does not fit'),
        ('wide-array', ['--sipm-pitch-mm', '6'], 'SiPM package does not fit')]:
        failure = run(*args, '-o', root/label, ok=False)
        assert failure.returncode == 1 and message in failure.stderr
    existing=root/'existing';existing.mkdir();(existing/'keep.txt').write_text('keep')
    run('-o',existing,ok=False)
    assert (existing/'keep.txt').read_text()=='keep'
print('PASS: help, aliases, validation, config precedence, spectrum loading, macro restrictions, output protection')
