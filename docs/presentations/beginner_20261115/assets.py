"""Scientific diagrams and measured-event plots; no generated imagery."""
from pathlib import Path
import csv, json, math, shutil, textwrap
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, Rectangle, FancyBboxPatch, Polygon, Wedge, Ellipse
from matplotlib.colors import LogNorm
from PIL import Image
from content import PROJECT, SLIDES

OUT=Path(__file__).resolve().parent
ASSETS=OUT/'assets'
RUN=OUT/'data'
ASSETS.mkdir(parents=True,exist_ok=True)
C={'ink':'#172c48','muted':'#526880','blue':'#2878c8','cyan':'#00a9bb','green':'#27946b','gold':'#e5aa35','red':'#d64a4f','purple':'#8655b9','pale':'#edf4fa','white':'#ffffff'}
plt.rcParams.update({'font.family':'DejaVu Sans','font.size':14,'axes.labelcolor':C['ink'],'text.color':C['ink'],'axes.spines.top':False,'axes.spines.right':False,'axes.edgecolor':'#b5c3d1','xtick.color':C['muted'],'ytick.color':C['muted'],'savefig.facecolor':'white'})
def rows(name):
    with (RUN/name).open() as f: return list(csv.DictReader(f))
EVENTS=rows('events.csv'); HITS=[r for r in rows('sipm_hits.csv') if r['event']=='0']
HISTORY=[r for r in rows('gamma_interactions.csv') if r['event']=='0' and r['parent_id']=='0']
def base():
    fig,ax=plt.subplots(figsize=(9,5.5)); ax.set(xlim=(0,10),ylim=(0,6));ax.axis('off'); return fig,ax
def txt(ax,x,y,s,size=16,color=None,ha='center',weight='normal',**kw):
    ax.text(x,y,s,fontsize=size,color=color or C['ink'],ha=ha,va='center',weight=weight,**kw)
def box(ax,x,y,w,h,s,color='blue',size=16):
    ax.add_patch(FancyBboxPatch((x,y),w,h,boxstyle='round,pad=0.05,rounding_size=.12',facecolor=C.get(color,color),edgecolor='none'))
    txt(ax,x+w/2,y+h/2,s,size,'white')
def arr(ax,a,b,color='ink',lw=2.5):
    ax.annotate('',xy=b,xytext=a,arrowprops={'arrowstyle':'-|>','color':C.get(color,color),'lw':lw,'mutation_scale':16})
def caption(ax,s):txt(ax,5,.16,s,11,C['muted'])
def flow(labels,kind='SCHEMATIC • read in arrow order'):
    fig,ax=base(); n=len(labels)
    if n<=4:
        w=8.8/n-.25
        for i,label in enumerate(labels):
            x=.6+i*(w+.3);box(ax,x,2.05,w,1.9,'\n'.join(textwrap.fill(part,max(9,int(w*5.5))) for part in label.splitlines()),['blue','cyan','green','purple'][i],15)
            if i<n-1:arr(ax,(x+w+.03,3),(x+w+.28,3))
    else:
        w=2.7
        for i,label in enumerate(labels):
            row=i//3; col=i%3 if row==0 else 2-i%3
            x=.55+col*3.1; y=3.5 if row==0 else .95
            box(ax,x,y,w,1.5,'\n'.join(textwrap.fill(part,16) for part in label.splitlines()),['blue','cyan','green','purple','blue','green'][i%6],15)
            if i<n-1:
                if i==2:arr(ax,(x+w/2,y-.1),(x+w/2,y-1.0))
                elif row==0:arr(ax,(x+w+.02,y+.75),(x+w+.36,y+.75))
                else:arr(ax,(x-.05,y+.75),(x-.38,y+.75))
    caption(ax,kind);return fig,ax

FLOWS={
'course_map':['Basic ideas','Build the detector','Follow an event','Read code & data','Test understanding','Next research steps'],
'pipeline':['Primary gamma','Gamma interactions','Electron energy deposits','Scintillation light','Optical transport','Accepted sensor hits'],
'model_flow':['Geometry + materials','Physics processes','Random sampling','Saved observations'],
'object_flow':['Geant4 calls action','GeneratePrimaries(event)','GPS adds primary'],
'material_flow':['Composition + density','Gamma response','Optical properties','Light response'],
'volume_flow':['SOLID\nshape','LOGICAL VOLUME\nmaterial','PHYSICAL VOLUME\nplacement'],
'hierarchy':['RUN\nmany events','EVENT\nparticle family','TRACK\none particle','STEP\none segment'],
'electron_steps':['Gamma transfers energy','Electron travels','Energy deposits','Crystal emits light'],
'energy_ledger':['Primary energy','Non-optical deposits','Scintillation response','Do not add optical energy again'],
'light_spread':['Interaction location','Light propagation','512-channel pattern','Position information'],
'sipm_avalanche':['Photon in silicon','Avalanche in a cell','Quench & recover','Shared sensor output'],
'sensor_filter':['Photon reaches sensor','Active-region filter','Probability filter','Accepted hit'],
'hit_record':['Event + track ID','Sensor ID','Time + energy','Arrival position'],
'photon_fates':['Created photons','Transport','Absorb / escape / end','Reconcile totals'],
'files':['Config.cc\nsettings','DetectorConstruction.cc\ngeometry','main.cc\nactions','CSV + JSON\nresults'],
'startup':['Read config','Build detector','Register physics','Register actions','Initialize','BeamOn(events)'],
'physics_list':['Livermore EM','Optical physics','Decay physics','Geant4 transport'],
'run_flow':['Begin run','Open output files','Simulate events','Write summary'],
'event_flow':['Begin event','Reset counters','Track descendants','Collect sensor hits','Write 512 counts','Write event row'],
'tracking_flow':['New track','Identify particle','Count optical creation','Select diagnostic storage'],
'stepping_flow':['Pre-step material','Step energy deposit','Post-step process','Record selected data'],
'counts_flow':['512 zeros','Accepted photon','Increment sensor ID','512 final counts'],
'config_flow':['Defaults','Config file','CLI overrides','Saved resolved settings'],
'terminal':['Project directory','Geant4 environment','Native GUI libraries','Run executable'],
'command_map':['Executable','Configuration','Source + display options','New output path'],
'output_path':['Choose new path','Run simulation','Keep previous data','Inspect new output'],
'viewer':['Rotate / zoom','Find an interface','Review kept events','continue to advance'],
'output_files':['events.csv\nevent totals','sipm_hits.csv\naccepted photons','sipm_counts.csv\n512 channels','gamma_interactions.csv\nhistory'],
'csv':['Open CSV','Read named columns','Select event 0','Convert text to numbers'],
'counts_check':['512 channel rows','Sum photon_count','Count accepted hit rows','Compare event total'],
'family_tree':['Primary gamma\ntrack 1, parent 0','Primary interaction','Secondary gamma\nparent 1','Separate history'],
'validation_ladder':['Correct implementation','Numerical checks','Measured inputs','Physical comparison'],
'paper_cnn':['AnnPET simulation','Sensor patterns','Known positions','Train and test'],
'cnn':['512 channel inputs','Geometry representation','Learn from labels','Independent test'],
'labels':['Gamma history','Choose target definition','Select appropriate sites','Preserve full truth'],
'future_flow':['Annihilation pair','Detector responses','Coincidence selection','Position estimation','Lines of response','Reconstructed image'],
'references':['Source snapshot','Saved event data','Official documentation','Two research papers'],
}

def make(key):
    if key in FLOWS:return flow(FLOWS[key])
    fig,ax=base()
    if key in ['hero','cross_section','source','first_interaction','quiz_radius','coordinates','cavity_crossing']:
        ax.set_aspect('equal');cx,cy=4.1,3.15
        ax.add_patch(Circle((cx,cy),2.35,fc='#e7eff8',ec=C['blue'],lw=2))
        ax.add_patch(Circle((cx,cy),1.67,fc='white',ec=C['blue'],lw=2))
        txt(ax,cx,cy-.65,'AIR',15);txt(ax,cx-1.97,cy,'GAGG',13,rotation=90)
        ax.plot(cx,cy,'o',color=C['red']);arr(ax,(cx,cy),(cx,cy+2.1),'red',3)
        txt(ax,7.5,4.6,'Outer radius\n34.5 mm',16);arr(ax,(6.8,4.4),(6.0,4.3),'muted',1.5)
        txt(ax,7.5,2.5,'Inner radius\n24.5 mm',16);arr(ax,(6.5,2.5),(5.7,2.8),'muted',1.5)
        txt(ax,7.5,1.0,'Shell thickness\n10 mm',16)
        if key in ['first_interaction','hero']:
            ax.plot(cx,cy+2.1,'o',color=C['gold'],ms=12);txt(ax,2.0,5.6,'First interaction',14)
            if key=='hero':
                rng=np.random.default_rng(3)
                for angle in rng.uniform(0,2*np.pi,12):
                    ax.plot([cx,cx+.7*np.cos(angle)],[cy+2.1,cy+2.1+.7*np.sin(angle)],color=C['cyan'],lw=1.8)
        if key=='cavity_crossing':
            ax.plot([2.1,5.9,5.0],[3.8,2.5,1.2],color=C['cyan'],lw=3);txt(ax,3.4,.6,'A permitted optical route',14,C['cyan'])
        if key=='coordinates':
            arr(ax,(cx,cy),(cx+1.1,cy),'green');txt(ax,cx+1.25,cy,'x',13);txt(ax,cx+.25,cy+1.2,'+z',13)
        caption(ax,'SCHEMATIC • central slice; guide and sensors omitted')
    elif key in ['pet_pair','single_vs_pair']:
        ax.add_patch(Circle((5,3),2.2,fill=False,ec=C['blue'],lw=12));ax.plot(5,3,'o',c=C['gold'],ms=12)
        arr(ax,(5,3),(7.1,3),'red',4);txt(ax,7.3,4,'511 keV',17)
        if key=='pet_pair':arr(ax,(5,3),(2.9,3),'red',4);txt(ax,2.6,4,'511 keV',17);txt(ax,5,1.8,'Annihilation',17)
        else:txt(ax,5,1.8,'Current source:\none gamma',17)
        caption(ax,'SCHEMATIC • simplified source directions; not to scale')
    elif key=='atom':
        ax.set_aspect('equal');ax.add_patch(Circle((5,3),.5,fc=C['gold']))
        for ang in [0,60,120]:ax.add_patch(Ellipse((5,3),5,2,angle=ang,fill=False,ec=C['blue'],lw=2))
        ax.plot([2.5,6.2],[3,4.7],'o',c=C['blue'],ms=12);txt(ax,7.9,4.9,'electron',16);txt(ax,7.6,2.5,'nucleus',16)
        caption(ax,'TEACHING SYMBOL • electrons do not follow classical drawn orbits')
    elif key in ['energy_scale','wavelength','units','code_vocab','quiz_basics','glossary','quiz_console','console','quiz_yield','quiz_choices','validation','assumptions','metrics','three_controls','display_vs_physics']:
        items={
          'energy_scale':[('PRIMARY GAMMA','511,000 eV'),('VISIBLE PHOTON','about 2.384 eV'),('ENERGY RATIO','about 214,318 : 1')],
          'wavelength':[('WAVELENGTH','520 nm'),('RELATIONSHIP','E = hc / wavelength'),('PHOTON ENERGY','about 2.384 eV')],
          'units':[('CRYSTAL THICKNESS','10 mm = 1 cm'),('COUPLING LAYER','0.05 mm = 50 µm'),('DECAY PARAMETER','88 ns = 0.000000088 s')],
          'code_vocab':[('TYPE','G4double → decimal number'),('ASSIGNMENT','= → store a value'),('UNIT','* mm → a physical length')],
          'quiz_basics':[('ENERGY','511 keV'),('LENGTH','10 mm'),('CHANNEL COUNT','512 sensors')],
          'glossary':[('TRACK','one particle history'),('STEP','one segment of a track'),('HIT','one accepted optical photon here')],
          'console':[('CREATED','29,002 optical photons'),('ACCEPTED','1,099 photons / 303 sensors'),('DISPLAYED','100 optical + 8 other trajectories')],
          'quiz_console':[('SIMULATED','29,002 optical photons'),('ACCEPTED','1,099 optical hits'),('DRAWN','100 optical paths')],
          'quiz_yield':[('DEPOSIT','100 keV'),('YIELD','57 photons / keV'),('MEAN EMITTED','5,700 scintillation photons')],
          'quiz_choices':[('CAMERA ROTATION','changes the view'),('VIS-PHOTONS','changes stored display paths'),('ABSORPTION LENGTH','changes photon survival')],
          'validation':[('10-EVENT CHECK','107,474 optical / 5,295 hits'),('ACCOUNTING','zero guard kills'),('GEOMETRY AUDIT','zero reported overlaps; sampled')],
          'assumptions':[('CONFIRMED STRUCTURE','full sphere; GAGG; 12 + 20 regions'),('PROVISIONAL OPTICS','bulk absorption, coupling, coatings'),('FUTURE EVIDENCE','measured inputs and benchmarks')],
          'metrics':[('OPTICAL FRACTION','accepted photons / created photons'),('EVENT EFFICIENCY','selected events / emitted gammas'),('COINCIDENCE SENSITIVITY','requires paired-event definition')],
          'three_controls':[('-e','primary event count'),('--vis-photons','optical paths drawn'),('--track-photons','optical paths saved to CSV')],
          'display_vs_physics':[('TRANSPORT','all 29,002 optical photons'),('DISPLAY SAMPLE','first N optical tracks'),('HIT PROCESSING','all transported photons')],
        }[key]
        for i,(label,value) in enumerate(items):
            y=4.8-i*1.65
            ax.add_patch(FancyBboxPatch((.3,y-.6),9.4,1.35,boxstyle='round,pad=.08',fc=C['pale'],ec='none'))
            txt(ax,.6,y+.32,label,11,C['blue'],ha='left',weight='bold');txt(ax,.6,y-.2,value,20,ha='left')
        caption(ax,'REFERENCE DATA' if key in ['console','quiz_console','validation'] else 'CONCEPT SUMMARY')
    elif key in ['gagg','scintillation']:
        for i in range(5):
            for j in range(4):ax.add_patch(Circle((2+i*.7,1.4+j*.8),.17,fc=[C['blue'],C['green'],C['gold']][(i+j)%3]))
        arr(ax,(.4,3),(2,3),'red',3);txt(ax,1.1,3.6,'energy',14,C['red'])
        for angle in np.linspace(-1.2,1.2,7):arr(ax,(5.5,3),(5.5+2*np.cos(angle),3+2*np.sin(angle)),'cyan',2)
        txt(ax,7.9,5.2,'light',17,C['cyan']);txt(ax,3.4,5.25,'Gd₃Al₂Ga₃O₁₂ host',19)
        caption(ax,'SCHEMATIC • lattice and emission directions are illustrative')
    elif key in ['hemispheres','coupling','layer_strip','coatings']:
        if key=='hemispheres':
            ax.set_aspect('equal')
            ax.add_patch(Wedge((5,3.12),2.4,0,180,width=.7,fc=C['blue']))
            ax.add_patch(Wedge((5,2.88),2.4,180,360,width=.7,fc=C['blue']))
            ax.plot([2.6,7.4],[3,3],c=C['gold'],lw=5);txt(ax,5,4.6,'NORTH',17,'white');txt(ax,5,1.4,'SOUTH',17,'white')
            txt(ax,5,3,'0.1 mm total seam',16);caption(ax,'SCHEMATIC • equatorial gap exaggerated')
        else:
            names=['AIR','GAGG','COUPLING','GUIDE','SiPM'];colors=['pale','blue','gold','cyan','green'];widths=[1.5,2.2,.9,2.3,1.2];x=.5
            for label,color,w in zip(names,colors,widths):
                ax.add_patch(Rectangle((x,1.7),w,3,fc=C[color],ec='white',lw=2));txt(ax,x+w/2,3.2,label,13,'white' if color not in ['pale','gold'] else C['ink'],rotation=90 if w<1.3 else 0);x+=w
            arr(ax,(.8,.85),(9,.85),'red');txt(ax,5,5.3,'From the center toward an array',18)
            caption(ax,'SCHEMATIC • radial order; layer widths not to scale')
    elif key in ['guide_shape','polygons']:
        for center,n,col in [((2.7,3.3),5,'blue'),((7,3.3),6,'cyan')]:
            angles=np.linspace(0,2*np.pi,n,endpoint=False)+np.pi/2
            xy=np.column_stack((center[0]+1.7*np.cos(angles),center[1]+1.7*np.sin(angles)))
            ax.add_patch(Polygon(xy,fc=C[col],alpha=.25,ec=C[col],lw=3));ax.add_patch(Rectangle((center[0]-.7,center[1]-.7),1.4,1.4,fill=False,ec=C['ink'],lw=2))
            for point in xy:ax.plot([point[0],center[0]+np.sign(point[0]-center[0])*.7],[point[1],center[1]+np.sign(point[1]-center[1])*.7],c=C[col],lw=1)
        txt(ax,2.7,.9,'12 pentagonal regions',16);txt(ax,7,.9,'20 hexagonal regions',16)
        txt(ax,5,5.65,'32 regions → 32 square array outlets',18)
        caption(ax,'SCHEMATIC • face topology; actual transitions are three-dimensional')
    elif key in ['array','microcells']:
        n=4 if key=='array' else 12;step=3.5/n;start=(3.2,1.3)
        for i in range(n):
            for j in range(n):ax.add_patch(Rectangle((start[0]+i*step,start[1]+j*step),step*.77,step*.77,fc=C['green'],ec='white'))
        txt(ax,5,5.35,'4 × 4 SiPM array' if n==4 else 'Zoom inside one SiPM',22)
        txt(ax,5,.65,'16 sensors • 4.4 mm pitch' if n==4 else 'Reference: 30 × 30 = 900 microcells',16)
        caption(ax,'SCHEMATIC • only a subset of microcells is drawn' if n!=4 else 'SCHEMATIC • active regions and gaps, not packaging details')
    elif key in ['surfaces','bidirectional','refraction','tir']:
        ax.add_patch(Rectangle((.4,.7),9.2,2.3,fc='#dbeaf6',ec='none'));ax.plot([.4,9.6],[3,3],c=C['blue'],lw=2)
        if key=='surfaces':
            x=np.linspace(5,9,45);y=3+np.sin(x*20)*.13;ax.plot(x,y,c=C['blue'],lw=3);txt(ax,2.5,1.8,'polished',20);txt(ax,7,1.8,'ground',20);caption(ax,'SCHEMATIC • roughness magnified enormously')
        elif key=='bidirectional':
            arr(ax,(3,4.8),(3,1.5),'cyan',4);arr(ax,(7,1.5),(7,4.8),'gold',4);txt(ax,3,5.3,'a → b',18);txt(ax,7,5.3,'b → a',18);caption(ax,'SCHEMATIC • two ordered volume pairs')
        else:
            ax.plot([5,5],[.6,5.5],ls='--',color=C['muted']);arr(ax,(2.5,1),(5,3),'cyan',3)
            if key=='tir':arr(ax,(5,3),(7.5,1),'cyan',3);txt(ax,7.4,4.4,'No transmitted ray',16)
            else:arr(ax,(5,3),(8.3,4.5),'cyan',3);arr(ax,(5,3),(7.5,1),'gold',2);txt(ax,8,5.1,'refracted',15)
            txt(ax,2.1,4.5,'lower index',15);txt(ax,2.1,1.2,'higher index',15);caption(ax,'SCHEMATIC • ideal smooth-interface teaching example')
    elif key in ['boolean','overlap']:
        ax.add_patch(Circle((3,3),2,fc=C['blue'],alpha=.75));ax.add_patch(Circle((3,3),1.25,fc='white' if key=='boolean' else C['gold'],alpha=1 if key=='boolean' else .8));txt(ax,7.2,3,'outer shape\nminus inner shape\n= hollow shell' if key=='boolean' else 'Check material\nownership at\nsampled points',20)
        caption(ax,'SCHEMATIC • two-dimensional explanation of three-dimensional solids')
    elif key in ['compton','photoelectric','rayleigh']:
        arr(ax,(.6,3),(4.5,3),'red',4);ax.plot(4.5,3,'o',c=C['gold'],ms=14);txt(ax,2,3.6,'incoming gamma',16)
        if key!='photoelectric':arr(ax,(4.5,3),(8.3,4.7),'red',4);txt(ax,7.5,5.4,'scattered gamma',16)
        if key!='rayleigh':arr(ax,(4.5,3),(7.3,1.2),'blue',3);txt(ax,7.2,.7,'electron',17,C['blue'])
        if key=='rayleigh':txt(ax,5,1.4,'same gamma energy',22)
        if key=='photoelectric':txt(ax,7,4.4,'incoming gamma ends',20)
        caption(ax,'SCHEMATIC • actual directions and atomic products can differ')
    elif key in ['random_distance','absorption','decay','spectrum','yield']:
        plt.close(fig);fig,ax=plt.subplots(figsize=(9,5.5));fig.subplots_adjust(left=.13,bottom=.2,right=.96,top=.87)
        if key=='random_distance':
            x=np.linspace(0,50,300);ax.plot(x,np.exp(-x/10)/10,c=C['blue'],lw=3);ax.set(xlabel='Distance (toy units)',ylabel='Probability density',title='Toy constant-rate interaction model')
        elif key=='absorption':
            x=np.linspace(0,3000,300);ax.plot(x,np.exp(-x/1000),c=C['cyan'],lw=3);ax.set(xlabel='Path length (mm)',ylabel='Survival probability',title='Bulk absorption only: exp(−distance / 1000 mm)')
        elif key=='decay':
            x=np.linspace(0,450,300);ax.plot(x,np.exp(-x/88),c=C['green'],lw=3);ax.set(xlabel='Emission delay (ns)',ylabel='Relative density',title='Idealized scintillation decay: 88 ns')
        elif key=='spectrum':
            mid=1239.841984/520;ax.plot([mid-.04,mid-.01,mid-.01,mid+.01,mid+.01,mid+.04],[0,0,1,1,0,0],c=C['green'],lw=3);ax.set(xlabel='Optical photon energy (eV)',ylabel='Relative intensity',title='Provisional narrow, flat input spectrum',ylim=(-.05,1.2))
        else:
            x=np.linspace(0,511);ax.plot(x,57*x,c=C['green'],lw=3);ax.scatter([511],[28983],c=C['gold'],s=90,zorder=3);ax.set(xlabel='Deposited energy (keV)',ylabel='Mean scintillation photons',title='57 photons / keV; dot = reference event')
        ax.grid(alpha=.2);fig.text(.5,.03,'TEACHING MODEL / configured input • not a measured performance curve',ha='center',fontsize=11,color=C['muted'])
    elif key=='track_steps':
        points=np.array([[.7,1.8],[2.5,4.5],[4.2,2.2],[6.5,4.2],[9,2.5]])
        ax.plot(points[:,0],points[:,1],'-o',c=C['cyan'],lw=3)
        for i in range(4):txt(ax,*((points[i]+points[i+1])/2+np.array([.1,.4])),f'step {i+1}',14)
        txt(ax,5,5.6,'One track, several steps',22);caption(ax,'SCHEMATIC • a trajectory can store the path for drawing')
    elif key in ['optical_breakdown','hits_sensors','acceptance']:
        if key=='optical_breakdown':items=[('Scintillation','28,983'),('Cherenkov','19'),('Total optical','29,002')]
        elif key=='hits_sensors':items=[('Physical channels','512'),('With ≥ 1 hit','303'),('Accepted photons','1,099')]
        else:items=[('Created optical','29,002'),('Accepted photons','1,099'),('Accepted / created','3.79%')]
        for i,(a,b) in enumerate(items):box(ax,.4+i*3.2,1.8,2.8,2.7,b+'\n\n'+textwrap.fill(a,16),['blue','green','cyan'][i],19)
        caption(ax,'ACTUAL DATA • reference event 0, seed 42')
    elif key=='seed':
        return flow(['Config + seed 42','Same validated software','Same sampled history'], 'REPRODUCIBILITY • versions and configuration must also match')
    elif key=='color_legend':
        for i,(name,color) in enumerate([('gamma','red'),('electron','blue'),('positron','purple'),('optical photon','cyan'),('accepted hit','green')]):
            y=5-i;ax.plot([1,3],[y,y],c=C[color],lw=5,marker='o' if name=='accepted hit' else None);txt(ax,3.6,y,name,20,ha='left')
        caption(ax,'VIEWER LEGEND • colors are drawing choices')
    elif key in ['gamma_history','interaction_sites']:
        plt.close(fig);fig,ax=plt.subplots(figsize=(9,5.5));fig.subplots_adjust(left=.13,bottom=.21,right=.97,top=.87)
        if key=='gamma_history':
            values=[511]+[float(r['post_energy_keV']) for r in HISTORY];ax.plot(range(7),values,'o-',c=C['red'],lw=3);ax.set(xticks=range(7),xticklabels=['start','C1','C2','C3','C4','Rayl','phot'],ylabel='Primary gamma energy (keV)',title='Actual primary track 1 • event 0')
        else:
            xyz=np.array([[float(r[k]) for k in ['x_mm','y_mm','z_mm']] for r in HISTORY]);ax.plot(xyz[:,0],xyz[:,2],c=C['red']);ax.scatter(xyz[:,0],xyz[:,2],s=100,c=C['gold'],zorder=4)
            for i,(x,y,z) in enumerate(xyz):ax.annotate(str(i+1),(x,z),xytext=(8,8),textcoords='offset points',fontsize=14)
            ax.set(xlabel='x (mm)',ylabel='z (mm)',title='Actual interaction sites • x–z projection');ax.margins(.2)
        ax.grid(alpha=.2)
    elif key in ['occupancy','arrival','event_variation']:
        plt.close(fig);fig,ax=plt.subplots(figsize=(9,5.5));fig.subplots_adjust(left=.12,bottom=.18,right=.9,top=.89)
        if key=='arrival':
            ax.hist([float(r['time_ns']) for r in HITS],bins=45,color=C['cyan'],edgecolor='white');ax.set(xlabel='Accepted-photon arrival time (ns)',ylabel='Accepted photons / bin',title='Actual event 0 • all 1,099 accepted hits')
        elif key=='event_variation':
            ax.bar([int(r['event']) for r in EVENTS],[float(r['edep_scintillator_keV']) for r in EVENTS],color=C['blue']);ax.set(xlabel='Event ID',ylabel='Crystal energy deposit (keV)',title='Actual ten-event reference run',xticks=range(10))
        else:
            from collections import Counter
            counts=Counter(int(r['sipm_id']) for r in HITS);sensors=rows('sensor_positions.csv')
            # Sensor IDs are explicit in sd_name, never inferred from file order.
            import re
            ids=[int(re.search(r'(\d+)$',r['sd_name']).group(1)) for r in sensors]
            xyz=np.array([[float(r[k]) for k in ['x_mm','y_mm','z_mm']] for r in sensors]);az=np.degrees(np.arctan2(xyz[:,1],xyz[:,0]));lat=np.degrees(np.arcsin(xyz[:,2]/np.linalg.norm(xyz,axis=1)));v=np.array([counts[i] for i in ids]);ax.scatter(az[v==0],lat[v==0],c='#d4dfe9',s=25);sc=ax.scatter(az[v>0],lat[v>0],c=v[v>0],norm=LogNorm(vmin=1,vmax=max(v)),cmap='viridis',s=40);fig.colorbar(sc,ax=ax,label='Accepted photons / sensor');ax.set(xlabel='Azimuth (degrees)',ylabel='Latitude (degrees)',title='Actual event 0 • 512 sensor positions')
        ax.grid(alpha=.15)
    elif key=='compare_native':return flow(['Draw 0 paths','Draw 5 paths','Draw 100 paths','Same 1,099 hits'], 'ACTUAL CHECK • seed 42 reproduced the same event in all three views')
    elif key=='paper_guides':
        box(ax,.5,1.6,4.2,3.3,'Printed candidate\n\nSomos WaterClear\n\n49% at 420 nm', 'blue',18);box(ax,5.3,1.6,4.2,3.3,'Cast candidate\n\nPoly-Optic 1730\n\n73% at 420 nm','cyan',18);caption(ax,'PAPER DATA • 1 cm samples; transmission is not bulk absorption alone')
    elif key=='sensitivity':return flow(['Choose one parameter','Choose scenario values','Run independent samples','Compare with uncertainty'],'STUDY DESIGN • current example scan is a demonstration, not a conclusion')
    else:raise KeyError(key)
    return fig,ax

def build_assets():
    for label in ['geometry','gamma','five','hundred']:
        p=ASSETS/f'native_{label}.png'
        if p.exists():
            im=Image.open(p).convert('RGB')
            # The virtual X desktop is black; crop the native viewport.
            if im.size==(1200,900):im.crop((200,0,1200,800)).save(ASSETS/f'native_{label}_crop.png')
    for key in sorted(set(s['visual'] for s in SLIDES)):
        if key.startswith('native_'):continue
        if key=='actual_geometry':
            shutil.copyfile(ASSETS/'native_geometry_crop.png',ASSETS/'actual_geometry.png');continue
        if key=='event_overview':
            if (RUN/'figures/event_000.png').exists():
                shutil.copyfile(RUN/'figures/event_000.png',ASSETS/'event_overview.png')
            assert (ASSETS/'event_overview.png').exists()
            continue
        fig,ax=make(key);fig.savefig(ASSETS/f'{key}.png',dpi=160,bbox_inches='tight',pad_inches=.12);fig.savefig(ASSETS/f'{key}.svg',bbox_inches='tight',pad_inches=.12);plt.close(fig)
    print('Built',len(set(s['visual'] for s in SLIDES)),'visual assets')
if __name__=='__main__':build_assets()
