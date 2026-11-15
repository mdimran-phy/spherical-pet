import os, subprocess, time, json
from pathlib import Path

project = Path('/work/eic3/users/ihossain/Medical_imaging/spherical-pet')
out = project/'docs/presentations/beginner_20261115'
assets = out/'assets'
assets.mkdir(parents=True, exist_ok=True)
env = os.environ.copy()
for key in ('G4UI_USE_QT', 'G4UI_USE_XM', 'G4UI_USE_WIN32'):
    env.pop(key, None)
env['G4UI_USE_TCSH'] = '1'
env['LIBGL_ALWAYS_SOFTWARE'] = '1'
for label, limit, mode in [('geometry',0,1),('gamma',0,0),('five',5,0),('hundred',100,0)]:
    run = project/'results'/f'presentation_native_20261115_{label}'
    if run.exists():
        raise RuntimeError(f'Refusing to replace existing run {run}')
    log = out/f'capture_{label}.log'
    args = [str(project/'build/spherical-pet'), '--config', 'config/ray_baseline.conf',
            '-b','0','-e','1','-seed','42','--vis-photons',str(limit),'-r',str(mode),
            '--export-geometry','0','-o',str(run)]
    with log.open('w') as f:
        proc = subprocess.Popen(args,cwd=project,env=env,stdin=subprocess.PIPE,stdout=f,stderr=subprocess.STDOUT,text=True)
        try:
            ready = 'Drawing up to' if mode else 'PET_RESULT'
            start=time.monotonic()
            while ready not in log.read_text(errors='replace'):
                if proc.poll() is not None or time.monotonic()-start>45:
                    raise RuntimeError(log.read_text()[-4000:])
                time.sleep(.25)
            time.sleep(1.5)
            subprocess.run(['import','-window','root',str(assets/f'native_{label}.png')],check=True,timeout=15)
            proc.communicate('exit\n',timeout=15)
            if proc.returncode:
                raise RuntimeError(f'Viewer exit {proc.returncode}')
        finally:
            if proc.poll() is None:
                proc.terminate()
                proc.wait(timeout=10)
    if not mode:
        d=json.loads((run/'summary.json').read_text())
        assert d['sipm_accepted_hits']==1099 and d['optical_created']==29002
    print('Captured',label,flush=True)
