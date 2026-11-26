#!/usr/bin/env bash
# 전체 실행. 인자는 launch 인자로 넘어간다.
#   ./scripts/run.sh                 실제 로봇
#   ./scripts/run.sh use_mock:=true  로봇 없이 시험
#   ./scripts/run.sh read_only:=true 토크 끄고 관절각 확인
#   ./scripts/run.sh port:=/dev/ttyACM1
set -eo pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [ ! -f "$WS/install/setup.bash" ]; then
  echo "빌드가 안 되어 있습니다. ./scripts/setup.sh 를 먼저 실행하세요." >&2
  exit 1
fi
source /opt/ros/jazzy/setup.bash
source "$WS/install/setup.bash"
exec ros2 launch so101_teleop teleop.launch.py "$@"
