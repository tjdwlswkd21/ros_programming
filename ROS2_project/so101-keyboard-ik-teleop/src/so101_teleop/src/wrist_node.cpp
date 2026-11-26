// wrist_node: B5 손목기울기계산
//
// 기울기 유지 ON : wrist_flex = 목표 기울기 - (lift + elbow)
// 기울기 유지 OFF: Ctrl+W/S 가 wrist_flex 관절각을 직접 바꾼다
// 관절 한계에 걸리면 wrist_flex 는 한계값에서 멈추고, 목표 기울기는 그대로 둔다.
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "so101_teleop/common.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int8_multi_array.hpp"

using so101_teleop::ELBOW;
using so101_teleop::LIFT;
using so101_teleop::WRIST_FLEX;

class WristNode : public rclcpp::Node
{
public:
  WristNode()
  : Node("wrist_node")
  {
    tilt_speed_ = declare_parameter("tilt_speed", 0.8);
    hold_ = declare_parameter("tilt_hold_default", true);
    cmd_timeout_ = declare_parameter("cmd_timeout", 0.3);
    const double rate_hz = declare_parameter("rate_hz", 50.0);
    dt_ = 1.0 / rate_hz;
    limits_ = so101_teleop::declare_joint_limits(*this);

    tilt_sub_ = create_subscription<std_msgs::msg::Int8MultiArray>(
      "tilt_cmd", 10, [this](const std_msgs::msg::Int8MultiArray & m) {
        if (m.data.size() >= 2) {
          tilt_dir_.set(m.data[0], now());
          set_hold(m.data[1] != 0);
        }
      });
    arm_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "arm_target", 10, [this](const sensor_msgs::msg::JointState & m) {
        double lift, elbow;
        if (so101_teleop::find_joint(m, LIFT, lift) && so101_teleop::find_joint(m, ELBOW, elbow)) {
          arm_sum_ = lift + elbow;
        }
      });
    // 시작할 때 한 번만 사용
    js_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "joint_states", 10, [this](const sensor_msgs::msg::JointState & m) {
        double lift, elbow, flex;
        if (initialized_ || !so101_teleop::find_joint(m, LIFT, lift) ||
        !so101_teleop::find_joint(m, ELBOW, elbow) || !so101_teleop::find_joint(m, WRIST_FLEX, flex))
        {
          return;
        }
        arm_sum_ = lift + elbow;
        flex_ = flex;
        tilt_ = arm_sum_ + flex;
        initialized_ = true;
        RCLCPP_INFO(get_logger(), "시작 기울기 %.2f rad, 기울기 유지 %s", tilt_, hold_ ? "ON" : "OFF");
      });

    flex_pub_ = create_publisher<std_msgs::msg::Float64>("wrist_flex_target", 10);
    timer_ = create_wall_timer(std::chrono::duration<double>(dt_), [this]() {on_timer();});
  }

private:
  // 토글할 때 손목이 튀지 않도록 지금 각도를 이어받는다
  void set_hold(bool hold)
  {
    if (hold == hold_) {
      return;
    }
    hold_ = hold;
    if (hold_) {
      tilt_ = arm_sum_ + flex_;
    }
    RCLCPP_INFO(get_logger(), "기울기 유지 %s (기울기 %.2f rad)", hold_ ? "ON" : "OFF", tilt_);
  }

  void on_timer()
  {
    if (!initialized_) {
      return;
    }
    const double step = tilt_dir_.get(now(), cmd_timeout_) * tilt_speed_ * dt_;
    const double lo = limits_.min[WRIST_FLEX];
    const double hi = limits_.max[WRIST_FLEX];

    if (hold_) {
      // 손목이 이미 한계에 걸린 방향으로는 목표 기울기를 더 쌓지 않는다
      const double flex_now = tilt_ - arm_sum_;
      if ((step > 0 && flex_now < hi) || (step < 0 && flex_now > lo)) {
        tilt_ += step;
      }
      flex_ = limits_.clamp(WRIST_FLEX, tilt_ - arm_sum_);
    } else {
      flex_ = limits_.clamp(WRIST_FLEX, flex_ + step);
    }

    std_msgs::msg::Float64 m;
    m.data = flex_;
    flex_pub_->publish(m);
  }

  so101_teleop::JointLimits limits_;
  double tilt_speed_, cmd_timeout_, dt_;
  bool hold_;
  bool initialized_{false};
  double tilt_{0.0};     // 목표 기울기 (lift + elbow + wrist_flex)
  double flex_{0.0};     // 발행하는 wrist_flex 목표각
  double arm_sum_{0.0};  // 목표 lift + elbow
  so101_teleop::TimedCommand tilt_dir_;

  rclcpp::Subscription<std_msgs::msg::Int8MultiArray>::SharedPtr tilt_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr arm_sub_, js_sub_;
  rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr flex_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WristNode>());
  rclcpp::shutdown();
  return 0;
}
