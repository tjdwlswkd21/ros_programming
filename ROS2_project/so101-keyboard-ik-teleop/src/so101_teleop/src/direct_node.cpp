// direct_node: B6 손목회전제어 + B7 그리퍼제어
//
// Ctrl+A/D 로 wrist_roll 목표각, ←/→ 로 그리퍼 목표값(0~100)을 만든다.
// 시작할 때 /joint_states 의 현재 값에서 출발한다.
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "so101_teleop/common.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int8.hpp"

using so101_teleop::GRIPPER;
using so101_teleop::WRIST_ROLL;

class DirectNode : public rclcpp::Node
{
public:
  DirectNode()
  : Node("direct_node")
  {
    roll_speed_ = declare_parameter("roll_speed", 1.5);
    gripper_speed_ = declare_parameter("gripper_speed", 60.0);
    cmd_timeout_ = declare_parameter("cmd_timeout", 0.3);
    const double rate_hz = declare_parameter("rate_hz", 50.0);
    dt_ = 1.0 / rate_hz;
    limits_ = so101_teleop::declare_joint_limits(*this);

    roll_sub_ = create_subscription<std_msgs::msg::Int8>(
      "roll_cmd", 10, [this](const std_msgs::msg::Int8 & m) {roll_dir_.set(m.data, now());});
    gripper_sub_ = create_subscription<std_msgs::msg::Int8>(
      "gripper_cmd", 10, [this](const std_msgs::msg::Int8 & m) {gripper_dir_.set(m.data, now());});
    js_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "joint_states", 10, [this](const sensor_msgs::msg::JointState & m) {
        if (initialized_ || !so101_teleop::find_joint(m, WRIST_ROLL, roll_) ||
        !so101_teleop::find_joint(m, GRIPPER, gripper_))
        {
          return;
        }
        initialized_ = true;
        RCLCPP_INFO(get_logger(), "시작 손목 회전 %.2f rad, 그리퍼 %.1f", roll_, gripper_);
      });

    roll_pub_ = create_publisher<std_msgs::msg::Float64>("roll_target", 10);
    gripper_pub_ = create_publisher<std_msgs::msg::Float64>("gripper_target", 10);
    timer_ = create_wall_timer(std::chrono::duration<double>(dt_), [this]() {on_timer();});
  }

private:
  void on_timer()
  {
    if (!initialized_) {
      return;
    }
    const auto t = now();
    // B6
    roll_ = limits_.clamp(WRIST_ROLL,
        roll_ + roll_dir_.get(t, cmd_timeout_) * roll_speed_ * dt_);
    // B7
    gripper_ = limits_.clamp(GRIPPER,
        gripper_ + gripper_dir_.get(t, cmd_timeout_) * gripper_speed_ * dt_);

    std_msgs::msg::Float64 m;
    m.data = roll_;
    roll_pub_->publish(m);
    m.data = gripper_;
    gripper_pub_->publish(m);
  }

  so101_teleop::JointLimits limits_;
  double roll_speed_, gripper_speed_, cmd_timeout_, dt_;
  bool initialized_{false};
  double roll_{0.0}, gripper_{0.0};
  so101_teleop::TimedCommand roll_dir_, gripper_dir_;

  rclcpp::Subscription<std_msgs::msg::Int8>::SharedPtr roll_sub_, gripper_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr js_sub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr roll_pub_, gripper_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DirectNode>());
  rclcpp::shutdown();
  return 0;
}
