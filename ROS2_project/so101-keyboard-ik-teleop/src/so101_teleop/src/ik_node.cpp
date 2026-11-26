// ik_node: B3 목표위치생성 + B4 역기구학계산 + B11 순기구학계산
//
// 시작할 때 /joint_states 로 현재 손목 위치를 구해 목표로 삼고,
// 이후 /move_cmd 방향으로 목표점을 쌓아 가며 IK로 pan/lift/elbow 를 구한다.
// 작업영역이나 관절 한계를 넘는 이동은 무시한다 (이미 밖에 있으면 안쪽으로 가는 이동만 허용).
#include <cmath>
#include <memory>
#include <vector>

#include "geometry_msgs/msg/point_stamped.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "so101_teleop/common.hpp"
#include "so101_teleop/kinematics.hpp"

using namespace std::chrono_literals;
using so101_teleop::ArmAngles;
using so101_teleop::Vec3;

class IkNode : public rclcpp::Node
{
public:
  IkNode()
  : Node("ik_node")
  {
    geo_.shoulder_height = declare_parameter("shoulder_height", geo_.shoulder_height);
    geo_.shoulder_offset = declare_parameter("shoulder_offset", geo_.shoulder_offset);
    geo_.upper_arm = declare_parameter("upper_arm_length", geo_.upper_arm);
    geo_.forearm = declare_parameter("forearm_length", geo_.forearm);
    ws_min_ = declare_parameter("workspace_min", std::vector<double>{0.05, -0.30, 0.02});
    ws_max_ = declare_parameter("workspace_max", std::vector<double>{0.40, 0.30, 0.40});
    if (ws_min_.size() != 3 || ws_max_.size() != 3) {
      throw std::runtime_error("workspace_min / workspace_max 는 (x, y, z) 3개 값이 필요합니다");
    }
    move_speed_ = declare_parameter("move_speed", 0.05);
    cmd_timeout_ = declare_parameter("cmd_timeout", 0.3);
    const double rate_hz = declare_parameter("rate_hz", 50.0);
    dt_ = 1.0 / rate_hz;
    limits_ = so101_teleop::declare_joint_limits(*this);

    move_sub_ = create_subscription<geometry_msgs::msg::Vector3>(
      "move_cmd", 10, [this](const geometry_msgs::msg::Vector3 & m) {
        const auto t = now();
        move_[0].set(m.x, t);
        move_[1].set(m.y, t);
        move_[2].set(m.z, t);
      });
    js_sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "joint_states", 10, [this](const sensor_msgs::msg::JointState & m) {on_joint_state(m);});

    arm_pub_ = create_publisher<sensor_msgs::msg::JointState>("arm_target", 10);
    target_pub_ = create_publisher<geometry_msgs::msg::PointStamped>("target_wrist_pos", 10);
    current_pub_ = create_publisher<geometry_msgs::msg::PointStamped>("current_wrist_pose", 10);

    timer_ = create_wall_timer(
      std::chrono::duration<double>(dt_), [this]() {on_timer();});
    RCLCPP_INFO(get_logger(), "/joint_states 를 기다립니다...");
  }

private:
  // B11: 현재 관절각 -> 현재 손목 위치. 처음 한 번은 목표를 현재 위치에 맞춘다
  void on_joint_state(const sensor_msgs::msg::JointState & m)
  {
    ArmAngles cur;
    if (!so101_teleop::find_joint(m, so101_teleop::PAN, cur.pan) ||
      !so101_teleop::find_joint(m, so101_teleop::LIFT, cur.lift) ||
      !so101_teleop::find_joint(m, so101_teleop::ELBOW, cur.elbow))
    {
      return;
    }
    const Vec3 p = so101_teleop::forward(geo_, cur);
    current_pub_->publish(point_msg(p));

    if (!initialized_) {
      target_ = p;
      angles_ = cur;
      initialized_ = true;
      RCLCPP_INFO(get_logger(), "시작 손목 위치 (%.3f, %.3f, %.3f) m", p.x, p.y, p.z);
      if (cur.elbow > 0.0) {
        RCLCPP_WARN(get_logger(),
          "현재 팔꿈치가 아래로 굽은 자세입니다 (elbow=%.2f rad). 첫 이동에서 팔꿈치가 위로 바뀝니다.",
          cur.elbow);
      }
    }
  }

  // B3 + B4
  void on_timer()
  {
    if (!initialized_) {
      return;
    }
    const auto t = now();
    Vec3 v{move_[0].get(t, cmd_timeout_), move_[1].get(t, cmd_timeout_),
      move_[2].get(t, cmd_timeout_)};
    const double n = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (n > 0.0) {
      // 대각선도 같은 속도로
      const double step = move_speed_ * dt_ / n;
      const Vec3 d{v.x * step, v.y * step, v.z * step};
      // 한꺼번에 안 되면 축별로 시도 (작업영역 경계를 따라 미끄러지도록)
      if (!try_move(d)) {
        const bool ok = (d.x != 0.0 && try_move({d.x, 0, 0})) |
          (d.y != 0.0 && try_move({0, d.y, 0})) | (d.z != 0.0 && try_move({0, 0, d.z}));
        if (!ok) {
          RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
            "작업영역/관절 한계 밖이라 이동을 무시합니다. 목표 (%.3f, %.3f, %.3f)",
            target_.x, target_.y, target_.z);
        }
      }
    }

    sensor_msgs::msg::JointState arm;
    arm.header.stamp = t;
    arm.name = {so101_teleop::joint_names()[so101_teleop::PAN],
      so101_teleop::joint_names()[so101_teleop::LIFT],
      so101_teleop::joint_names()[so101_teleop::ELBOW]};
    arm.position = {angles_.pan, angles_.lift, angles_.elbow};
    arm_pub_->publish(arm);
    target_pub_->publish(point_msg(target_));
  }

  bool try_move(const Vec3 & d)
  {
    const Vec3 cand{target_.x + d.x, target_.y + d.y, target_.z + d.z};
    const auto sol = so101_teleop::inverse(geo_, cand);
    if (!sol) {
      return false;  // 닿지 않음: 직전 값 유지
    }
    const double viol = violation(cand, *sol);
    if (viol > 1e-9 && viol >= violation(target_, angles_)) {
      return false;
    }
    target_ = cand;
    angles_ = *sol;
    return true;
  }

  // 작업영역/관절 한계를 벗어난 정도 (안쪽이면 0)
  double violation(const Vec3 & p, const ArmAngles & a) const
  {
    const double pv[3] = {p.x, p.y, p.z};
    double v = 0.0;
    for (int i = 0; i < 3; ++i) {
      v += std::max(0.0, ws_min_[i] - pv[i]) + std::max(0.0, pv[i] - ws_max_[i]);
    }
    v += limits_.excess(so101_teleop::PAN, a.pan);
    v += limits_.excess(so101_teleop::LIFT, a.lift);
    v += limits_.excess(so101_teleop::ELBOW, a.elbow);
    return v;
  }

  geometry_msgs::msg::PointStamped point_msg(const Vec3 & p)
  {
    geometry_msgs::msg::PointStamped m;
    m.header.stamp = now();
    m.header.frame_id = "base";
    m.point.x = p.x;
    m.point.y = p.y;
    m.point.z = p.z;
    return m;
  }

  so101_teleop::ArmGeometry geo_;
  so101_teleop::JointLimits limits_;
  std::vector<double> ws_min_, ws_max_;
  double move_speed_, cmd_timeout_, dt_;

  bool initialized_{false};
  Vec3 target_;
  ArmAngles angles_;
  so101_teleop::TimedCommand move_[3];

  rclcpp::Subscription<geometry_msgs::msg::Vector3>::SharedPtr move_sub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr js_sub_;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr arm_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr target_pub_, current_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<IkNode>());
  rclcpp::shutdown();
  return 0;
}
