#!/usr/bin/env bash
# LeRobot 캘리브레이션 (로봇마다 한 번). 결과는 robot_config/calibration/<id>.json 에 저장되니 git 에 커밋하세요.
#   ./scripts/calibrate.sh [포트=/dev/ttyACM0] [id=so101_follower]
set -eo pipefail
WS="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PORT="${1:-/dev/ttyACM0}"
ID="${2:-so101_follower}"
exec "$WS/.venv/bin/lerobot-calibrate" \
  --robot.type=so101_follower \
  --robot.port="$PORT" \
  --robot.id="$ID" \
  --robot.calibration_dir="$WS/robot_config/calibration"
