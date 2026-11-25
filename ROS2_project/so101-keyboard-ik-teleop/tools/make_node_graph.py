#!/usr/bin/env python3
import pathlib
OUT = pathlib.Path(__file__).resolve().parent.parent / "docs" / "images" / "node-graph.svg"
W, H = 2490, 1050
STYLE = {"in":("#dbeafe","#2563eb"),"calc":("#dcfce7","#16a34a"),"act":("#ffedd5","#ea580c"),
         "fb":("#ede9fe","#7c3aed"),"ext":("#f3f4f6","#6b7280"),"file":("#fef9c3","#ca8a04")}
o=[]
o.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="Noto Sans CJK KR, Noto Sans KR, Malgun Gothic, sans-serif">')
o.append('<defs><marker id="arr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#374151"/></marker>'
         '<marker id="arrg" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#6b7280"/></marker></defs>')
o.append(f'<rect width="{W}" height="{H}" fill="#fff"/>')
def tw(s,size): return sum(size*(0.62 if ord(c)<128 else 1.0) for c in s)
def esc(s): return s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")
def box(x,yc,w,h,st,title,subs,rx=8,dashed=False):
    f,s=STYLE[st]; y=yc-h/2; d=' stroke-dasharray="6 5"' if dashed else ""
    o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{f}" stroke="{s}" stroke-width="2"{d}/>')
    cx=x+w/2; n=len(subs); top=yc-(n*8)-2
    o.append(f'<text x="{cx}" y="{top}" text-anchor="middle" font-size="16" font-weight="700" fill="#111827">{esc(title)}</text>')
    for i,t in enumerate(subs):
        o.append(f'<text x="{cx}" y="{top+19+i*15}" text-anchor="middle" font-size="11.5" fill="#4b5563">{esc(t)}</text>')
def arrow(pts,dashed=False):
    d="M"+" L".join(f"{x},{y}" for x,y in pts)
    c,m=("#6b7280","arrg") if dashed else ("#374151","arr")
    da=' stroke-dasharray="6 5"' if dashed else ""
    o.append(f'<path d="{d}" fill="none" stroke="{c}" stroke-width="1.8"{da} stroke-linejoin="round" marker-end="url(#{m})"/>')
def t(x,y,s,size,anchor,mono,color):
    w=tw(s,size); bx=x-w/2-3 if anchor=="middle" else (x-3 if anchor=="start" else x-w-3)
    fam='font-family="DejaVu Sans Mono, monospace"' if mono else ""
    o.append(f'<rect x="{bx}" y="{y-size+1}" width="{w+6}" height="{size+4}" fill="#fff" opacity="0.93"/>')
    o.append(f'<text x="{x}" y="{y}" text-anchor="{anchor}" font-size="{size}" {fam} fill="{color}">{esc(s)}</text>')
LS,CS=17,14.5   # 박스 밖 글씨: 토픽 이름 / 메시지 타입
def hl(cx,Y,name,typ=None,mono=True):  # 가로선: 이름은 위, 타입은 아래
    t(cx,Y-8,name,LS,"middle",mono,"#111827")
    if typ: t(cx,Y+22,typ,CS,"middle",True,"#4b5563")
def vl(x,ym,name,typ=None,anchor="start"):
    t(x,ym,name,LS,anchor,True,"#111827")
    if typ: t(x,ym+21,typ,CS,anchor,True,"#4b5563")

o.append('<rect x="200" y="10" width="2010" height="850" rx="14" fill="#fafafa" stroke="#9ca3af" stroke-width="1.6" stroke-dasharray="8 6"/>')
o.append('<text x="220" y="42" font-size="20" font-weight="700" fill="#4b5563">WSL2 Ubuntu · ROS 2</text>')

box(10,380,120,64,"ext","사용자",["(키보드)"],rx=30)
box(2320,380,130,64,"ext","SO-101",["서보모터 6개"],rx=30)
box(240,380,230,90,"in","keyboard_node",["C++ · GLFW 창  (B1, B2)","키 상태 읽기 → 명령으로 변환"])
box(840,190,260,100,"calc","ik_node",["C++  (B3, B4, B11)","목표 손목 위치 · 역기구학 · 순기구학"])
box(840,380,260,100,"calc","wrist_node",["C++  (B5)","목표 기울기 유지 → wrist_flex 계산"])
box(840,620,260,100,"calc","direct_node",["C++  (B6, B7)","wrist_roll · gripper 목표 만들기"])
box(1370,380,260,100,"calc","merge_node",["C++  (B8)","6개 관절 취합 · 한계·변화량 제한"])
box(1900,380,250,100,"act","driver_node",["Python · LeRobot  (B9, B10)","모터 명령 전달 · 현재 위치 읽기"])
box(1330,150,240,100,"fb","RViz2  (B12)",["디버깅용 · 선택"],dashed=True)
box(1780,150,200,100,"fb","robot_state_publisher",["URDF → tf · 디버깅용"],dashed=True)
box(1430,790,300,80,"file","controller_params.yaml",["속도 · 작업영역 · 관절 한계","기울기 유지 기본값"])

arrow([(130,380),(240,380)]); t(185,364,"키 입력",LS,"middle",False,"#111827")
# keyboard → 3 nodes
arrow([(470,380),(520,380),(520,190),(840,190)]); hl(680,190,"/move_cmd","geometry_msgs/Vector3")
arrow([(470,380),(840,380)]); hl(680,380,"/tilt_cmd","sensor_msgs/Joy")
arrow([(520,380),(520,620),(840,620)]); hl(680,620,"/roll_cmd · /gripper_cmd","std_msgs/Float32 (각각)")
# ik
arrow([(970,240),(970,330)]); vl(982,282,"/arm_target","sensor_msgs/JointState")
arrow([(1100,232),(1500,232),(1500,330)]); hl(1300,232,"/arm_target","sensor_msgs/JointState")
# wrist, direct → merge
arrow([(1100,380),(1370,380)]); hl(1235,380,"/wrist_flex_target","std_msgs/Float32")
arrow([(1100,620),(1500,620),(1500,430)]); hl(1300,620,"/roll_target · /gripper_target","std_msgs/Float32 (각각)")
# merge → driver
arrow([(1630,380),(1900,380)]); hl(1765,380,"/joint_command","sensor_msgs/JointState")
# driver ↔ robot
arrow([(2150,360),(2320,360)]); t(2235,346,"모터 목표 위치",LS,"middle",False,"#111827")
arrow([(2320,405),(2150,405)]); t(2235,434,"모터 현재 위치",LS,"middle",False,"#111827")
t(2385,442,"USB 시리얼 (usbipd)",CS,"middle",False,"#4b5563")
# joint_states trunk → ik
arrow([(2025,330),(2025,62),(970,62),(970,140)]); hl(1500,62,"/joint_states","sensor_msgs/JointState")
# debug (dashed)
arrow([(1100,170),(1330,170)],True); t(1215,155,"/target_wrist_pos",LS,"middle",True,"#111827"); t(1215,198,"/current_wrist_pose",LS,"middle",True,"#111827")
arrow([(2025,150),(1980,150)],True)
arrow([(1780,150),(1570,150)],True); hl(1675,150,"/tf")
# params
arrow([(1580,750),(1580,430)],True); vl(1592,590,"ROS 2 파라미터","(C++ 노드 4개가 읽음)")
notes=["※ 점선 박스·화살표는 디버깅용(선택)이다. 조작에는 필요 없고, 목표·현재 손목 위치와 관절각은 터미널 로그로도 확인한다.",
 "※ /joint_states는 wrist_node·direct_node도 구독한다(시작 때 목표를 현재 관절각에 맞춤). 선을 줄이려고 ik_node만 그렸다.",
 "※ GLFW 창은 WSLg가 Windows 화면에 띄워준다 (우분투 데스크톱 불필요). 키 안내·경고는 노드를 실행한 터미널에 출력한다.",
 "※ 노드 안의 블럭끼리는 함수 호출로 데이터를 주고받고, 노드 사이만 토픽을 쓴다. 토픽 아래 작은 글씨는 메시지 타입(안)이다."]
for i,s_ in enumerate(notes): o.append(f'<text x="220" y="{895+i*30}" font-size="17" fill="#4b5563">{esc(s_)}</text>')
o.append('<text x="1900" y="790" font-size="16" fill="#374151">실선 = 필수 토픽</text>')
o.append('<text x="1900" y="820" font-size="16" fill="#374151">점선 = 디버깅용(선택)·설정 파일</text>')
o.append("</svg>")
OUT.write_text("\n".join(o),encoding="utf-8"); print("wrote",OUT)
