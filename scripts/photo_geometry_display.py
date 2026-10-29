#!/usr/bin/env python3
"""Shade actual exported geometry, with a presentation-only hemisphere and cutaway."""
from pathlib import Path
import argparse
import csv
import json
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--input', type=Path, default=Path('results/run'))
parser.add_argument('--output', type=Path)
args = parser.parse_args()
INPUT = args.input
OUT = args.output if args.output is not None else INPUT/'figures' 
OUT.mkdir(exist_ok=True)
def rows(name):
    if not (INPUT/name).exists() and name in ('events.csv','sipm_hits.csv','gamma_steps.csv'):
        return []
    with (INPUT/name).open() as f:
        return list(csv.DictReader(f))
sensors=rows('sensor_positions.csv')
positions=np.array([[float(s[k]) for k in ('x_mm','y_mm','z_mm')] for s in sensors])
ids=[int(s['sd_name'].split('-')[-1]) for s in sensors]
centers=np.array([positions[i:i+16].mean(0) for i in range(0,len(positions),16)])
directions=centers/np.linalg.norm(centers,axis=1)[:,None]
front=np.array([0.5,-0.8660254,0.])
hidden=(directions@front>0.18)&(directions[:,2]<0.80)
vertices=[];groups={};name=''
for line in (INPUT/'geometry.obj').read_text().splitlines():
    f=line.split()
    if not f:continue
    if f[0]=='o':name=f[1];groups.setdefault(name,[])
    elif f[0]=='v':vertices.append(list(map(float,f[1:4])))
    elif f[0]=='f':groups[name].append(np.array([vertices[int(i)-1] for i in f[1:]]))

def clip_upper(face):
    result=[]
    for i,p in enumerate(face):
        q=face[(i+1)%len(face)]
        a,b=p[2]>=-1e-7,q[2]>=-1e-7
        if a:result.append(p)
        if a!=b:
            t=-p[2]/(q[2]-p[2]);r=p+t*(q-p);r[2]=0;result.append(r)
    return np.array(result)

def clip_cutaway(face):
    plane=front-np.array([0.,0.,0.65])
    result=[]
    for i,p in enumerate(face):
        q=face[(i+1)%len(face)]
        dp,dq=float(p@plane),float(q@plane)
        inside_p,inside_q=dp<=1e-8,dq<=1e-8
        if inside_p:result.append(p)
        if inside_p!=inside_q:result.append(p+(q-p)*dp/(dp-dq))
    return np.array(result)

def triangulate(faces, smooth=False):
    result=[]
    for face in faces:
        face=clip_upper(face)
        if len(face)<3:continue
        for j in range(1,len(face)-1):
            tri=np.array([face[0],face[j],face[j+1]])
            n=np.cross(tri[1]-tri[0],tri[2]-tri[0]);length=np.linalg.norm(n)
            if length<1e-9:continue
            n/=length
            r=np.linalg.norm(tri,axis=1)
            is_sphere=(r.max()-r.min()<0.03) and abs(n[2])<.99999
            if smooth and is_sphere:
                normals=tri/r[:,None]
                if r.mean()<30:normals=-normals
            else:normals=np.tile(n,(3,1))
            result.append({'v':np.round(tri,6).tolist(),'n':np.round(normals,6).tolist()})
    return result
scint=triangulate(groups['hemisphereNorthScint_PV'],smooth=True)
# The guide/scintillator contact surface is concealed by the opaque shell.
# Omit that coincident interface to avoid mesh z-fighting in the display.
guide_faces=[f for f in groups['chapeauPoofMinusHomeOrb_PV']
             if not np.all(np.abs(np.linalg.norm(f,axis=1)-34.5)<0.002)]
guide=triangulate(guide_faces)
cutguide=triangulate([clip_cutaway(f) for f in guide_faces])
for t in guide:
    c=np.mean(t['v'],axis=0);idx=int(np.argmax(directions@c))
    t['cut']=bool(hidden[idx])
housing=triangulate(groups['g4sipmHousingPv'])
for t in housing:
    idx=int(np.argmax(directions@np.mean(t['v'],axis=0)))
    t['cut']=bool(hidden[idx])
payload={'scint':scint,'guide':guide,'cutguide':cutguide,'housing':housing,'sensors':[], 'events':[]}
for sensor_id,point in zip(ids,positions):
    if point[2]>=-0.001:payload['sensors'].append([sensor_id,*point.tolist()])
hits=rows('sipm_hits.csv');steps=rows('gamma_steps.csv')
for e in rows('events.csv'):
    i=int(e['event'])
    payload['events'].append({'id':i,'edep':float(e['edep_lyso_keV']),'photons':int(e['optical_created']),
        'hits':[[int(h['sipm_id']),float(h['time_ns'])] for h in hits if int(h['event'])==i],
        'steps':[[*[float(s[k]) for k in ['x0_mm','y0_mm','z0_mm','x1_mm','y1_mm','z1_mm']],s['process']] for s in steps if int(s['event'])==i]})
(OUT/'photo_display_geometry.json').write_text(json.dumps(payload,separators=(',',':')))

# Software rasterization: actual exported triangles, smooth scintillator shading,
# depth-tested translucent guide. No model dimensions are inferred from the photo.
W,H=1500,1350
scale=2
rw,rh=W*scale,H*scale
image=np.ones((rh,rw,3),dtype=np.float32)*np.array([0.965,0.958,0.945])
y,x=np.mgrid[:rh,:rw]
shadow=np.exp(-(((x-rw*.50)/(rw*.27))**2+((y-rh*.79)/(rh*.040))**2)*2)*.20
image-=shadow[:,:,None]
depth=np.full((rh,rw),np.inf,dtype=np.float32)
eye=np.array([88.,-152.,110.]);target=np.array([0.,0.,15.])
forward=(target-eye);forward/=np.linalg.norm(forward)
right=np.cross(forward,[0.,0.,1.]);right/=np.linalg.norm(right)
up=np.cross(right,forward)
basis=np.array([right,up,forward]);focal=rw*1.84
light=np.array([-0.4,-0.55,0.75]);light/=np.linalg.norm(light)

def project(points):
    camera=(points-eye)@basis.T
    return np.column_stack([rw*.5+focal*camera[:,0]/camera[:,2],rh*.50-focal*camera[:,1]/camera[:,2],camera[:,2]])

def raster(t,glass=False):
    verts=np.array(t['v']);normals=np.array(t['n']);pp=project(verts)
    xmin=max(0,int(np.floor(pp[:,0].min())));xmax=min(rw-1,int(np.ceil(pp[:,0].max())))
    ymin=max(0,int(np.floor(pp[:,1].min())));ymax=min(rh-1,int(np.ceil(pp[:,1].max())))
    if xmin>xmax or ymin>ymax:return
    xx,yy=np.meshgrid(np.arange(xmin,xmax+1)+.5,np.arange(ymin,ymax+1)+.5)
    x0,y0=pp[0,:2];x1,y1=pp[1,:2];x2,y2=pp[2,:2]
    den=(y1-y2)*(x0-x2)+(x2-x1)*(y0-y2)
    if abs(den)<1e-8:return
    b0=((y1-y2)*(xx-x2)+(x2-x1)*(yy-y2))/den
    b1=((y2-y0)*(xx-x2)+(x0-x2)*(yy-y2))/den
    b2=1-b0-b1
    invz=b0/pp[0,2]+b1/pp[1,2]+b2/pp[2,2]
    zz=1/invz
    existing=depth[ymin:ymax+1,xmin:xmax+1]
    mask=(b0>=-1e-7)&(b1>=-1e-7)&(b2>=-1e-7)&(zz<existing-1e-5)
    if not mask.any():return
    bary=np.stack([b0[mask]/pp[0,2],b1[mask]/pp[1,2],b2[mask]/pp[2,2]],axis=-1)/invz[mask,None]
    world=bary@verts;norm=bary@normals;norm/=np.linalg.norm(norm,axis=1)[:,None]
    view=eye-world;view/=np.linalg.norm(view,axis=1)[:,None]
    # Two-sided display of cut solids.
    norm*=np.where(np.sum(norm*view,axis=1)>=0,1.,-1.)[:,None]
    diffuse=np.maximum(norm@light,0)
    half=view+light;half/=np.linalg.norm(half,axis=1)[:,None]
    spec=np.maximum(np.sum(norm*half,axis=1),0)**(90 if glass else 65)
    fresnel=(1-np.maximum(np.sum(norm*view,axis=1),0))**3
    patch=image[ymin:ymax+1,xmin:xmax+1]
    if glass:
        alpha=np.clip(.16+.38*fresnel+.3*spec,.12,.7)[:,None]
        color=np.clip(np.array([.84,.91,.93])[None,:]*(.55+.45*diffuse[:,None])+.6*spec[:,None],0,1)
        patch[mask]=patch[mask]*(1-alpha)+color*alpha
    else:
        base=np.array([.43,.055,.64])
        color=base[None,:]*(.35+.66*diffuse[:,None])+.47*spec[:,None]+fresnel[:,None]*np.array([.15,.025,.12])
        patch[mask]=np.clip(color,0,1);existing[mask]=zz[mask]

for t in scint:raster(t)
visible=list(cutguide)
visible.sort(key=lambda t:float(np.mean(project(np.array(t['v']))[:,2])),reverse=True)
for t in visible:raster(t,True)
img=Image.fromarray(np.uint8(np.clip(image,0,1)*255)).resize((W,H),Image.Resampling.LANCZOS if hasattr(Image,'Resampling') else Image.LANCZOS)
draw=ImageDraw.Draw(img)
from matplotlib import font_manager
fontpath=font_manager.findfont('DejaVu Sans')
font=lambda size:ImageFont.truetype(fontpath,size)
def center(text,y,size,color='#263141'):
    f=font(size);box=draw.textbbox((0,0),text,font=f);draw.text(((W-(box[2]-box[0]))/2,y),text,font=f,fill=color)
center('Spherical PET — hemispherical model',42,39)
center('Purple scintillator · clear faceted light guide · demonstration cutaway',102,23,'#606975')
center('Actual exported geometry; upper hemisphere shown',1182,25)
center('Guide sections are hidden for presentation. Simulation geometry is unchanged.',1224,20,'#606975')
center('Supplied model: pentagonal and hexagonal guide sections.',1260,20,'#606975')
img.save(OUT/'photo_style_detector.png')
img.save(OUT/'photo_style_detector.pdf',resolution=150)
# Embed data so the viewer needs no server or external libraries.
template=(ROOT/'scripts/display_assets/photo_viewer.html').read_text()
(OUT/'photo_style_event_display.html').write_text(template.replace('__GEOMETRY_DATA__',json.dumps(payload,separators=(',',':'))))
print('Rendered:',len(scint),'scintillator triangles;',len(visible),'visible guide triangles.')
print('Upper hemisphere sensor centers:',len(payload['sensors']))
