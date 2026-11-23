#!/usr/bin/env python3
"""작품설명 한 장 그림 생성 (docs/images/overview.svg). PNG: rsvg-convert -z 1.2 overview.svg -o overview.png"""
import pathlib, math, base64, io
from PIL import Image, ImageFont
ROOT = pathlib.Path(__file__).resolve().parent.parent
IMG = ROOT/"docs"/"images"
OUT = IMG/"overview.svg"
W, H = 1980, 1900
NODE = {"kb":("#dbeafe","#2563eb"),"ik":("#dcfce7","#16a34a"),"wr":("#fef3c7","#d97706"),
        "dr":("#fce7f3","#db2777"),"mg":("#ede9fe","#7c3aed"),"dv":("#ffedd5","#ea580c"),
        "ext":("#f3f4f6","#6b7280"),"dbg":("#f9fafb","#6b7280")}
GRN,AMB,PNK="#16a34a","#d97706","#db2777"
o=[]; markers={}
FP="/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
_f={}
def tw(s,size):
    k=round(size*4)
    if k not in _f: _f[k]=ImageFont.truetype(FP,k,index=1) if False else ImageFont.truetype(FP,k)
    return _f[k].getlength(s)/4
def fit(s,maxw,size,mn=13,bold=False):
    f=1.06 if bold else 1.0
    while size>mn and tw(s,size)*f>maxw: size-=0.5
    return size
def esc(s): return s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")
def T(x,y,s,size=14,w="400",fill="#111827",anchor="start",maxw=None):
    if maxw: size=fit(s,maxw,size,13,w in("700","600"))
    o.append(f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{w}" fill="{fill}" text-anchor="{anchor}">{esc(s)}</text>')
def R(x,y,w,h,fill,stroke,sw=2,rx=8,dash=False):
    d=' stroke-dasharray="7 5"' if dash else ""
    o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{d}/>')
def mk(c):
    if c not in markers: markers[c]=f"m{len(markers)}"
    return markers[c]
def A(pts,c="#374151",sw=1.8,dash=False,both=False,head=True):
    d="M"+" L".join(f"{x},{y}" for x,y in pts)
    da=' stroke-dasharray="6 5"' if dash else ""
    ms=f' marker-start="url(#{mk(c)})"' if both else ""
    me=f' marker-end="url(#{mk(c)})"' if head else ""
    o.append(f'<path d="{d}" fill="none" stroke="{c}" stroke-width="{sw}"{da} stroke-linejoin="round"{me}{ms}/>')
def block(x,y,w,h,node,title,subs,dash=False,ts=22,ss=17):
    f,s=NODE[node]; R(x,y,w,h,f,s,2,9,dash)
    n=len(subs); tot=28+22*n; b=y+h/2-tot/2+21
    T(x+w/2,b,title,ts,"700","#111827","middle",maxw=w-16)
    for i,t in enumerate(subs): T(x+w/2,b+28+i*22,t,ss,"400","#4b5563","middle",maxw=w-20)
def cap(x,y,label,sub,w=100):
    R(x,y,w,56,"#fff","#374151",1.8,8)
    T(x+w/2,y+25,label,18,"700","#111827","middle",maxw=w-8); T(x+w/2,y+46,sub,14.5,"400","#4b5563","middle",maxw=w-8)
def card(x,y,w,h,col,title):
    R(x,y,w,h,"#fff",col,2.4,10); T(x+12,y+27,title,18,"700",col,maxw=w-24)

# ===== ① 로봇팔 =====
T(20,36,"① 로봇팔",24,"700"); T(20,66,"어떤 키가 어느 부분을 움직이나",24,"700")
im=Image.open(IMG/'arm-photo.png').convert('RGB')
lut=[min(255,round(v*255/246)) if v>150 else v for v in range(256)]
im=im.point(lut*3); buf=io.BytesIO(); im.save(buf,'JPEG',quality=90); b64=base64.b64encode(buf.getvalue()).decode()
o.append(f'<image x="420" y="215" width="635" height="474" href="data:image/jpeg;base64,{b64}"/>')
J={1:(606,513),2:(708,359),3:(501,354),4:(739,302),5:(848,463),6:(919,525)}
def Jb(n,dx,dy,r=19):
    x,y=J[n]; bx,by=x+dx,y+dy
    o.append(f'<circle cx="{x}" cy="{y}" r="4.5" fill="#fff" stroke="#111827" stroke-width="2"/>')
    if dx or dy: o.append(f'<path d="M{x},{y} L{bx},{by}" stroke="#111827" stroke-width="2"/>')
    o.append(f'<circle cx="{bx}" cy="{by}" r="{r}" fill="#fff" stroke="#111827" stroke-width="2.4"/>')
    T(bx,by+7,str(n),21,"700","#111827","middle")
kx,ky=J[4]
o.append(f'<circle cx="{kx}" cy="{ky}" r="30" fill="none" stroke="{GRN}" stroke-width="3.5" stroke-dasharray="6 4"/>')
o.append(f'<path d="M{kx-38},{ky} L{kx+38},{ky} M{kx},{ky-38} L{kx},{ky+38}" stroke="{GRN}" stroke-width="2.4"/>')
R(kx-172,ky-70,122,32,GRN,"#fff",1.8,16); T(kx-111,ky-48,"IK 포인트",17,"700","#fff","middle")
# 손목 기울기: 수평 기준선 + 손목 방향선 + 각도 호
a=math.radians(56)
o.append(f'<path d="M{kx},{ky} L{kx+190},{ky}" stroke="{AMB}" stroke-width="2.6" stroke-dasharray="3 6" stroke-linecap="round"/>')
o.append(f'<path d="M{kx},{ky} L{kx+215*math.cos(a):.0f},{ky+215*math.sin(a):.0f}" stroke="{AMB}" stroke-width="4.5" stroke-dasharray="12 6" stroke-linecap="round"/>')
o.append(f'<path d="M{kx+110},{ky} A110,110 0 0 1 {kx+110*math.cos(a):.0f},{ky+110*math.sin(a):.0f}" fill="none" stroke="{AMB}" stroke-width="4.5" marker-end="url(#{mk(AMB)})" marker-start="url(#{mk(AMB)})"/>')
R(kx+120,ky-46,116,32,AMB,"#fff",1.8,16); T(kx+178,ky-24,"손목 기울기",17,"700","#fff","middle")
Jb(1,-44,46,20); Jb(2,-36,60,20); Jb(3,-40,-46,20); Jb(4,54,-40); Jb(5,44,-46); Jb(6,-38,50)
px,py=J[1]
o.append(f'<path d="M{px-120},{py+95} A130,28 0 0 0 {px+125},{py+95}" fill="none" stroke="{GRN}" stroke-width="3.5" marker-end="url(#{mk(GRN)})" marker-start="url(#{mk(GRN)})"/>')
rx,ry=J[5]
o.append(f'<g transform="translate({rx},{ry}) rotate(35)"><path d="M0,-40 A14,40 0 0 1 0,40" fill="none" stroke="{PNK}" stroke-width="3.5" marker-end="url(#{mk(PNK)})" marker-start="url(#{mk(PNK)})"/></g>')
gx,gy=J[6]
o.append(f'<path d="M{gx+110},{gy+10} A60,60 0 0 1 {gx+78},{gy+95}" fill="none" stroke="{PNK}" stroke-width="3.5" marker-end="url(#{mk(PNK)})" marker-start="url(#{mk(PNK)})"/>')
# ---- 키 카드
card(755,10,470,176,GRN,"IK 포인트 이동  (손목 점 = ④ 관절 축 위치)")
for i,(l,s) in enumerate([("W","앞"),("S","뒤"),("A","왼쪽"),("D","오른쪽")]): cap(767+i*108,42,l,s)
cap(767,104,"Ctrl+W","위"); cap(875,104,"Ctrl+S","아래")
T(993,140,"x 앞 · y 왼쪽 · z 위",16,"400","#4b5563",maxw=225)
card(395,10,350,158,AMB,"④ wrist_flex  기울기")
for i,(l,s) in enumerate([("↑","기울기 위"),("↓","기울기 아래"),("T","유지 ON/OFF")]): cap(407+i*108,42,l,s)
T(407,122,"목표 기울기를 정한다. 이동 중에도",17,"400","#4b5563",maxw=326); T(407,146,"유지된다 (T로 켜고 끔)",17,"400","#4b5563",maxw=326)
card(1010,200,200,112,PNK,"⑤ wrist_roll")
cap(1018,238,"←","반시계",86); cap(1112,238,"→","시계",86)
card(970,322,240,165,PNK,"⑥ gripper")
cap(978,358,"Ctrl+A","닫기",108); cap(1100,358,"Ctrl+D","열기",100)
T(982,438,"누르는 동안 조금씩",17,"400","#4b5563",maxw=220); T(982,462,"닫히고 열린다",17,"400","#4b5563",maxw=220)
# 자동 계산 안내
R(20,200,395,300,"#f9fafb","#6b7280",2,10,True)
T(34,236,"① ② ③ 은 자동 계산 (역기구학)",21,"700",maxw=368)
for i,t in enumerate(["① shoulder_pan  베이스 회전","② shoulder_lift  어깨","③ elbow_flex  팔꿈치"]): T(34,274+i*32,t,20,maxw=368)
T(34,382,"IK 포인트(손목 점)의 목표 위치에 맞춰",19,"700",GRN,maxw=368)
T(34,410,"①②③ 각도를 로봇이 알아서 정한다.",19,"700",GRN,maxw=368)
T(34,452,"IK 포인트 = ④ 관절 축의 위치",18,"400","#4b5563",maxw=368); T(34,478,"(초록 점선 원)",18,"400","#4b5563")
# 카드 → 부분
A([(880,186),(kx+14,ky-30)],GRN,3)
A([(690,168),(kx-24,ky-18)],AMB,3)
A([(1010,256),(rx+44,ry-30)],PNK,3)
A([(970,420),(gx+24,gy-18)],PNK,3)
# 키 박스 → B1
TR=715
A([(1225,98),(1248,98),(1248,TR)],GRN,3.5,head=False)
A([(1210,256),(1232,256),(1232,TR)],PNK,3.5,head=False)
A([(1210,404),(1232,404),(1232,TR)],PNK,3.5,head=False)
A([(395,88),(375,88),(375,TR)],AMB,3.5,head=False)
A([(1248,TR),(230,TR),(230,R0:=800+58)],"#374151",4)
T(520,TR-12,"키 입력  → B1 키보드입력",21,"700","#374151")
# 좌표축
A([(60,640),(135,640)],"#374151",2); T(142,648,"x 앞",19)
A([(60,640),(60,572)],"#374151",2); T(50,560,"z 위",19)
o.append('<circle cx="60" cy="640" r="9" fill="#fff" stroke="#374151" stroke-width="2"/><circle cx="60" cy="640" r="2.5" fill="#374151"/>'); T(24,680,"y 왼쪽 (화면 앞쪽)",18)
T(270,762,"※ 키를 누르고 있는 동안만 움직이고, 떼면 멈춘다. 두 키를 같이 누르면 대각선으로 움직인다.",21,"400","#4b5563")

# ===== ③ 노드 =====
NX=1285; NW=675
T(NX,36,"③ 노드 (블럭이 어느 노드에 들어 있나)",24,"700")
nodes=[("kb","keyboard_node","C++",[("B1","키보드입력"),("B2","입력해석")]),
 ("ik","ik_node","C++",[("B3","목표위치생성"),("B4","역기구학계산"),("B11","순기구학계산")]),
 ("wr","wrist_node","C++",[("B5","손목기울기계산")]),
 ("dr","direct_node","C++",[("B6","손목회전제어"),("B7","그리퍼제어")]),
 ("mg","merge_node","C++",[("B8","관절명령통합")]),
 ("dv","driver_node","Python",[("B9","액추에이터제어"),("B10","관절상태읽기")])]
y=56; CH=104; ST=114
for k,name,lang,chips in nodes:
    f,s=NODE[k]; R(NX,y,NW,CH,f,s,2,10)
    T(NX+16,y+33,name,24,"700"); T(NX+NW-16,y+32,lang,17,"400","#4b5563","end")
    for i,(bb,n) in enumerate(chips):
        cx=NX+14+i*216; R(cx,y+46,208,46,"#fff",s,1.6,8); T(cx+104,y+76,f"{bb} {n}",18,"600","#111827","middle",maxw=196)
    y+=ST
f,s=NODE["dbg"]; R(NX,y,NW,CH,f,s,2,10,True)
T(NX+16,y+32,"RViz2 / 터미널 로그",24,"700"); T(NX+NW-16,y+32,"디버깅용 (선택)",17,"400","#4b5563","end")
R(NX+14,y+46,260,46,"#fff",s,1.6,8,True); T(NX+144,y+76,"B12 시각화·모니터링",18,"600","#111827","middle",maxw=248)
T(NX,y+CH+34,"블럭 하나는 노드 하나에만 들어 있다.",21,"400","#4b5563")
T(NX,y+CH+60,"같은 노드 안은 함수 호출, 노드 사이는 토픽으로 주고받는다.",21,"400","#4b5563",maxw=NW)

# ===== ② 흐름 구성도 =====
R0=800
T(290,R0+32,"② 흐름 구성도 (블럭)",24,"700")
f,s=NODE["kb"]; R(40,R0+45,230,770,f,s,2,12)
block(50,R0+60,210,54,"kb","B1. 키보드입력",[]); block(50,R0+140,210,54,"kb","B2. 입력해석",[])
A([(155,R0+114),(155,R0+140)]); T(155,R0+800,"keyboard_node",19,"700","#1d4ed8","middle")
LY=[R0+280,R0+440,R0+590,R0+740]
for xk,yy,t,c in zip([245,222,199,176],LY,["이동","기울기","회전","그리퍼"],[GRN,AMB,PNK,PNK]):
    A([(xk,R0+194),(xk,yy),(340,yy)],c,4.5)
    T(xk+10,yy-9,t,19,"700",c,"start")
BH=108
block(340,LY[0]-BH/2,270,BH,"ik","B3. 목표위치생성",["손목 목표점 만들기"])
block(670,LY[0]-BH/2,270,BH,"ik","B4. 역기구학계산",["손목 위치 → 관절각"])
block(340,LY[1]-BH/2,270,BH,"wr","B5. 손목기울기계산",["목표 기울기 유지","wrist_flex 계산"])
block(340,LY[2]-BH/2,270,BH,"dr","B6. 손목회전제어",["손목 회전 목표 만들기"])
block(340,LY[3]-BH/2,270,BH,"dr","B7. 그리퍼제어",["그리퍼 목표 만들기"])
block(1000,R0+215,370,640,"mg","B8. 관절명령통합",["6개 관절 목표를 한 벌로 모은다","관절 한계(캘리브레이션) 안으로 자른다","한 주기 변화량을 제한한다"])
MID=R0+535
block(1420,MID-BH/2,270,BH,"dv","B9. 액추에이터제어",["모터에 목표 위치 전달"])
block(1750,MID-38,150,76,"ext","SO-101",["서보모터 6개"])
FB=R0+935
block(1420,FB-BH/2,270,BH,"dv","B10. 관절상태읽기",["모터 현재 위치 읽기"])
block(1000,FB-BH/2,370,BH,"ik","B11. 순기구학계산",["관절각 → 현재 손목 위치"])
block(650,FB-BH/2,290,BH,"dbg","B12. 시각화·모니터링",["목표·현재 위치 표시","(디버깅용)"],dash=True)
A([(610,LY[0]),(670,LY[0])]); A([(940,LY[0]),(1000,LY[0])])
A([(805,LY[0]+BH/2),(805,LY[1]-24),(610,LY[1]-24)])
A([(610,LY[1]+24),(1000,LY[1]+24)]); A([(610,LY[2]),(1000,LY[2])]); A([(610,LY[3]),(1000,LY[3])])
A([(1370,MID),(1420,MID)]); A([(1690,MID),(1750,MID)])
A([(1825,MID+38),(1825,FB),(1690,FB)]); T(1811,R0+740,"현재 모터 위치",20,"400","#4b5563","end")
A([(1420,FB),(1370,FB)]); A([(1000,FB),(940,FB)],"#6b7280",1.8,True); A([(650,FB),(600,FB)],"#6b7280",1.8,True); T(594,FB+7,"화면",20,"400","#4b5563","end")
T(650,FB+92,"※ 시작할 때만: B11이 구한 현재 위치로 B3·B5·B6·B7의 목표를 맞춘다 (로봇이 튀지 않게)",20,"400","#4b5563")
T(20,H-20,"굵은 색 선 = 키 → B1 → B2 → 해당 블럭 · 가는 회색 화살표 = 데이터 흐름 · 블럭 색 = 들어 있는 노드 · 점선 = 디버깅용",20,"400","#4b5563")

hd=[f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="Noto Sans CJK KR, Noto Sans KR, Malgun Gothic, sans-serif">','<defs>']
hd+=[f'<marker id="{i}" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="{c}"/></marker>' for c,i in markers.items()]
hd+=['</defs>',f'<rect width="{W}" height="{H}" fill="#fff"/>']
OUT.write_text("\n".join(hd+o+["</svg>"]),encoding="utf-8"); print("wrote",OUT)
