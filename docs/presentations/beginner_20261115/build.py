"""Build editable PPTX, matching PDF, full study guide and offline viewer."""
import sys, os, csv, json, math, textwrap, hashlib, shutil, html, base64, subprocess
from pathlib import Path
from collections import Counter
from xml.sax.saxutils import escape
from PIL import Image
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.shapes import MSO_SHAPE
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.colors import HexColor
from reportlab.platypus import SimpleDocTemplate, Paragraph, Spacer, Preformatted, Image as RLImage, PageBreak
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_LEFT
import matplotlib.font_manager as fm
from content import SLIDES, REFS, PROJECT
from assets import OUT, ASSETS, RUN

for name,family,weight in [('Body','DejaVu Sans','normal'),('Bold','DejaVu Sans','bold'),('Mono','DejaVu Sans Mono','normal')]:
    pdfmetrics.registerFont(TTFont(name,fm.findfont(fm.FontProperties(family=family,weight=weight))))
COL={'ink':'172C48','muted':'526880','blue':'2878C8','cyan':'00A9BB','green':'27946B','pale':'EDF4FA','dark':'12263F','white':'FFFFFF'}
W,H=960,540

def wrap(s,font,size,width):
    out=[]
    if font == 'Mono':
        limit=max(12, int(width/pdfmetrics.stringWidth('M',font,size)))
        for raw in s.split('\n'):
            out.extend(textwrap.wrap(raw,width=limit,replace_whitespace=False,drop_whitespace=False,subsequent_indent='  ↳ ') or [''])
        return out
    for raw in s.split('\n'):
        if raw=='':out.append('');continue
        words=raw.split(' '); line=''
        for word in words:
            candidate=(line+' '+word) if line else word
            if pdfmetrics.stringWidth(candidate,font,size)<=width:line=candidate
            else:
                if line:out.append(line);line=''
                while pdfmetrics.stringWidth(word,font,size)>width:
                    n=max(1,int(width/(size*.65)))
                    while pdfmetrics.stringWidth(word[:n],font,size)>width:n-=1
                    out.append(word[:n]);word=word[n:]
                line=word
        out.append(line)
    return out

class Page:
    def __init__(self,slide,pdf):self.slide=slide;self.pdf=pdf
    def rect(self,x,y,w,h,color):
        sh=self.slide.shapes.add_shape(MSO_SHAPE.RECTANGLE,Pt(x),Pt(y),Pt(w),Pt(h));sh.fill.solid();sh.fill.fore_color.rgb=RGBColor.from_string(color);sh.line.fill.background()
        self.pdf.setFillColor(HexColor('#'+color));self.pdf.rect(x,H-y-h,w,h,fill=1,stroke=0)
    def text(self,x,y,w,text,size=17,font='Body',color=None,leading=None):
        color=color or COL['ink'];leading=leading or size*1.24
        lines=wrap(text,font,size,w);height=max(leading*len(lines)+4,leading+4)
        sh=self.slide.shapes.add_textbox(Pt(x),Pt(y),Pt(w+2),Pt(height));tf=sh.text_frame;tf.clear();tf.margin_left=tf.margin_right=tf.margin_top=tf.margin_bottom=0;tf.word_wrap=False
        for i,line in enumerate(lines):
            p=tf.paragraphs[0] if i==0 else tf.add_paragraph();p.text=line;p.space_before=Pt(0);p.space_after=Pt(0);p.line_spacing=Pt(leading)
            for r in p.runs:r.font.name='DejaVu Sans Mono' if font=='Mono' else 'DejaVu Sans';r.font.size=Pt(size);r.font.bold=(font=='Bold');r.font.color.rgb=RGBColor.from_string(color)
        self.pdf.setFillColor(HexColor('#'+color));self.pdf.setFont(font,size)
        for i,line in enumerate(lines):self.pdf.drawString(x,H-y-size-i*leading,line)
        return len(lines)*leading
    def picture(self,path,x,y,w,h):
        im=Image.open(path);iw,ih=im.size;scale=min(w/iw,h/ih);dw,dh=iw*scale,ih*scale;xx=x+(w-dw)/2;yy=y+(h-dh)/2
        self.slide.shapes.add_picture(str(path),Pt(xx),Pt(yy),width=Pt(dw),height=Pt(dh));self.pdf.drawImage(str(path),xx,H-yy-dh,dw,dh,mask='auto')

def image_path(s):
    key=s['visual'];return ASSETS/(key+'_crop.png' if key.startswith('native_') else key+'.png')

def source_label(s):
    # Display only stable short keys; full titles and links are preserved in notes.
    return 'Sources: '+', '.join(s['refs'])+'  •  full links in notes / references'

def fullnotes(s,i):
    lines=[f"Slide {i}: {s['title']}",s['notes'],'','CODE WALKTHROUGH',*s['explain'],'',s['code_kind'],s['code_source'],s['code']]
    if s['question']:lines+=['','CHECKPOINT',s['question'],'ANSWER: '+s['answer']]
    lines+=['','REFERENCES']+[f'{k}: {REFS[k][0]} — {REFS[k][1]}' for k in s['refs']]
    return '\n'.join(lines)

def render():
    prs=Presentation();prs.slide_width=Pt(W);prs.slide_height=Pt(H)
    prs.core_properties.title='Spherical PET: from one gamma to a measured light pattern'
    prs.core_properties.subject='112-slide beginner course with actual Geant4 code, native event displays and data'
    prs.core_properties.author='Spherical PET project / Imran Hossain'
    prs.core_properties.comments='Current GAGG model; snapshot 2026-11-15. See notes and references.'
    pdf=canvas.Canvas(str(OUT/'spherical_pet_beginner.pdf'),pagesize=(W,H));pdf.setTitle(prs.core_properties.title)
    code_sizes=[]
    for i,s in enumerate(SLIDES,1):
        slide=prs.slides.add_slide(prs.slide_layouts[6]);p=Page(slide,pdf)
        p.rect(0,0,W,H,'FFFFFF');p.rect(0,0,8,H,COL['blue']);p.rect(526,103,402,400,COL['pale'])
        p.text(32,16,800,s['chapter'],11,'Bold',COL['blue']);p.text(32,38,896,s['title'],25.5,'Bold',leading=30)
        yy=111
        for b in s['bullets']:
            p.text(32,yy,14,'•',16.5,'Bold',COL['cyan']);h=p.text(49,yy,450,b,16.5,leading=20.5);yy+=h+7
        # Visual placement adapts to the number of wrapped bullet lines.
        visual_y=max(225,yy+5);p.picture(image_path(s),30,visual_y,476,503-visual_y)
        p.text(540,113,370,s['code_kind']+'  •  ↳ = wrap',10.5,'Bold',COL['blue'])
        p.text(540,132,374,s['code_source'],9.3,color=COL['muted'],leading=11.4)
        p.rect(538,164,378,205,COL['dark'])
        code_size=12.3
        while len(wrap(s['code'],'Mono',code_size,350))*code_size*1.35>184 and code_size>9:code_size-=.2
        code_sizes.append(round(code_size,2));p.text(551,175,350,s['code'],code_size,'Mono','F1F7FF',code_size*1.35)
        ey=382
        for e in s['explain']:
            h=p.text(541,ey,370,e,14.5,leading=18);ey+=h+8
        if ey>505:raise ValueError(('explanation overflow',i,ey))
        p.text(32,520,800,source_label(s),8.2,color=COL['muted']);p.text(866,519,65,f'{i:03d} / {len(SLIDES)}',9.3,'Bold',COL['blue'])
        slide.notes_slide.notes_text_frame.text=fullnotes(s,i)
        pdf.showPage()
    prs.save(OUT/'spherical_pet_beginner.pptx');pdf.save()
    return code_sizes

def markdown_and_handout():
    md=['# Spherical PET: a complete beginner course','',f'{len(SLIDES)} slides • current GAGG model • source snapshot 2026-11-15','',
        'Read the idea, inspect the figure, then follow the code. Actual source excerpts are copied from the snapshot; teaching Python illustrates a concept and is not the transport engine. Future pseudocode describes unimplemented work. The native screenshots reproduce event 0 with seed 42.','', '## Course map','']
    for chapter,count in Counter(s['chapter'] for s in SLIDES).items():md.append(f'- {chapter}: {count} slides')
    styles=getSampleStyleSheet();styles.add(ParagraphStyle(name='CourseTitle',fontName='Bold',fontSize=23,leading=28,spaceAfter=14));styles.add(ParagraphStyle(name='CourseBody',fontName='Body',fontSize=10,leading=15,spaceAfter=9));styles.add(ParagraphStyle(name='CourseSmall',fontName='Body',fontSize=8,leading=11,spaceAfter=7));styles.add(ParagraphStyle(name='CourseCode',fontName='Mono',fontSize=8,leading=10,backColor=HexColor('#EDF4FA'),borderPadding=8,spaceAfter=12))
    story=[Paragraph('Spherical PET<br/>Complete beginner study guide',styles['CourseTitle']),Paragraph('Companion to the 112-slide presentation. Includes every explanation, code walkthrough, checkpoint answer and source reference. Snapshot: 15 November 2026.',styles['CourseBody']),Paragraph('The PowerPoint is editable. The slide PDF and offline HTML viewer use the same authored layout. This study guide expands the teaching notes. Diagrams are labeled as schematics, configured teaching models, or actual results.',styles['CourseBody'])]
    for chapter,count in Counter(s['chapter'] for s in SLIDES).items():story.append(Paragraph(escape(f'{chapter} — {count} slides'),styles['CourseBody']))
    for i,s in enumerate(SLIDES,1):
        rel='assets/'+image_path(s).name
        md+=['',f"## Slide {i:03d} — {s['title']}",'',f"**{s['chapter']}**",'']+[f'- {b}' for b in s['bullets']]+['',f"![{s['title']}]({rel})",'',s['notes'],'',f"**{s['code_kind']}** — `{s['code_source']}`",'']
        lang='cpp' if s['code_kind']=='ACTUAL C++ / CONFIG' else 'python' if s['code_kind']=='TEACHING PYTHON' else 'bash' if s['code_kind']=='TERMINAL COMMAND' else 'text'
        md+=['```'+lang,s['code'],'```','']+[f'- {e}' for e in s['explain']]
        if s['question']:md+=['','**Checkpoint:** '+s['question'],'','**Answer:** '+s['answer']]
        md+=['','Sources: '+ '; '.join(f"[{k}: {REFS[k][0]}]({REFS[k][1] if REFS[k][1].startswith('https') else '../../../'+REFS[k][1]})" for k in s['refs'])+'.']
        story.append(PageBreak());story.append(Paragraph(escape(f'{i:03d} • {s["chapter"]}'),styles['CourseSmall']));story.append(Paragraph(escape(s['title']),styles['CourseTitle']))
        for b in s['bullets']:story.append(Paragraph('• '+escape(b),styles['CourseBody']))
        im=Image.open(image_path(s));iw,ih=im.size;scale=min(470/iw,210/ih);story.append(RLImage(str(image_path(s)),width=iw*scale,height=ih*scale));story.append(Spacer(1,10));story.append(Paragraph(escape(s['notes']),styles['CourseBody']));story.append(Paragraph(escape(s['code_kind']+' — '+s['code_source']),styles['CourseSmall']))
        story.append(Preformatted('\n'.join(wrap(s['code'],'Mono',8,455)),styles['CourseCode']))
        for e in s['explain']:story.append(Paragraph('• '+escape(e),styles['CourseBody']))
        if s['question']:story.append(Paragraph('<b>Checkpoint:</b> '+escape(s['question'])+'<br/><b>Answer:</b> '+escape(s['answer']),styles['CourseBody']))
        story.append(Paragraph('Sources: '+escape('; '.join(k+': '+REFS[k][1] for k in s['refs'])),styles['CourseSmall']))
    (OUT/'study_guide.md').write_text('\n'.join(md)+'\n')
    def footer(c,doc):c.setFont('Body',8);c.setFillColor(HexColor('#526880'));c.drawString(44,24,'Spherical PET • beginner study guide • 2026-11-15');c.drawRightString(568,24,str(doc.page))
    SimpleDocTemplate(str(OUT/'study_guide.pdf'),pagesize=(612,792),rightMargin=44,leftMargin=44,topMargin=38,bottomMargin=40).build(story,onFirstPage=footer,onLaterPages=footer)

def references():
    lines=['# Reference index','', 'The local implementation and saved output define the behavior described in this course. External sources support background concepts and motivate future studies. A cited paper is not evidence that this baseline reproduces its detector performance.','']
    for k,(name,url) in REFS.items():lines += [f'## {k} — {name}','',f'[{name}]({url if url.startswith("https") else "../../../"+url})','']
    (OUT/'references.md').write_text('\n'.join(lines))
    glossary={'primary':'Particle injected by the configured source.','secondary':'Particle created by an interaction or emission process.','run':'A group of events; this application supports one BeamOn per invocation.','event':'One configured primary and its descendants in the current baseline.','track':'The history of one particle.','step':'A segment of a track ending at a process or navigation limit.','trajectory':'A stored path used for visualization.','hit':'Here, a recorded accepted optical photon at a SiPM.','scintillator':'Material that emits light in response to deposited energy.','GAGG':'Gadolinium aluminum gallium garnet; the present crystal host.','coupling':'Optical contact material between parts.','light guide':'Transparent shaped part that transports and distributes light.','SiPM':'Silicon photomultiplier: many avalanche microcells with a combined sensor output.','PDE':'Photon detection efficiency; do not reapply it to already accepted hits.','spectrum':'Distribution of emitted intensity over energy or wavelength.','refractive index':'Property that affects optical propagation and boundary refraction/reflection.','absorption length':'Characteristic distance for exponential photon survival in bulk material.','Monte Carlo':'Simulation based on probabilistic sampling under specified physical rules.','seed':'Initialization of a pseudorandom sequence.','Compton':'Gamma scatter transferring energy to an electron.','photoelectric':'Absorption of an incident gamma, releasing an electron and possible atomic-relaxation products.','Rayleigh':'Coherent gamma scattering with essentially unchanged energy in this model.','Cherenkov':'Light emitted by a charged particle moving faster than the phase speed of light in a medium.','validation':'Comparison with suitable trusted measurements or benchmarks.','CNN':'Convolutional neural network; no network is trained by the present application.','coincidence':'Detections compatible under a specified timing and event-selection rule.','line of response':'Line connecting estimated paired detector interaction positions.'}
    (OUT/'glossary.md').write_text('# Beginner glossary\n\n'+'\n\n'.join(f'**{k}** — {v}' for k,v in glossary.items())+'\n')

TEACHING='''#!/usr/bin/env python3
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
'''

def snapshot():
    data=OUT/'data';data.mkdir(exist_ok=True)
    for name in ['events.csv','sipm_hits.csv','sipm_counts.csv','gamma_interactions.csv','gamma_steps.csv','sensor_positions.csv','guide_sections.csv','summary.json','resolved.conf','source_commands.mac','geometry_check.json']:
        if (RUN/name).exists():shutil.copy2(RUN/name,data/name)
    source=OUT/'source_snapshot';source.mkdir(exist_ok=True)
    paths=['src/Config.cc','src/DetectorConstruction.cc','src/main.cc','include/GeometryParameters.hh','config/ray_baseline.conf','docs/ray_baseline.md','docs/event_display_guide.md','docs/output.md','docs/parameters.md','docs/physics.md','docs/validation.md','third_party/g4sipm/src/hit/G4SipmSensitiveDetector.cc','third_party/g4sipm/src/hit/G4SipmSensitiveDetectorFilter.cc','third_party/g4sipm/LICENSE']
    hashes={}
    for path in paths:
        dst=source/path;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(PROJECT/path,dst);hashes[path]=hashlib.sha256(dst.read_bytes()).hexdigest()
    (OUT/'teaching_examples.py').write_text(TEACHING)
    for name in ['content.py','assets.py','build.py','capture_native.py']:shutil.copy2(Path(__file__).parent/name,OUT/name)
    # Rebuilding in the package uses its preserved source, unless explicitly overridden.
    cp=OUT/'content.py';st=cp.read_text();st=st.replace("PROJECT = Path('/work/eic3/users/ihossain/Medical_imaging/spherical-pet')","PROJECT = Path(__file__).resolve().parent / 'source_snapshot'")
    cp.write_text(st)
    # source commands are used directly by one slide.
    saved=source/'results/ray_baseline_20261113';saved.mkdir(parents=True,exist_ok=True);shutil.copy2(RUN/'source_commands.mac',saved/'source_commands.mac')
    ap=OUT/'assets.py';st=ap.read_text().replace("OUT=PROJECT/'docs/presentations/beginner_20261115'","OUT=Path(__file__).resolve().parent").replace("RUN=PROJECT/'results/ray_baseline_20261113'","RUN=OUT/'data'")
    ap.write_text(st)
    return hashes

def viewer():
    preview=OUT/'previews';preview.mkdir(exist_ok=True)
    subprocess.run(['pdftoppm','-scale-to','1280','-jpeg','-jpegopt','quality=85',str(OUT/'spherical_pet_beginner.pdf'),str(preview/'slide')],check=True)
    files=sorted(preview.glob('slide-*.jpg'));assert len(files)==len(SLIDES)
    pages=[]
    for i,(s,path) in enumerate(zip(SLIDES,files),1):
        pages.append({'title':s['title'],'chapter':s['chapter'],'image':'data:image/jpeg;base64,'+base64.b64encode(path.read_bytes()).decode(),'notes':fullnotes(s,i)})
    payload=json.dumps(pages,ensure_ascii=False).replace('</','<\\/')
    template='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Spherical PET — beginner course</title>
<style>*{box-sizing:border-box}body{margin:0;background:#102238;color:#edf4fa;font:16px system-ui}header{padding:12px 20px;display:flex;gap:12px;align-items:center;flex-wrap:wrap}button,select,input{font:inherit;padding:7px;border-radius:5px;border:1px solid #8097b1}button{cursor:pointer}#stage{max-width:1500px;margin:auto}#slide{display:block;width:100%;height:auto}#notes{white-space:pre-wrap;background:#fff;color:#172c48;padding:28px;line-height:1.6;max-width:1100px;margin:18px auto}#hint{color:#bdcede;font-size:13px}a{color:#9ee5ef}#search{width:180px}@media print{body{background:white}header{display:none}#notes{display:block!important}#slide{width:100%}}</style>
<header><strong>Spherical PET</strong><button id="prev" aria-label="Previous slide">←</button><span id="counter"></span><button id="next" aria-label="Next slide">→</button><select id="chap" aria-label="Chapter"></select><input id="search" placeholder="Search / Enter" aria-label="Search slide text"><button id="toggle">Notes (N)</button><a href="spherical_pet_beginner.pdf">PDF</a><a href="study_guide.pdf">Study guide</a><span id="hint">← → navigate · Home / End · N notes · #slide-number links</span></header><main id="stage"><img id="slide" alt=""><section id="notes" hidden></section></main>
<script>const pages=__DATA__;let current=Math.max(0,Math.min(pages.length-1,(parseInt(location.hash.slice(1))||1)-1));const el=id=>document.getElementById(id);let last='';pages.forEach((p,i)=>{if(p.chapter!==last){const o=document.createElement('option');o.value=i;o.textContent=p.chapter;el('chap').append(o);last=p.chapter}});function show(){const p=pages[current];el('slide').src=p.image;el('slide').alt=p.title;el('notes').textContent=p.notes;el('counter').textContent=(current+1)+' / '+pages.length;el('prev').disabled=current===0;el('next').disabled=current===pages.length-1;for(const o of el('chap').options){if(pages[Number(o.value)].chapter===p.chapter)el('chap').value=o.value}history.replaceState(null,'','#'+(current+1));document.title=(current+1)+'. '+p.title}function go(d){current=Math.max(0,Math.min(pages.length-1,current+d));show()}el('prev').onclick=()=>go(-1);el('next').onclick=()=>go(1);el('toggle').onclick=()=>el('notes').hidden=!el('notes').hidden;el('chap').onchange=e=>{current=Number(e.target.value);show()};el('search').onkeydown=e=>{if(e.key==='Enter'){const q=e.target.value.toLowerCase();for(let j=1;j<=pages.length;j++){let i=(current+j)%pages.length;if((pages[i].title+' '+pages[i].notes).toLowerCase().includes(q)){current=i;show();return}}}};document.onkeydown=e=>{if(['INPUT','SELECT'].includes(e.target.tagName))return;if(e.key==='ArrowRight'||e.key==='PageDown'){e.preventDefault();go(1)}if(e.key==='ArrowLeft'||e.key==='PageUp'){e.preventDefault();go(-1)}if(e.key==='Home'){current=0;show()}if(e.key==='End'){current=pages.length-1;show()}if(e.key.toLowerCase()==='n')el('toggle').click()};window.addEventListener('hashchange',()=>{current=Math.max(0,Math.min(pages.length-1,(parseInt(location.hash.slice(1))||1)-1));show()});show();</script></html>'''
    (OUT/'presentation.html').write_text(template.replace('__DATA__',payload))
    # Contact sheets make every slide visually reviewable without opening PowerPoint.
    from PIL import ImageDraw
    sheets=[]
    for start in range(0,len(files),16):
        sheet=Image.new('RGB',(1280,800),'#dde5ee');d=ImageDraw.Draw(sheet)
        for j,path in enumerate(files[start:start+16]):
            im=Image.open(path);im.thumbnail((310,175));x=(j%4)*320+5;y=(j//4)*200+20;sheet.paste(im,(x,y));d.text((x,y-15),str(start+j+1),fill='#172c48')
        p=preview/f'contact_{start//16+1:02d}.jpg';sheet.save(p,quality=90);sheets.append(str(p))
    return sheets

def validate(code_sizes,hashes,sheets):
    subprocess.run([sys.executable,str(OUT/'teaching_examples.py')],check=True)
    for i,s in enumerate(SLIDES,1):
        if s['code_kind']=='TEACHING PYTHON':compile(s['code'],f'slide_{i}','exec')
        if s['code_kind']=='TERMINAL COMMAND':
            result=subprocess.run(['bash','-n'],input=s['code'],text=True,capture_output=True)
            if result.returncode:raise ValueError((i,result.stderr))
        assert s['notes'] and s['code'] and s['explain'] and image_path(s).exists()
    prs=Presentation(OUT/'spherical_pet_beginner.pptx');assert len(prs.slides)==112
    for i,slide in enumerate(prs.slides,1):
        assert len(slide.notes_slide.notes_text_frame.text)>200
        for sh in slide.shapes:
            if sh.left < -Pt(.2) or sh.top < -Pt(.2) or sh.left+sh.width>prs.slide_width+Pt(.2) or sh.top+sh.height>prs.slide_height+Pt(.2):raise ValueError(('shape outside slide',i,sh.left,sh.top,sh.width,sh.height))
    report={'date':'2026-11-15','slides':112,'chapters':10,'all_slides_have_code':True,'all_slides_have_notes':True,'all_teaching_python_compiles':True,'all_shell_examples_pass_bash_n':True,'pptx_package_reopened':True,'pptx_shapes_inside_canvas':True,'minimum_code_font_pt':min(code_sizes),'reference_data_checks':'passed; see teaching_examples.py','native_captures':'0, 5 and 100 optical display paths each reproduce 29002 created and 1099 accepted','pdf_rendered_pages':112,'source_sha256':hashes,'rendering_note':'PDF and PPTX use one authored layout. PDF pages rendered with Poppler. Native PowerPoint rendering not available on this host; PPTX reopened and checked structurally.','physical_validation_note':'Presentation checks do not establish detector calibration or experimental accuracy.'}
    (OUT/'verification.json').write_text(json.dumps(report,indent=2)+'\n')
    return report

def readme():
    (OUT/'README.md').write_text('''# Spherical PET beginner presentation

112 slides in 10 chapters, with actual code excerpts, labeled teaching code, detailed notes, schematics, flowcharts, native event displays, actual data plots and answered exercises. Prepared 2026-11-15 against the current GAGG baseline.

## Start here

- **presentation.html** — open locally in a browser; arrow keys navigate, N toggles notes, chapter selector and search are available. Slide images and notes are embedded, so the HTML works alone without network access.
- **spherical_pet_beginner.pptx** — editable PowerPoint with speaker notes. Install DejaVu Sans and DejaVu Sans Mono for the closest layout match; applications may substitute fonts.
- **spherical_pet_beginner.pdf** — 112-slide PDF.
- **study_guide.pdf / study_guide.md** — all explanations, figures, code walkthroughs, answers and source references.
- **glossary.md / references.md** — definitions and full source links.
- **teaching_examples.py** — executable, read-only data lesson; standard-library Python only.
- **data/** — portable copy of the small reference CSV/JSON/configuration files.
- **source_snapshot/** — the files supporting the extracted implementation examples; this is not a complete buildable checkout.
- **assets/** — scientific PNG/SVG diagrams and native screenshots.
- **previews/** — rendered slide images and contact sheets, generated locally during regeneration and excluded from Git.
- **verification.json** — checks and source hashes.

Run the portable data lesson:

```bash
python3 teaching_examples.py
```

## How to use the course

Start with chapters 1–2 even if the detector screenshot is your immediate interest. Chapters 3–5 build the detector and follow gamma/electron/optical histories. Chapter 6 connects these to C++ actions. Chapter 7 reads the actual event. Chapters 8–10 provide terminal practice, interpretation, literature and exercises. Cover this over several lessons, not one lecture.

Each slide includes a code block. ACTUAL C++ excerpts are extracted from identified files and line numbers. TEACHING PYTHON explains an idea and sometimes depends on variables introduced in the preceding data-loading lesson. FUTURE / PSEUDOCODE is explicitly not an implemented API. The companion script provides complete runnable versions of the saved-data checks.

## Scope and evidence

The reference is results/ray_baseline_20261113: ten single 511 keV gamma events, seed 42. Event 0 deposited 511 keV and created 29,002 optical photons (28,983 scintillation + 19 Cherenkov), with 1,099 accepted hits across 303 of 512 sensors. The 0/5/100-path native captures reproduce those totals. Drawing limits do not reduce optical transport.

Diagrams state when they are schematic or not to scale. The detector has provisional material/boundary inputs, a reference SiPM, and no application electronics digitization, coincidence analysis or trained CNN. The two papers motivate future material and reconstruction work; their results are not claimed as this model's performance.

## Regeneration

The packaged content.py uses source_snapshot for exact excerpts. assets.py can regenerate the scientific diagrams from data; the native screenshots and original event overview are preserved assets. build.py regenerates the presentation files. Dependencies: Python 3.9+, numpy, matplotlib, Pillow, python-pptx, reportlab, Poppler pdftoppm, and DejaVu fonts. The default build reads existing assets. Native capture additionally needs the original project, Geant4 11.1.2, Xvfb and ImageMagick; its paths are installation-specific.

```bash
python3 build.py
```

The matching PDF was rendered and inspected; PPTX was reopened and checked for slide/notes counts and canvas bounds. This host has no PowerPoint/LibreOffice renderer, so application-specific font substitution remains possible. All presentation data checks pass; they do not add physical calibration or performance validation.
''')

if __name__=='__main__':
    OUT.mkdir(parents=True,exist_ok=True)
    sizes=render();print('Rendered PPTX and PDF',flush=True)
    markdown_and_handout();references();print('Wrote study guides',flush=True)
    # For a packaged rebuild, preserve the already captured snapshots.
    if PROJECT.name=='source_snapshot':
        hashes=json.loads((OUT/'verification.json').read_text())['source_sha256']
    else:hashes=snapshot()
    sheets=viewer();report=validate(sizes,hashes,sheets);readme();print(json.dumps({'slides':report['slides'],'minimum_code_font_pt':report['minimum_code_font_pt'],'contact_sheets':sheets}),flush=True)
