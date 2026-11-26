// 여러 노드가 같이 쓰는 관절 이름, 관절 한계, 명령 타임아웃
#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"

namespace so101_teleop
{

enum Joint { PAN = 0, LIFT, ELBOW, WRIST_FLEX, WRIST_ROLL, GRIPPER, NUM_JOINTS };

inline const std::vector<std::string> & joint_names()
{
  static const std::vector<std::string> names = {
    "shoulder_pan", "shoulder_lift", "elbow_flex", "wrist_flex", "wrist_roll", "gripper"};
  return names;
}

// JointState 메시지에서 관절 하나의 값을 이름으로 찾는다
inline bool find_joint(const sensor_msgs::msg::JointState & msg, Joint j, double & out)
{
  const auto & name = joint_names()[j];
  for (size_t i = 0; i < msg.name.size() && i < msg.position.size(); ++i) {
    if (msg.name[i] == name) {
      out = msg.position[i];
      return true;
    }
  }
  return false;
}

// 관절 한계 (controller_params.yaml 의 joint_min / joint_max)
struct JointLimits
{
  std::vector<double> min;
  std::vector<double> max;

  double clamp(Joint j, double v) const {return std::clamp(v, min[j], max[j]);}
  // 한계를 벗어난 양 (안쪽이면 0)
  double excess(Joint j, double v) const
  {
    return std::max(0.0, min[j] - v) + std::max(0.0, v - max[j]);
  }
};

inline JointLimits declare_joint_limits(rclcpp::Node & node)
{
  JointLimits l;
  l.min = node.declare_parameter("joint_min", std::vector<double>{});
  l.max = node.declare_parameter("joint_max", std::vector<double>{});
  if (l.min.size() != NUM_JOINTS || l.max.size() != NUM_JOINTS) {
    throw std::runtime_error("joint_min / joint_max 는 관절 6개 값이 필요합니다 (controller_params.yaml)");
  }
  return l;
}

// 키를 누르는 동안만 움직이도록: 마지막 명령이 오래되면 0으로 본다
struct TimedCommand
{
  double value{0.0};
  rclcpp::Time stamp{0, 0, RCL_ROS_TIME};

  void set(double v, const rclcpp::Time & now) {value = v; stamp = now;}
  double get(const rclcpp::Time & now, double timeout) const
  {
    if (stamp.nanoseconds() == 0 || (now - stamp).seconds() > timeout) {
      return 0.0;
    }
    return value;
  }
};

}  // namespace so101_teleop
