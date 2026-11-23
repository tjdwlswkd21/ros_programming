#!/usr/bin/env python3
"""개념설계 블럭도 SVG 생성 스크립트.

실행: python3 tools/make_block_diagram.py
PNG: rsvg-convert -z 1.2 docs/images/block-diagram.svg -o docs/images/block-diagram.png
"""
import pathlib
OUT = pathlib.Path(__file__).resolve().parent.parent / "docs" / "images" / "block-diagram.svg"
W, H = 2740, 1140
BW = 240

STYLE = {
    "in":   ("#dbeafe", "#2563eb"),
    "calc": ("#dcfce7", "#16a34a"),
    "act":  ("#ffedd5", "#ea580c"),
    "fb":   ("#ede9fe", "#7c3aed"),
    "ext":  ("#f3f4f6", "#6b7280"),
}
# id: (x, yc, w, h, 제목, 노드태그, [설명줄], 스타일)
B = {
 "USER":  (20, 560, 100, 64, "사용자", "(키보드)", [], "ext"),
 "B1":  (190, 560, BW, 150, "B1. 키보드입력", "[keyboard_node · C++]",
         ["GLFW 창이 받은 키 이벤트로", "각 키의 눌림/뗌 상태를 갱신한다", "매 주기 지금 눌려 있는 모든 키와",
          "Ctrl 상태를 key_state로 내보낸다"], "in"),
 "B2":  (540, 560, BW, 150, "B2. 입력해석", "[keyboard_node · C++]",
         ["Ctrl이 눌렸으면 Ctrl 명령만 처리", "· Ctrl+W/S → 기울기 증감", "· Ctrl+A/D → 손목 회전",
          "· W/S·A/D → 앞뒤·좌우(x,y)", "· ↑↓ 위·아래(z), ←→ 그리퍼", "· T 토글, 각 방향은 -1 / 0 / +1"], "in"),
 "B3":  (1010, 330, BW, 150, "B3. 목표위치생성", "[ik_node · C++]",
         ["목표 손목 위치(x,y,z)를 들고 있다가", "매 주기: 목표 += 방향×속도×dt", "작업영역 밖이면 그 방향은 무시",
          "시작 때는 current_wrist_pose로", "목표를 맞춰 로봇이 튀지 않게 한다"], "calc"),
 "B4":  (1410, 330, BW, 150, "B4. 역기구학계산", "[ik_node · C++]",
         ["pan = atan2(y, x)", "손목까지의 거리 r, 높이 z로", "코사인법칙 → lift, elbow",
          "(팔꿈치가 위로 가는 해만 사용)", "도달할 수 없으면 직전 값 유지"], "calc"),
 "B5":  (1010, 560, BW, 150, "B5. 손목기울기계산", "[wrist_node · C++]",
         ["목표 기울기를 들고 있다가", "Ctrl+W/S 오면 목표 기울기 ±= 속도×dt", "wrist_flex = 목표 기울기 − (lift+elbow)",
          "T를 끄면 목표를 관절각 자체로 사용", "한계에 걸리면 한계값, 목표는 유지"], "calc"),
 "B6":  (1010, 770, BW, 150, "B6. 손목회전제어", "[direct_node · C++]",
         ["wrist_roll 목표각을 들고 있다가", "Ctrl+A/D 오면 목표 ±= 속도×dt", "관절 한계를 넘지 않게 자른다",
          "시작 때는 현재 관절각으로 맞춘다"], "calc"),
 "B7":  (1010, 950, BW, 150, "B7. 그리퍼제어", "[direct_node · C++]",
         ["gripper 목표값을 들고 있다가", "←(닫기) / →(열기)가", "눌린 동안 목표 ±= 속도×dt",
          "열림·닫힘 한계를 넘지 않게 자른다"], "calc"),
 "B8":  (1820, 610, BW, 200, "B8. 관절명령통합", "[merge_node · C++]",
         ["B4·B5·B6·B7의 목표값을 모아", "6개 관절 순서의 한 벌로 만든다", "① 각 값을 joint_limits로 자른다",
          "② 한 주기 최대 변화량을 넘으면", "   그만큼만 움직이게 제한한다", "③ joint_command로 내보낸다"], "calc"),
 "B9":  (2210, 610, BW, 150, "B9. 액추에이터제어", "[driver_node · Python]",
         ["joint_command의 각도를", "LeRobot 단위(보정값 적용)로 바꿔", "서보모터에 목표 위치로 보낸다"], "act"),
 "MOTOR": (2580, 610, 110, 64, "SO-101", "서보모터 6개", [], "ext"),
 "B10": (2210, 90, BW, 150, "B10. 관절상태읽기", "[driver_node · Python]",
         ["서보모터의 현재 위치를 읽어", "각도(rad)로 바꿔 joint_state로", "매 주기 내보낸다"], "fb"),
 "B11": (1010, 90, BW, 150, "B11. 순기구학계산", "[ik_node · C++]",
         ["joint_state의 pan·lift·elbow로", "손목 점(4번째 관절 축) 위치를", "링크 길이로 계산한다",
          "→ current_wrist_pose", "(역기구학의 반대 방향 계산)"], "fb"),
 "B12": (300, 90, BW, 150, "B12. 시각화·모니터링", "[디버깅용 · 선택]",
         ["목표·현재 손목 위치를", "화면(터미널 로그 또는 RViz2)에", "표시한다. 조작에는 필요 없다"], "fb"),
 "SCREEN": (20, 90, 100, 64, "사용자", "(화면)", [], "ext"),
 "CAL": (1820, 950, BW, 64, "캘리브레이션", "관절 최대각 값 (joint_limits)", [], "ext"),
}
DASHED_BLOCKS = {"B12", "SCREEN"}

# (점들, 이름, 주석, 라벨위치, dashed)
# 라벨위치: ("h", 구간번호, 가로중심x)  가로 구간: 이름은 선 위, 주석은 선 아래
#           ("r", 구간번호)  세로 구간: 선 오른쪽 / ("l", 구간번호) 왼쪽
E = [
 ([(120,560),(190,560)], "키 누름·뗌", "", ("h",0,155), False),
 ([(430,560),(540,560)], "key_state", "눌린 키·Ctrl", ("h",0,485), False),
 ([(780,560),(830,560),(830,330),(1010,330)], "move_vel_cmd", "x·y·z 이동 방향 (-1/0/+1)", ("h",2,920), False),
 ([(780,560),(1010,560)], "tilt_cmd", "Ctrl+W/S 기울기 · T 토글", ("h",0,925), False),
 ([(780,560),(830,560),(830,770),(1010,770)], "roll_cmd", "Ctrl+A/D 회전 방향 (-1/0/+1)", ("h",2,920), False),
 ([(780,560),(830,560),(830,950),(1010,950)], "gripper_cmd", "← 닫기 · → 열기", ("h",2,920), False),
 ([(1250,330),(1410,330)], "target_wrist_pos", "손목 점 목표 (x,y,z)", ("h",0,1330), False),
 ([(1530,405),(1530,455),(1130,455),(1130,485)], "arm_joint_target", "팔 관절 목표 (pan·lift·elbow)", ("h",1,1330), False),
 ([(1650,330),(1740,330),(1740,545),(1820,545)], "arm_joint_target", "pan·lift·elbow", ("l",1), False),
 ([(1250,580),(1820,580)], "wrist_flex_target", "4번째 관절 목표각", ("h",0,1500), False),
 ([(1250,770),(1750,770),(1750,615),(1820,615)], "wrist_roll_target", "5번째 관절 목표각", ("h",0,1490), False),
 ([(1250,950),(1790,950),(1790,650),(1820,650)], "gripper_target", "그리퍼 목표 열림값", ("h",0,1520), False),
 ([(1940,918),(1940,710)], "joint_limits", "관절별 최소·최대각", ("r",0), False),
 ([(2060,610),(2210,610)], "joint_command", "6개 관절 목표각 (rad)", ("h",0,2135), False),
 ([(2450,610),(2580,610)], "motor_goal_position", "각도→모터 값", ("h",0,2515), False),
 ([(2635,578),(2635,90),(2450,90)], "motor_present_position", "현재 모터 위치", ("h",1,2545), False),
 ([(2210,90),(1250,90)], "joint_state", "현재 관절각 (rad)", ("h",0,1730), False),
 ([(1130,165),(1130,255)], "current_wrist_pose", "현재 손목 위치", ("r",0), False),
 ([(1010,90),(540,90)], "current_wrist_pose", "현재 손목 위치", ("h",0,775), True),
 ([(1050,255),(1050,200),(420,200),(420,165)], "target_wrist_pos", "목표 손목 위치", ("h",1,735), True),
 ([(300,90),(120,90)], "현재·목표 위치 표시", "", ("h",0,210), True),
]
def tw(s, size):
    return sum(size * (0.62 if ord(c) < 128 else 1.0) for c in s)
def esc(s):
    return s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")

o=[]
o.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W}" height="{H}" font-family="Noto Sans CJK KR, Noto Sans KR, Malgun Gothic, sans-serif">')
o.append('<defs><marker id="arr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#374151"/></marker>'
         '<marker id="arrg" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="#6b7280"/></marker></defs>')
o.append(f'<rect width="{W}" height="{H}" fill="#fff"/>')
for pts,_,_,_,dash in E:
    d="M"+" L".join(f"{x},{y}" for x,y in pts)
    col,mk=("#6b7280","arrg") if dash else ("#374151","arr")
    da=' stroke-dasharray="6 5"' if dash else ""
    o.append(f'<path d="{d}" fill="none" stroke="{col}" stroke-width="1.8"{da} stroke-linejoin="round" marker-end="url(#{mk})"/>')
for bid,(x,yc,w,h,title,tag,lines,st) in B.items():
    fill,stroke=STYLE[st]; y=yc-h/2
    da=' stroke-dasharray="7 5"' if bid in DASHED_BLOCKS else ""
    o.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{32 if st=="ext" else 6}" fill="{fill}" stroke="{stroke}" stroke-width="2"{da}/>')
    cx=x+w/2
    if not lines:
        o.append(f'<text x="{cx}" y="{yc-4}" text-anchor="middle" font-size="15" font-weight="700" fill="#111827">{esc(title)}</text>')
        o.append(f'<text x="{cx}" y="{yc+15}" text-anchor="middle" font-size="11" fill="#4b5563">{esc(tag)}</text>')
    else:
        o.append(f'<text x="{cx}" y="{y+22}" text-anchor="middle" font-size="15" font-weight="700" fill="#111827">{esc(title)}</text>')
        o.append(f'<text x="{cx}" y="{y+38}" text-anchor="middle" font-size="10.5" fill="#6b7280">{esc(tag)}</text>')
        for i,t in enumerate(lines):
            o.append(f'<text x="{x+10}" y="{y+60+i*14.5}" font-size="11" fill="#1f2937">{esc(t)}</text>')
def lab(x,y,t,size,anchor,fam,color):
    w=tw(t,size)
    bx = x-w/2-3 if anchor=="middle" else (x-3 if anchor=="start" else x-w-3)
    o.append(f'<rect x="{bx}" y="{y-size+1}" width="{w+6}" height="{size+4}" fill="#fff" opacity="0.93"/>')
    o.append(f'<text x="{x}" y="{y}" text-anchor="{anchor}" font-size="{size}" {fam} fill="{color}">{esc(t)}</text>')
MONO='font-family="DejaVu Sans Mono, monospace"'
for pts,name,com,pos,_ in E:
    fam = MONO if name.isascii() else ""
    if pos[0]=="h":
        k,cx=pos[1],pos[2]; Y=pts[k][1]
        lab(cx,Y-6,name,12,"middle",fam,"#111827")
        if com: lab(cx,Y+16,com,10.5,"middle","","#4b5563")
    else:
        k=pos[1]; (x1,y1),(x2,y2)=pts[k],pts[k+1]; ym=(y1+y2)/2
        if pos[0]=="r": lab(x1+8,ym,name,12,"start",fam,"#111827"); lab(x1+8,ym+14,com,10.5,"start","","#4b5563")
        else: lab(x1-8,ym,name,12,"end",fam,"#111827"); lab(x1-8,ym+14,com,10.5,"end","","#4b5563")
notes=["※ 시작할 때 B3·B5·B6·B7은 현재 상태(current_wrist_pose, joint_state)로 목표를 맞추고 시작한다. 처음에 로봇이 튀지 않게 하기 위해서다. (joint_state 화살표는 선을 줄이려고 B11만 그렸다)",
       "※ 역기구학 기준점 = 4번째 관절(wrist_flex 축)의 위치(손목 점). B5~B7도 joint_limits를 같은 설정 파일에서 읽어 목표가 한계를 넘지 않게 한다.",
       "※ 같은 노드 안의 블럭끼리는 함수 호출로, 노드 사이는 토픽으로 데이터를 주고받는다. 블럭 이름 아래 [ ]는 그 블럭이 들어가는 노드다."]
for i,t in enumerate(notes):
    o.append(f'<text x="20" y="{1068+i*22}" font-size="13" fill="#4b5563">{esc(t)}</text>')
lx,y0=2200,880
for i,(st,n) in enumerate([("in","입력·해석"),("calc","목표 만들기·계산"),("act","구동"),("fb","상태·표시"),("ext","블럭 밖 (사람·하드웨어)")]):
    f,s=STYLE[st]; y=y0+i*26
    o.append(f'<rect x="{lx}" y="{y}" width="16" height="16" rx="3" fill="{f}" stroke="{s}" stroke-width="1.5"/>')
    o.append(f'<text x="{lx+24}" y="{y+13}" font-size="12.5" fill="#374151">{n}</text>')
o.append(f'<text x="{lx}" y="{y0+5*26+14}" font-size="12.5" fill="#374151">화살표 = 데이터 흐름 (굵은 글씨 = 데이터 이름, 작은 글씨 = 설명)</text>')
o.append(f'<text x="{lx}" y="{y0+5*26+34}" font-size="12.5" fill="#374151">점선 = 디버깅용(선택)</text>')
o.append("</svg>")
OUT.write_text("\n".join(o),encoding="utf-8"); print("wrote",OUT)
