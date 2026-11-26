"""driver_node: B9 액추에이터제어 + B10 관절상태읽기.

/joint_command (rad, 그리퍼 0~100) 를 LeRobot SO-101 모터 값(도)으로 바꿔 보내고,
모터 현재 위치를 관절각으로 바꿔 /joint_states 로 발행한다.

  모델 각도(rad) = home_model_rad + sign * radians(LeRobot 각도 - home_deg)

use_mock:=true  이면 로봇 없이 명령을 그대로 상태로 돌려준다.
read_only:=true 이면 토크를 끄고 읽기만 한다 (손으로 움직여 방향/한계 확인).
"""
import math
from pathlib import Path

import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from sensor_msgs.msg import JointState

DEFAULT_NAMES = ['shoulder_pan', 'shoulder_lift', 'elbow_flex', 'wrist_flex', 'wrist_roll',
                 'gripper']


def connect_robot(port, robot_id, calibration_dir, disable_torque_on_exit, logger):
    """LeRobot SO-101 에 연결한다. 캘리브레이션 파일이 없으면 예외."""
    from lerobot.robots.so_follower import SO101Follower, SO101FollowerConfig

    cfg = SO101FollowerConfig(
        port=port,
        id=robot_id,
        calibration_dir=Path(calibration_dir) if calibration_dir else None,
        use_degrees=True,
        disable_torque_on_disconnect=disable_torque_on_exit,
    )
    robot = SO101Follower(cfg)
    if not robot.calibration:
        raise RuntimeError(
            f'캘리브레이션 파일이 없습니다: {robot.calibration_fpath}\n'
            '  scripts/calibrate.sh 로 먼저 캘리브레이션 하세요.')
    robot.connect(calibrate=False)
    if not robot.is_calibrated:
        # 모터에 저장된 값과 파일이 다르면 파일 값을 모터에 쓴다 (lerobot-calibrate 에서 ENTER 와 같음)
        logger.warn('모터의 캘리브레이션 값이 파일과 달라서 파일 값을 모터에 씁니다.')
        robot.bus.write_calibration(robot.calibration)
    return robot


class DriverNode(Node):
    def __init__(self):
        super().__init__('driver_node')
        p = self.declare_parameter
        self.port = p('port', '/dev/ttyACM0').value
        self.robot_id = p('robot_id', 'so101_follower').value
        self.calibration_dir = p('calibration_dir', '').value
        self.use_mock = p('use_mock', False).value
        self.read_only = p('read_only', False).value
        self.disable_torque_on_exit = p('disable_torque_on_exit', False).value
        self.names = list(p('joint_names', DEFAULT_NAMES).value)
        self.home_model = list(p('home_model_rad', [0.0, 1.326, -1.326, 0.0, 0.0]).value)
        self.home_deg = list(p('home_deg', [0.0] * 5).value)
        self.signs = list(p('joint_signs', [1.0] * 5).value)
        mock_gripper = p('mock_gripper', 30.0).value
        rate_hz = p('rate_hz', 50.0).value
        if len(self.names) != 6 or not (len(self.home_model) == len(self.home_deg)
                                        == len(self.signs) == 5):
            raise ValueError('joint_names 는 6개, home_model_rad/home_deg/joint_signs 는 5개여야 합니다')

        self.command = None
        self.robot = None
        if self.use_mock:
            self.mock_state = self.home_model + [mock_gripper]
            self.get_logger().info('가상 드라이버(use_mock) 로 실행합니다.')
        else:
            self.robot = connect_robot(self.port, self.robot_id, self.calibration_dir,
                                       self.disable_torque_on_exit, self.get_logger())
            bus = self.robot.bus
            if self.read_only:
                bus.disable_torque()
                self.get_logger().info('read_only: 토크를 껐습니다. 손으로 움직이며 /joint_states 를 확인하세요.')
            else:
                # 토크가 켜질 때 예전 목표로 튀지 않도록 목표 = 현재 위치
                bus.sync_write('Goal_Position', bus.sync_read('Present_Position'))
            self.get_logger().info(f'SO-101 연결됨 ({self.port}, id={self.robot_id})')

        self.state_pub = self.create_publisher(JointState, 'joint_states', 10)
        self.create_subscription(JointState, 'joint_command', self.on_command, 10)
        self.create_timer(1.0 / rate_hz, self.on_timer)
        if self.read_only:
            self.create_timer(1.0, self.log_angles)
        self.last_deg = None

    # ---- 단위 변환 -------------------------------------------------------
    def to_model(self, deg):
        """LeRobot 값 dict(관절 이름 -> 도, 그리퍼 0~100) -> 모델 값 6개."""
        out = []
        for i, name in enumerate(self.names[:5]):
            out.append(self.home_model[i] + self.signs[i] * math.radians(deg[name] - self.home_deg[i]))
        out.append(deg[self.names[5]])
        return out

    def to_motor(self, model):
        """모델 값 6개 -> LeRobot action dict."""
        action = {}
        for i, name in enumerate(self.names[:5]):
            action[f'{name}.pos'] = (self.home_deg[i] +
                                     self.signs[i] * math.degrees(model[i] - self.home_model[i]))
        action[f'{self.names[5]}.pos'] = model[5]
        return action

    # ---- 콜백 -----------------------------------------------------------
    def on_command(self, msg: JointState):
        index = dict(zip(msg.name, msg.position))
        if all(n in index for n in self.names):
            self.command = [index[n] for n in self.names]

    def on_timer(self):
        try:
            if self.use_mock:
                if self.command is not None:
                    self.mock_state = list(self.command)
                position = self.mock_state
            else:
                obs = self.robot.get_observation()
                self.last_deg = {n: obs[f'{n}.pos'] for n in self.names}
                position = self.to_model(self.last_deg)
                if self.command is not None and not self.read_only:
                    self.robot.send_action(self.to_motor(self.command))
        except Exception as e:  # 통신 오류 한두 번으로 노드가 죽지 않도록
            self.get_logger().warn(f'모터 통신 오류: {e}', throttle_duration_sec=1.0)
            return

        msg = JointState()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.name = self.names
        msg.position = [float(v) for v in position]
        self.state_pub.publish(msg)

    def log_angles(self):
        if self.last_deg is None:
            return
        model = self.to_model(self.last_deg)
        lines = [f'  {n:14s} LeRobot {self.last_deg[n]:8.1f}   모델 {math.degrees(m):8.1f} deg'
                 for n, m in zip(self.names[:5], model)]
        lines.append(f'  {self.names[5]:14s} LeRobot {self.last_deg[self.names[5]]:8.1f}')
        self.get_logger().info('관절각\n' + '\n'.join(lines))

    def close(self):
        if self.robot is not None and self.robot.is_connected:
            self.robot.disconnect()


def main():
    rclpy.init()
    node = None
    try:
        node = DriverNode()
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        pass
    finally:
        if node is not None:
            node.close()
            node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
