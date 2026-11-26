"""home 자세의 LeRobot 각도를 읽어 robot_config/home_offsets.yaml 로 저장한다.

home 자세 = LeRobot 캘리브레이션 그림 자세: 상완 수직, 전완 수평 앞, 손목 일자, 손목 회전 가운데.
이 값이 있어야 IK 모델 각도와 모터 각도가 맞는다.
"""
import argparse
from pathlib import Path

from so101_driver.driver_node import DEFAULT_NAMES, connect_robot


class _PrintLogger:
    def warn(self, msg):
        print('경고:', msg)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--port', default='/dev/ttyACM0')
    ap.add_argument('--robot-id', default='so101_follower')
    ap.add_argument('--calibration-dir', required=True)
    ap.add_argument('--out', required=True)
    args = ap.parse_args()

    robot = connect_robot(args.port, args.robot_id, args.calibration_dir, True, _PrintLogger())
    try:
        robot.bus.disable_torque()
        print('\n토크를 껐습니다. 팔을 손으로 home 자세로 맞추세요:')
        print('  상완은 수직으로 세우고, 전완은 앞으로 수평, 손목은 전완과 일자, 손목 회전은 가운데.')
        input('맞췄으면 ENTER ')
        obs = robot.get_observation()
        home = [round(float(obs[f'{n}.pos']), 2) for n in DEFAULT_NAMES[:5]]
    finally:
        robot.disconnect()

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(
        '# scripts/capture_home.sh 가 만든 파일. home 자세에서 읽은 LeRobot 각도(도)\n'
        'driver_node:\n'
        '  ros__parameters:\n'
        f'    home_deg: [{", ".join(str(v) for v in home)}]\n')
    print(f'\n저장했습니다: {out}')
    for n, v in zip(DEFAULT_NAMES, home):
        print(f'  {n:14s} {v:8.2f}')


if __name__ == '__main__':
    main()
