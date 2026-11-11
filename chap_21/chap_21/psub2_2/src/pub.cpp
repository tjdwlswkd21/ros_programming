#include "psub2_2/pub.hpp"
Pub::Pub() : Node("mypub")
{
    count_.x = 0.0;
    count_.y = 0.0;
    count_.z = 0.0;
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    pub_ = this->create_publisher<geometry_msgs::msg::Vector3>("mytopic", qos_profile);
    timer_ = this->create_wall_timer(1s, std::bind(&Pub::publish_msg, this));
}

void Pub::publish_msg()
{
    auto msg = geometry_msgs::msg::Vector3();
    msg.x = count_.x;
    msg.y = count_.y;
    msg.z = count_.z;
    RCLCPP_INFO(this->get_logger(), "Published message: x=%.1f, y=%.1f, z=%.1f",
                msg.x, msg.y, msg.z);
    pub_->publish(msg);
    count_.x += 1.0;
    count_.y += 1.0;
    count_.z += 1.0;
}
