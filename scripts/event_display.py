#!/usr/bin/env python3
"""Event display from saved gamma steps and SiPM hits; no inferred photon paths."""
from pathlib import Path
import csv
import json
import argparse
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from mpl_toolkits.mplot3d.art3d import Line3DCollection

p = argparse.ArgumentParser()
p.add_argument('--input', type=Path, default=Path('results/run'))
p.add_argument('--output', type=Path, default=None)
p.add_argument('--event', type=int, default=0)
a = p.parse_args()
if a.output is None: a.output = a.input / "figures"
a.output.mkdir(parents=True, exist_ok=True)
def read(name):
    with (a.input/name).open() as f:
        return list(csv.DictReader(f))
events = read('events.csv')
hits = read('sipm_hits.csv')
steps = read('gamma_steps.csv')
sensors = read('sensor_positions.csv')
xyz = np.array([[float(s[k]) for k in ('x_mm','y_mm','z_mm')] for s in sensors])
ids = np.array([int(s['sd_name'].split('-')[-1]) for s in sensors])
vertices, edges, group = [], [], ''
for line in (a.input/'geometry.obj').read_text().splitlines():
    v = line.split()
    if not v: continue
    if v[0] == 'o': group = v[1]
    elif v[0] == 'v': vertices.append([float(x) for x in v[1:4]])
    elif v[0] == 'f' and group == 'chapeauPoofMinusHomeOrb_PV':
        face = np.array([vertices[int(x)-1] for x in v[1:]])
        if face[:,1].mean() >= 0:
            edges.extend([[face[i].tolist(),face[(i+1)%len(face)].tolist()] for i in range(len(face))])
# Deduplicate the wireframe's shared facet edges.
unique = {}
for edge in edges:
    key = tuple(sorted(tuple(round(x,4) for x in point) for point in edge))
    unique[key] = edge
edges = list(unique.values())
e = next(row for row in events if int(row['event']) == a.event)
h = [row for row in hits if int(row['event']) == a.event]
g = [row for row in steps if int(row['event']) == a.event]
assert len(h) == int(e['sipm_accepted_hits'])
counts_all = np.bincount([int(row['sipm_id']) for row in h], minlength=max(ids)+1)
counts = counts_all[ids]
active = counts > 0
vmax = max(2,int(counts.max()))
norm = LogNorm(vmin=1,vmax=vmax)
plt.rcParams.update({'font.size':11,'axes.spines.top':False,'axes.spines.right':False})
fig = plt.figure(figsize=(14,8),facecolor='white')
gs = fig.add_gridspec(2,2,width_ratios=[1.4,1],left=0.025,right=0.96,bottom=0.13,top=0.82,wspace=0.16,hspace=0.38)
ax = fig.add_subplot(gs[:,0],projection='3d')
ax.add_collection3d(Line3DCollection(edges,colors='#7cc2ce',linewidths=0.35,alpha=0.3))
ax.scatter(*xyz[~active].T,color='#d6dbe1',s=7,alpha=0.45,depthshade=False)
sc = ax.scatter(*xyz[active].T,c=counts[active],cmap='viridis',norm=norm,s=30,depthshade=False)
for row in g:
    p0 = [float(row[k]) for k in ('x0_mm','y0_mm','z0_mm')]
    p1 = [float(row[k]) for k in ('x1_mm','y1_mm','z1_mm')]
    ax.plot(*np.array([p0,p1]).T,color='#eb492f',lw=3,zorder=20)
    if row['process'] != 'Transportation':
        ax.scatter(*p1,color='#eb492f',marker='*',s=100,depthshade=False)
ax.scatter(0,0,0,color='black',s=32,depthshade=False)
ax.text(1,-2,-4,'Source',fontsize=10)
ax.set(xlim=(-45,45),ylim=(-45,45),zlim=(-45,45),xlabel='x [mm]',ylabel='y [mm]',zlabel='z [mm]')
ax.set_box_aspect((1,1,1));ax.view_init(elev=18,azim=-65)
ax.set_title('Sensor response and gamma trajectory',pad=14)
cbar = fig.colorbar(sc,ax=ax,shrink=0.55,pad=0.02)
cbar.set_label('Accepted hits / SiPM (log scale)')
proj = fig.add_subplot(gs[0,1])
settings = {}
config_path = a.input / "resolved.conf"
if config_path.exists():
    for line in config_path.read_text().splitlines():
        if "=" in line and not line.lstrip().startswith("#"):
            key, value = line.split("=", 1); settings[key.strip()] = value.strip()
for radius in (34.5-float(settings.get("shell-thickness-mm",10)),34.5):
    proj.add_patch(plt.Circle((0,0),radius,fill=False,color='#a16cba',lw=1.2,ls='--'))
proj.scatter(xyz[~active,0],xyz[~active,2],s=4,color='#d6dbe1')
proj.scatter(xyz[active,0],xyz[active,2],s=12,c=counts[active],norm=norm,cmap='viridis')
for row in g:
    proj.plot([float(row['x0_mm']),float(row['x1_mm'])],[float(row['z0_mm']),float(row['z1_mm'])],color='#eb492f',lw=2)
    if row['process'] != 'Transportation':
        proj.scatter(float(row['x1_mm']),float(row['z1_mm']),color='#eb492f',marker='*',s=85,zorder=10)
proj.scatter(0,0,color='black',s=20,zorder=11)
proj.set(xlim=(-44,44),ylim=(-44,44),xlabel='x [mm]',ylabel='z [mm]',title='x–z projection; dashed circles: scintillator radii')
proj.set_aspect('equal')
timing = fig.add_subplot(gs[1,1])
times = [float(row['time_ns']) for row in h]
timing.hist(times,bins=45,color='#278487')
timing.set(xlabel='Photon arrival time from event origin [ns]',ylabel='Accepted hits',title='SiPM photon arrival times')
fig.suptitle(f"Spherical PET — event {a.event}",fontsize=22,weight='bold',y=0.98)
fig.text(0.5,0.91,f"{float(e.get('edep_scintillator_keV', e.get('edep_lyso_keV'))):g} keV deposited in scintillator  •  "
         f"{int(e['optical_created']):,} optical photons  •  {len(h):,} accepted hits",ha='center',fontsize=12)
fig.text(0.5,0.06,'Red: saved gamma path and interaction points. Colored dots: hit counts at sensor centers. Gray: sensors with no accepted hits.',ha='center',fontsize=10)
fig.text(0.5,0.03,'Rear guide wireframe shown for visibility. Optical-photon paths are not drawn. Provisional optical and SiPM models.',ha='center',fontsize=10,color='#555555')
fig.savefig(a.output/f'event_{a.event:03d}.png',dpi=170)
fig.savefig(a.output/f'event_{a.event:03d}.pdf')
plt.close(fig)
# A dependency-free interactive viewer: actual 3D positions, event selection, and arrival-time gate.
payload={'sensors':[[int(i),*v.tolist()] for i,v in zip(ids,xyz)],'edges':edges,'events':[]}
for row in events:
    event_id = int(row['event'])
    event_hits = [[int(r['sipm_id']),float(r['time_ns'])] for r in hits if int(r['event'])==event_id]
    event_steps = [[*[float(r[k]) for k in ('x0_mm','y0_mm','z0_mm','x1_mm','y1_mm','z1_mm')],r['process']] for r in steps if int(r['event'])==event_id]
    payload['events'].append({'id':event_id,'energy':float(row.get('edep_scintillator_keV', row.get('edep_lyso_keV'))),'photons':int(row['optical_created']),'hits':event_hits,'steps':event_steps})
html='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Spherical PET event display</title>
<style>body{margin:0;background:#101823;color:#eef4fc;font:16px system-ui}main{max-width:1200px;margin:auto;padding:24px}h1{margin:0 0 10px}p{color:#b5c5d8;line-height:1.5}.controls{display:flex;gap:22px;flex-wrap:wrap;align-items:center;background:#1b293b;padding:16px;border-radius:12px}select,button{font:inherit;padding:6px;border-radius:5px}input{width:240px;vertical-align:middle}canvas{width:100%;height:640px;touch-action:none;display:block;background:#0c1420;border-radius:12px;margin-top:16px}.key{display:flex;gap:20px;flex-wrap:wrap;font-size:14px}.bar{display:inline-block;width:100px;height:10px;background:linear-gradient(90deg,hsl(240,85%,58%),hsl(0,85%,58%))}#stats{font-size:18px;color:white}</style>
<main><h1>Spherical PET event display</h1><p>Drag to rotate · Scroll to zoom · Select an event or restrict photon arrival time.</p>
<div class="controls"><label>Event <select id="event"></select></label><label>Arrival time ≤ <input id="gate" type="range" min="0" step="0.1"><span id="time"></span> ns</label><button id="reset">Reset view</button></div><p id="stats"></p><canvas id="canvas" aria-label="Rotatable 3D gamma trajectory and sensor response"></canvas>
<div class="key"><span style="color:#ff674e">● Saved gamma trajectory</span><span>● Source at origin</span><span><span class="bar"></span> 1 → <span id="peak"></span> hits / SiPM (log)</span><span style="color:#8195aa">● No accepted hits within gate</span></div>
<p>All sensor dots use their saved center coordinates. The rear light-guide wireframe is shown for visibility. The complete gamma trajectory stays visible at every arrival-time setting. Full reflected optical-photon paths were not saved; no paths are inferred from hit endpoints. The run uses provisional optical properties and a reference SiPM model.</p></main>
<script>const DATA=__DATA__;
const canvas=document.getElementById('canvas'),ctx=canvas.getContext('2d'),select=document.getElementById('event'),gate=document.getElementById('gate');
let yaw=-0.35,pitch=0.3,zoom=1,drag=null;
DATA.events.forEach((e,i)=>{const o=document.createElement('option');o.value=i;o.textContent=e.id;select.appendChild(o)});
function current(){return DATA.events[+select.value]}
function change(){const e=current();gate.max=Math.ceil(Math.max(1,...e.hits.map(h=>h[1])));gate.value=gate.max;draw()}
function project(p){let x=Math.cos(yaw)*p[0]-Math.sin(yaw)*p[1],y=Math.sin(yaw)*p[0]+Math.cos(yaw)*p[1],z=p[2];const v=Math.cos(pitch)*z-Math.sin(pitch)*y,d=Math.sin(pitch)*z+Math.cos(pitch)*y;const s=Math.min(canvas.clientWidth,canvas.clientHeight)/108*zoom;return [canvas.clientWidth/2+x*s,canvas.clientHeight/2-v*s,d]}
function line(a,b,color,width){const x=project(a),y=project(b);ctx.beginPath();ctx.moveTo(x[0],x[1]);ctx.lineTo(y[0],y[1]);ctx.strokeStyle=color;ctx.lineWidth=width;ctx.stroke()}
function dot(p,color,r){const x=project(p);ctx.beginPath();ctx.arc(x[0],x[1],r,0,Math.PI*2);ctx.fillStyle=color;ctx.fill()}
function draw(){const w=canvas.clientWidth,h=canvas.clientHeight,dpr=window.devicePixelRatio||1;canvas.width=w*dpr;canvas.height=h*dpr;ctx.setTransform(dpr,0,0,dpr,0,0);ctx.clearRect(0,0,w,h);const e=current(),counts={},all={};let n=0;for(const [id,t] of e.hits){all[id]=(all[id]||0)+1;if(t<=+gate.value){counts[id]=(counts[id]||0)+1;n++}}const peak=Math.max(2,...Object.values(all));document.getElementById('peak').textContent=peak;document.getElementById('time').textContent=(+gate.value).toFixed(1);document.getElementById('stats').textContent=`Event ${e.id} · ${e.energy} keV in scintillator · ${e.photons.toLocaleString()} optical photons · ${n.toLocaleString()} / ${e.hits.length.toLocaleString()} accepted hits within gate`;
for(const [a,b] of DATA.edges)line(a,b,'#284d61',0.65);
const sorted=DATA.sensors.map(s=>({id:s[0],p:s.slice(1),depth:project(s.slice(1))[2]})).sort((a,b)=>b.depth-a.depth);
for(const s of sorted){const v=counts[s.id]||0;const color=v?`hsl(${240*(1-Math.log(v)/Math.log(peak))},85%,58%)`:'#3d4e60';dot(s.p,color,v?4.1:2.1)}
for(const s of e.steps){line(s.slice(0,3),s.slice(3,6),'#ff674e',2.8);if(s[6]!=='Transportation')dot(s.slice(3,6),'#ff674e',6)}dot([0,0,0],'#ffffff',4);
for(const [end,label,color] of [[[12,0,0],'x','#ff9d9d'],[[0,12,0],'y','#9ddda5'],[[0,0,12],'z','#9dbaff']]){line([0,0,0],end,color,1);const p=project(end);ctx.fillStyle=color;ctx.font='14px system-ui';ctx.fillText(label,p[0]+5,p[1])}ctx.fillStyle='#9fb1c8';ctx.font='14px system-ui';ctx.fillText('Coordinates in mm · guide outlet radius 39.5 mm',18,h-20)}
canvas.addEventListener('pointerdown',e=>{drag=[e.clientX,e.clientY];canvas.setPointerCapture(e.pointerId)});canvas.addEventListener('pointermove',e=>{if(!drag)return;yaw+=(e.clientX-drag[0])*0.008;pitch=Math.max(-1.5,Math.min(1.5,pitch+(e.clientY-drag[1])*0.008));drag=[e.clientX,e.clientY];draw()});canvas.addEventListener('pointerup',()=>drag=null);canvas.addEventListener('pointercancel',()=>drag=null);canvas.addEventListener('wheel',e=>{e.preventDefault();zoom=Math.min(3,Math.max(.45,zoom*Math.exp(-e.deltaY*.001)));draw()},{passive:false});select.onchange=change;gate.oninput=draw;document.getElementById('reset').onclick=()=>{yaw=-.35;pitch=.3;zoom=1;draw()};window.addEventListener('resize',draw);change();</script></html>'''
(a.output/'event_display.html').write_text(html.replace('__DATA__',json.dumps(payload,separators=(',',':'))))
print(f'Created event {a.event} display: {len(h)} accepted hits; {len(g)} gamma steps; {active.sum()} active sensors.')
print('Created offline interactive viewer for all',len(events),'events.')
