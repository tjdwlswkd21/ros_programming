// viz_node: B12 시각화·모니터링 (디버깅용, debug:=true 일 때만 실행)
//
// /joint_states 의 현재 관절각으로 팔 모양을 막대 그림으로 그려 /arm_markers 로 발행한다.
// ik_node 와 같은 링크 길이·공식(kinematics.hpp)을 쓰므로 실제 로봇이 없어도(use_mock) 팔이 움직이는 모습을 볼 수 있다.
#include <cstdio>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "so101_teleop/common.hpp"
#include "so101_teleop/kinematics.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

using namespace so101_teleop;
using visualization_msgs::msg::Marker;

class VizNode : public rclcpp::Node
{
public:
  VizNode()
  : Node("viz_node")
  {
    geo_.shoulder_height = declare_parameter("shoulder_height", geo_.shoulder_height);
    geo_.shoulder_offset = declare_parameter("shoulder_offset", geo_.shoulder_offset);
    geo_.upper_arm = declare_parameter("upper_arm_length", geo_.upper_arm);
    geo_.forearm = declare_parameter("forearm_length", geo_.forearm);
    gripper_length_ = declare_parameter("gripper_length", 0.10);

    pub_ = create_publisher<visualization_msgs::msg::MarkerArray>("arm_markers", 10);
    sub_ = create_subscription<sensor_msgs::msg::JointState>(
      "joint_states", 10, [this](const sensor_msgs::msg::JointState & m) {on_joint_state(m);});
  }

private:
  void on_joint_state(const sensor_msgs::msg::JointState & m)
  {
    ArmAngles a;
    double flex, gripper;
    if (!find_joint(m, PAN, a.pan) || !find_joint(m, LIFT, a.lift) ||
      !find_joint(m, ELBOW, a.elbow) || !find_joint(m, WRIST_FLEX, flex) ||
      !find_joint(m, GRIPPER, gripper))
    {
      return;
    }
    const auto chain = arm_chain(geo_, a, flex, gripper_length_);

    visualization_msgs::msg::MarkerArray arr;
    // 링크: 하늘색 굵은 선
    Marker links = base_marker(0, Marker::LINE_STRIP);
    links.scale.x = 0.012;
    links.color = color(0.3, 0.7, 1.0);
    // 관절: 흰 공 (마지막 점은 그리퍼 끝)
    Marker joints = base_marker(1, Marker::SPHERE_LIST);
    joints.scale.x = joints.scale.y = joints.scale.z = 0.022;
    joints.color = color(0.95, 0.95, 0.95);
    for (size_t i = 0; i < chain.size(); ++i) {
      geometry_msgs::msg::Point p;
      p.x = chain[i].x;
      p.y = chain[i].y;
      p.z = chain[i].z;
      links.points.push_back(p);
      if (i >= 2) {
        joints.points.push_back(p);
      }
    }
    // 그리퍼 끝 위에 열림 값
    Marker text = base_marker(2, Marker::TEXT_VIEW_FACING);
    text.pose.position = links.points.back();
    text.pose.position.z += 0.04;
    text.scale.z = 0.025;
    text.color = color(1.0, 0.85, 0.3);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "grip=%.0f", gripper);  // RViz 글꼴에서 공백이 크게 벌어져서 공백 없이
    text.text = buf;

    arr.markers = {links, joints, text};
    pub_->publish(arr);
  }

  Marker base_marker(int id, int type) const
  {
    Marker mk;
    mk.header.frame_id = "base";
    mk.header.stamp = now();
    mk.ns = "arm";
    mk.id = id;
    mk.type = type;
    mk.action = Marker::ADD;
    mk.pose.orientation.w = 1.0;
    return mk;
  }

  static std_msgs::msg::ColorRGBA color(float r, float g, float b)
  {
    std_msgs::msg::ColorRGBA c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = 1.0f;
    return c;
  }

  ArmGeometry geo_;
  double gripper_length_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr pub_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr sub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<VizNode>());
  rclcpp::shutdown();
  return 0;
}
