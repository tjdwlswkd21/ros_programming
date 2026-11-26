#!/usr/bin/env bash
# 새 PC(WSL2 Ubuntu 24.04 + ROS 2 Jazzy)에서 클론한 뒤 한 번 실행: 의존성 설치 + 빌드
#   ./scripts/setup.sh
set -eo pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$WS"

if [ ! -f /opt/ros/jazzy/setup.bash ]; then
  echo "ROS 2 Jazzy(/opt/ros/jazzy)가 없습니다. 먼저 ROS 2 Jazzy 를 설치하세요." >&2
  exit 1
fi
source /opt/ros/jazzy/setup.bash

echo "=== [1/4] apt + rosdep: package.xml 의 의존성 설치 ==="
sudo apt-get update
sudo apt-get install -y python3-rosdep python3-venv python3-pip python3-colcon-common-extensions
[ -f /etc/ros/rosdep/sources.list.d/20-default.list ] || sudo rosdep init
rosdep update --rosdistro jazzy
rosdep install --from-paths src --ignore-src -y --rosdistro jazzy

echo "=== [2/4] .venv + requirements.txt: lerobot 설치 (처음엔 몇 분 걸림) ==="
[ -x .venv/bin/python3 ] || /usr/bin/python3 -m venv --system-site-packages .venv
.venv/bin/pip install --upgrade pip
.venv/bin/pip install -r requirements.txt

echo "=== [3/4] colcon build ==="
colcon build --symlink-install

echo "=== [4/4] 시리얼 포트 권한 (dialout 그룹) ==="
if id -nG "$USER" | grep -qw dialout; then
  echo "이미 dialout 그룹입니다."
else
  sudo usermod -aG dialout "$USER"
  echo "dialout 그룹에 추가했습니다. WSL 터미널을 새로 열어야 적용됩니다."
fi

echo
echo "완료. 실행: ./scripts/run.sh   (로봇 없이 시험: ./scripts/run.sh use_mock:=true)"
[ -f robot_config/calibration/so101_follower.json ] || \
  echo "주의: robot_config/calibration/so101_follower.json 이 없습니다. ./scripts/calibrate.sh 를 먼저 하세요."
[ -f robot_config/home_offsets.yaml ] || \
  echo "주의: robot_config/home_offsets.yaml 이 없습니다. ./scripts/capture_home.sh 를 먼저 하세요."
