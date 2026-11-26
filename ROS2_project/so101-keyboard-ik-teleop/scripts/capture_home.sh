#!/usr/bin/env bash
# home 자세의 모터 각도를 robot_config/home_offsets.yaml 로 저장 (calibrate.sh 다음에 한 번). git 에 커밋하세요.
#   ./scripts/capture_home.sh [포트=/dev/ttyACM0] [id=so101_follower]
set -eo pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source /opt/ros/jazzy/setup.bash
source "$WS/install/setup.bash"
exec "$WS/.venv/bin/python3" -m so101_driver.capture_home \
  --port "${1:-/dev/ttyACM0}" \
  --robot-id "${2:-so101_follower}" \
  --calibration-dir "$WS/robot_config/calibration" \
  --out "$WS/robot_config/home_offsets.yaml"
