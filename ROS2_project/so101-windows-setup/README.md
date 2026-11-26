# SO-101 텔레옵 — Windows 준비 (시연 PC)

> 이 폴더는 **Windows에서 직접 받아서** 쓰는 파일이다. git clone 할 필요 없다.
> ROS 2 패키지는 [`../so101-ros2-ws`](../so101-ros2-ws) 이고, 그건 WSL 안에서 clone 한다. 작품 설명은 [`../so101-keyboard-ik-teleop`](../so101-keyboard-ik-teleop).

Windows에 필요한 것은 3가지다. **설정이 따로 필요한 것은 usbipd 하나뿐**이다.

| 프로그램 | 하는 일 | 설치 후 설정 |
|---|---|---|
| WSL2 + Ubuntu 24.04 | ROS 2를 돌리는 리눅스 | 처음 켤 때 사용자 이름·비밀번호만 정한다 |
| WSLg | 키 입력 창을 Windows 화면에 띄운다 | 없음 (WSL에 들어 있음) |
| **usbipd-win** (이 폴더) | 로봇 USB를 WSL로 넘긴다 | **bind(처음 한 번), attach(연결할 때마다)** |

---

## 1. usbipd-win 받기 (clone 없이)

아래 링크를 누르면 설치 파일이 바로 받아진다.

**👉 [usbipd-win_5.3.0_x64.msi 다운로드](https://github.com/tjdwlswkd21/ros_programming/raw/main/ROS2_project/so101-windows-setup/usbipd-win_5.3.0_x64.msi)**

또는 GitHub 웹에서 이 폴더의 `usbipd-win_5.3.0_x64.msi` 를 누르고, 오른쪽 위 **다운로드 버튼(↓, "Download raw file")** 을 누른다.

- 일반 PC(Intel/AMD)용 x64 파일이다. ARM 노트북이면 [공식 릴리스](https://github.com/dorssel/usbipd-win/releases/tag/v5.3.0)에서 `arm64.msi` 를 받는다.
- 공식 릴리스 v5.3.0 파일 그대로다. SHA-256: `1c984914aec944de19b64eff232421439629699f8138e3ddc29301175bc6d938`
- 인터넷이 되면 PowerShell에서 `winget install --interactive --exact dorssel.usbipd-win` 으로 설치해도 같다.

## 2. 설치

1. 받은 `usbipd-win_5.3.0_x64.msi` 를 더블클릭한다.
2. "Windows의 PC 보호" 창이 뜨면 **추가 정보 → 실행** 을 누른다.
3. 설치 마법사에서 **Next → Install → Finish**. (관리자 권한을 물으면 **예**)
4. **열려 있던 PowerShell 은 모두 닫고 새로 연다.** (그래야 `usbipd` 명령이 잡힌다)

확인 — PowerShell:
```powershell
usbipd --version
```

## 3. WSL2 + Ubuntu 24.04 (이미 있으면 건너뛴다)

PowerShell을 **관리자 권한**으로 열고:
```powershell
wsl --install -d Ubuntu-24.04
```
재부팅 후 Ubuntu 창이 뜨면 사용자 이름과 비밀번호를 정한다.

이미 WSL이 있으면 WSLg가 들어 있는 최신 버전으로만 맞춘다:
```powershell
wsl --update
```

> Ubuntu 안에 **ROS 2 Jazzy** 도 미리 설치해 둬야 한다 ([설치 문서](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html), `ros-jazzy-desktop`).

## 4. 로봇 USB 를 WSL 로 연결 (usbipd 설정)

로봇 컨트롤러 USB 를 꽂고 전원을 켠다. **WSL(Ubuntu) 터미널을 하나 열어 둔다** (WSL이 켜져 있어야 연결된다).

**① 장치 찾기** — PowerShell
```powershell
usbipd list
```
로봇 컨트롤러 줄의 **BUSID** 를 확인한다. 보통 이름이 `USB-Enhanced-SERIAL CH343` 또는 `USB 직렬 장치 (COMx)` 로 보인다.
```
BUSID  VID:PID    DEVICE                                  STATE
2-3    1a86:55d3  USB-Enhanced-SERIAL CH343 (COM9)        Not shared
```
아래 명령의 `2-3` 은 예시다. **자기 PC에서 나온 BUSID** 를 넣는다.

**② 공유 설정 (처음 한 번만)** — PowerShell **관리자 권한**
```powershell
usbipd bind --busid 2-3
```

**③ WSL 로 연결 (로봇을 꽂을 때마다, PC 재부팅 후마다)** — PowerShell (관리자 필요 없음)
```powershell
usbipd attach --wsl --busid 2-3
```

**④ WSL 에서 확인** — Ubuntu 터미널
```bash
ls /dev/ttyACM*        # /dev/ttyACM0 이 보이면 성공
```

**연결 해제** (필요할 때)
```powershell
usbipd detach --busid 2-3
```

## 5. 다음 단계 (WSL 안)

```bash
cd ~ && git clone https://github.com/tjdwlswkd21/ros_programming.git
cd ros_programming/ROS2_project/so101-ros2-ws
./scripts/setup.sh     # 처음 한 번
# WSL 터미널을 새로 연 뒤
./scripts/run.sh
```
자세한 내용은 [ROS 2 워크스페이스 README](../so101-ros2-ws).

---

## 문제가 생기면

| 증상 | 해결 |
|---|---|
| `usbipd` 명령을 찾을 수 없음 | PowerShell 을 새로 연다. 그래도 안 되면 `& "C:\Program Files\usbipd-win\usbipd.exe" list` 처럼 전체 경로로 실행 |
| `attach` 가 실패 | WSL 터미널이 열려 있는지 확인. 안 되면 `wsl --shutdown` 후 Ubuntu 를 다시 열고 attach 다시 |
| `bind` 에서 권한 오류 | PowerShell 을 **관리자 권한**으로 연다 |
| 로봇이 `usbipd list` 에 안 보임 | USB 케이블·로봇 전원 확인, 다른 USB 포트에 꽂기 (포트를 바꾸면 BUSID 도 바뀐다 → bind 다시) |
| `/dev/ttyACM0` 이 없음 | attach 를 다시 한다. 로봇을 뽑았다 꽂으면 attach 는 매번 다시 해야 한다 |
| attach 중 Windows 에서 COM 포트가 안 보임 | 정상이다. attach 하는 동안은 WSL 만 쓸 수 있다 |

---

<sub>usbipd-win 은 Frans van Dorsselaer 의 오픈소스 프로그램이다 (GPL-3.0). 원본·소스: https://github.com/dorssel/usbipd-win</sub>
