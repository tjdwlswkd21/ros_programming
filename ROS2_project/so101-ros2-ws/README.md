# SO-101 키보드 IK 텔레옵 — ROS 2 워크스페이스

> 작품 설명·설계: [`../so101-keyboard-ik-teleop`](../so101-keyboard-ik-teleop) · Windows 준비(usbipd-win): [`../so101-windows-setup`](../so101-windows-setup)

이 폴더(`so101-ros2-ws`)가 그대로 colcon 워크스페이스다. 필요 환경: WSL2 Ubuntu 24.04 + ROS 2 Jazzy + WSLg.
로봇 USB 는 Windows 에서 usbipd 로 WSL 에 attach 해 둔다 ([Windows 준비](../so101-windows-setup)).

## 처음 한 번 (새 PC)

```bash
cd ~ && git clone https://github.com/tjdwlswkd21/ros_programming.git
cd ros_programming/ROS2_project/so101-ros2-ws
./scripts/setup.sh
```

`setup.sh`가 하는 일:
1. `rosdep install` — `package.xml`에 적힌 의존성(GLFW, OpenGL, ROS 메시지 등)을 apt로 설치
2. `.venv` 생성 + `requirements.txt`(lerobot) 설치 — `--system-site-packages`라 ROS 파이썬과 같이 쓴다
3. `colcon build --symlink-install`
4. 시리얼 포트 권한(dialout 그룹)

> WSL은 Windows 폴더(`/mnt/c/...`)가 아니라 WSL 홈(`~`)에 클론해야 빌드가 빠르다.

## 실행

USB 를 attach 한 뒤, **새로 연 WSL 터미널**에서 (포트 권한 적용):

```bash
cd ~/ros_programming/ROS2_project/so101-ros2-ws
./scripts/run.sh                    # 실제 로봇
./scripts/run.sh use_mock:=true     # 로봇 없이 시험 (가상 드라이버)
./scripts/run.sh read_only:=true    # 토크 끄고 관절각만 읽기 (손으로 움직여 방향/한계 확인)
./scripts/run.sh port:=/dev/ttyACM1 # 포트가 다를 때
./scripts/run.sh debug:=true        # 디버그(B12): 팔 막대 그림 + 손목 위치를 RViz 로 표시
```
옵션은 같이 쓸 수 있다. 로봇 없이 화면으로만 보려면(시뮬레이션):
```bash
./scripts/run.sh use_mock:=true debug:=true
```

**debug 옵션**
- 기본(옵션 없음): 제어에 필요한 노드 6개만 실행한다. 디버그용 노드·토픽은 아예 만들지 않고 RViz 도 띄우지 않는다.
- `debug:=true`: 아래가 추가로 실행되고 RViz 가 설정(`config/debug.rviz`)을 불러와 함께 뜬다.
  - `viz_node`: 현재 관절각으로 팔 모양을 막대 그림으로 그린다 (`/arm_markers`, IK 와 같은 공식). 그리퍼 끝 위에 열림 값 표시
  - `ik_node`: 목표·현재 손목 위치 발행 → 빨간 점 = 목표, 초록 점 = 현재(지나온 자취 포함)
  - 바닥 격자 한 칸 = 5 cm, 빨강/초록/파랑 축 = x(앞)/y(왼쪽)/z(위)

| 옵션 | 뜻 |
|---|---|
| `use_mock:=true` | 팔로워 암 없이 가상 로봇으로 실행 (없으면 실제 팔이 연결돼 있어야 한다) |
| `debug:=true` | RViz 로 팔 모양·손목 위치 표시 (실제 팔, 가상 로봇 모두 가능) |

키 입력 창이 Windows 화면에 뜨면 클릭해서 포커스를 주고 조작한다. 창 색: 회색 = 포커스 없음(정지), 파랑 = 대기, 초록 = 동작 중. 창을 닫거나 터미널에서 Ctrl+C 를 누르면 전체가 종료된다.

| 키 | 동작 |
|---|---|
| `W` / `S` | 손목 앞 / 뒤 |
| `A` / `D` | 손목 왼쪽 / 오른쪽 |
| `↑` / `↓` | 손목 위 / 아래 |
| `Ctrl+W` / `Ctrl+S` | 목표 기울기 + / − |
| `Ctrl+A` / `Ctrl+D` | 손목 회전 |
| `←` / `→` | 그리퍼 닫기 / 열기 |
| `T` | 기울기 유지 켜기 / 끄기 |

## 로봇별 설정 (로봇마다 한 번, 결과를 git에 커밋)

| 파일 | 만드는 방법 | 내용 |
|---|---|---|
| `robot_config/calibration/so101_follower.json` | `./scripts/calibrate.sh` | LeRobot 캘리브레이션 (모터 범위) |
| `robot_config/home_offsets.yaml` | `./scripts/capture_home.sh` | home 자세(상완 수직, 전완 수평 앞, 손목 일자)의 모터 각도 → IK 각도와 맞추는 기준 |

지금 들어 있는 두 파일은 이 프로젝트의 로봇에서 실측한 값이다. 같은 로봇이면 다시 할 필요가 없다.
관절 회전 방향(`joint_signs`), 관절 한계(`joint_min/max`), 링크 길이, 속도는 `src/so101_teleop/config/controller_params.yaml`에서 바꾼다.

## 구성

```
so101-ros2-ws/
├── scripts/            setup.sh, run.sh, calibrate.sh, capture_home.sh
├── robot_config/       캘리브레이션, home 자세 값
├── requirements.txt    lerobot (pip)
└── src/
    ├── so101_teleop/   (C++)    keyboard_node, ik_node, wrist_node, direct_node, merge_node
    │                            viz_node (debug 때만)
    │                            launch/teleop.launch.py, config/controller_params.yaml
    └── so101_driver/   (Python) driver_node (LeRobot), capture_home
```

| 토픽 | 타입 | 내용 |
|---|---|---|
| `/move_cmd` | `geometry_msgs/Vector3` | 손목 이동 방향 x, y, z (−1/0/+1) |
| `/tilt_cmd` | `std_msgs/Int8MultiArray` | [기울기 방향 −1/0/+1, 기울기 유지 0/1] |
| `/roll_cmd`, `/gripper_cmd` | `std_msgs/Int8` | −1/0/+1 |
| `/arm_target` | `sensor_msgs/JointState` | IK 결과 pan, lift, elbow (rad) |
| `/wrist_flex_target`, `/roll_target`, `/gripper_target` | `std_msgs/Float64` | 관절 목표 |
| `/joint_command`, `/joint_states` | `sensor_msgs/JointState` | 6개 관절 명령 / 현재값 (rad, 그리퍼 0~100) |
| `/target_wrist_pos`, `/current_wrist_pose` | `geometry_msgs/PointStamped` | 목표 / 현재 손목 위치 (`debug:=true` 일 때만) |
| `/arm_markers` | `visualization_msgs/MarkerArray` | 팔 막대 그림 (`debug:=true` 일 때만, viz_node) |
