"""SO-101 키보드 IK 텔레옵 전체 실행.

예)
  ros2 launch so101_teleop teleop.launch.py                  # 실제 로봇
  ros2 launch so101_teleop teleop.launch.py use_mock:=true   # 로봇 없이 시험
  ros2 launch so101_teleop teleop.launch.py read_only:=true  # 토크 끄고 관절각만 확인
  ros2 launch so101_teleop teleop.launch.py debug:=true      # + 손목 위치 토픽 발행, RViz 표시
"""
import os
from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, EmitEvent, OpaqueFunction, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def find_workspace(share_dir: str) -> Path:
    """install/so101_teleop/share/so101_teleop -> 워크스페이스(= 레포의 프로젝트 폴더)."""
    p = Path(share_dir).resolve()
    for parent in p.parents:
        if parent.name == 'install':
            return parent.parent
    return p.parents[3]


def launch_setup(context):
    share = get_package_share_directory('so101_teleop')
    ws = find_workspace(share)

    arg = lambda name: LaunchConfiguration(name).perform(context)  # noqa: E731
    params_file = arg('params_file') or os.path.join(share, 'config', 'controller_params.yaml')
    robot_config = Path(arg('robot_config_dir') or ws / 'robot_config')
    venv = Path(arg('venv') or os.environ.get('SO101_VENV', '') or ws / '.venv')
    use_mock = arg('use_mock').lower() == 'true'
    debug = arg('debug').lower() == 'true'

    # 로봇별 값: capture_home.sh 가 만든 home_offsets.yaml 이 있으면 덮어쓴다
    params = [params_file]
    home_file = robot_config / 'home_offsets.yaml'
    if home_file.is_file():
        params.append(str(home_file))
    elif not use_mock:
        print(f'[teleop.launch] 경고: {home_file} 이 없습니다. scripts/capture_home.sh 를 먼저 실행하세요.')

    driver_params = params + [{
        'port': arg('port'),
        'robot_id': arg('robot_id'),
        'calibration_dir': str(robot_config / 'calibration'),
        'use_mock': use_mock,
        'read_only': arg('read_only').lower() == 'true',
    }]

    # driver_node 는 lerobot 이 설치된 venv 의 파이썬으로 실행한다
    venv_python = venv / 'bin' / 'python3'
    driver_prefix = None
    if venv_python.is_file():
        driver_prefix = str(venv_python)
    elif not use_mock:
        raise RuntimeError(f'lerobot venv 를 찾을 수 없습니다: {venv}. scripts/setup.sh 를 실행하세요.')

    keyboard = Node(package='so101_teleop', executable='keyboard_node', name='keyboard_node',
                    parameters=params, output='screen')
    nodes = [
        keyboard,
        Node(package='so101_teleop', executable='ik_node', name='ik_node',
             parameters=params + [{'publish_debug': debug}], output='screen'),
        Node(package='so101_teleop', executable='wrist_node', name='wrist_node',
             parameters=params, output='screen'),
        Node(package='so101_teleop', executable='direct_node', name='direct_node',
             parameters=params, output='screen'),
        Node(package='so101_teleop', executable='merge_node', name='merge_node',
             parameters=params, output='screen'),
        Node(package='so101_driver', executable='driver_node', name='driver_node',
             parameters=driver_params, output='screen', prefix=driver_prefix),
        # 키 입력 창을 닫으면 전체 종료
        RegisterEventHandler(OnProcessExit(
            target_action=keyboard, on_exit=[EmitEvent(event=Shutdown(reason='키 입력 창 닫힘'))])),
    ]
    # B12 시각화·모니터링 (디버깅용): debug:=true 일 때만
    if debug:
        # RViz 가 'base' 좌표계를 알도록 world -> base 고정 TF
        nodes.append(Node(package='tf2_ros', executable='static_transform_publisher',
                          name='base_tf', output='log',
                          arguments=['--frame-id', 'world', '--child-frame-id', 'base']))
        nodes.append(Node(package='so101_teleop', executable='viz_node', name='viz_node',
                          parameters=params, output='screen'))
        nodes.append(Node(package='rviz2', executable='rviz2', name='rviz2',
                          arguments=['-d', os.path.join(share, 'config', 'debug.rviz')],
                          output='log'))
    return nodes


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('use_mock', default_value='false',
                              description='true: 로봇 없이 가상 드라이버로 실행'),
        DeclareLaunchArgument('read_only', default_value='false',
                              description='true: 토크를 끄고 관절각만 읽는다 (방향/한계 확인용)'),
        DeclareLaunchArgument('debug', default_value='false',
                              description='true: 손목 위치 토픽 + 팔 막대 그림(viz_node) + RViz 실행 (B12)'),
        DeclareLaunchArgument('port', default_value='/dev/ttyACM0'),
        DeclareLaunchArgument('robot_id', default_value='so101_follower',
                              description='robot_config/calibration/<robot_id>.json'),
        DeclareLaunchArgument('robot_config_dir', default_value='',
                              description='비우면 <워크스페이스>/robot_config'),
        DeclareLaunchArgument('venv', default_value='',
                              description='비우면 $SO101_VENV 또는 <워크스페이스>/.venv'),
        DeclareLaunchArgument('params_file', default_value=''),
        OpaqueFunction(function=launch_setup),
    ])
