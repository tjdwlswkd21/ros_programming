// merge_node: B8 관절명령통합
//
// 4개 목표(arm, wrist_flex, roll, gripper)가 모두 들어온 뒤부터
// 관절 한계와 최대 변화량(속도)을 제한한 6개 관절 명령을 /joint_command 로 보낸다.
#include <algorithm>
#include <array>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "so101_teleop/common.hpp"
#include "std_msgs/msg/float64.hpp"

using namespace so101_teleop;

class MergeNode : public rclcpp::Node
{
public:
  MergeNode()
  : Node("merge_node")
  {
    max_joint_speed_ = declare_parameter("max_joint_speed", 1.5);
    max_gripper_speed_ = declare_parameter("max_gripper_speed", 100.0);
    const double rate_hz = declare_parameter("rate_hz", 50.0);
    dt_ = 1.0 / rate_hz;
    limits_ = declare_joint_limits(*this);

    arm_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "arm_target", 10, [this](const sensor_msgs::msg::JointState & m) {
        if (find_joint(m, PAN, desired_[PAN]) && find_joint(m, LIFT, desired_[LIFT]) &&
        find_joint(m, ELBOW, desired_[ELBOW]))
        {
          got_[0] = true;
        }
      });
    flex_sub_ = scalar_sub("wrist_flex_target", WRIST_FLEX, 1);
    roll_sub_ = scalar_sub("roll_target", WRIST_ROLL, 2);
    gripper_sub_ = scalar_sub("gripper_target", GRIPPER, 3);

    cmd_pub_ = create_publisher<sensor_msgs::msg::JointState>("joint_command", 10);
    timer_ = create_wall_timer(std::chrono::duration<double>(dt_), [this]() {on_timer();});
  }

private:
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr scalar_sub(
    const std::string & topic, Joint j, int slot)
  {
    return create_subscription<std_msgs::msg::Float64>(
      topic, 10, [this, j, slot](const std_msgs::msg::Float64 & m) {
        desired_[j] = m.data;
        got_[slot] = true;
      });
  }

  void on_timer()
  {
    if (!std::all_of(got_.begin(), got_.end(), [](bool b) {return b;})) {
      return;
    }
    std::array<double, NUM_JOINTS> target;
    for (int j = 0; j < NUM_JOINTS; ++j) {
      target[j] = limits_.clamp(static_cast<Joint>(j), desired_[j]);
    }
    if (!started_) {
      // 처음에는 목표 = 현재 위치이므로 그대로 시작
      command_ = target;
      started_ = true;
      RCLCPP_INFO(get_logger(), "모든 목표 수신, /joint_command 발행 시작");
    }
    for (int j = 0; j < NUM_JOINTS; ++j) {
      const double max_step = (j == GRIPPER ? max_gripper_speed_ : max_joint_speed_) * dt_;
      command_[j] += std::clamp(target[j] - command_[j], -max_step, max_step);
    }

    sensor_msgs::msg::JointState m;
    m.header.stamp = now();
    m.name = joint_names();
    m.position.assign(command_.begin(), command_.end());
    cmd_pub_->publish(m);
  }

  JointLimits limits_;
  double max_joint_speed_, max_gripper_speed_, dt_;
  std::array<double, NUM_JOINTS> desired_{};
  std::array<double, NUM_JOINTS> command_{};
  std::array<bool, 4> got_{};
  bool started_{false};

  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr arm_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr flex_sub_, roll_sub_, gripper_sub_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr cmd_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MergeNode>());
  rclcpp::shutdown();
  return 0;
}
