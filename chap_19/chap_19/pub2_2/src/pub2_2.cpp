#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include <memory>
#include <chrono>
#include <functional>
#include <iostream>
using namespace std::chrono_literals;

void callback(rclcpp::Node::SharedPtr node,
              rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr mypub,
              double x, double y, double z)
{
    auto message = geometry_msgs::msg::Vector3();
    message.x = x;
    message.y = y;
    message.z = z;
    RCLCPP_INFO(node->get_logger(), "Publish: x=%.2f, y=%.2f, z=%.2f",
                message.x, message.y, message.z);
    mypub->publish(message);
}

int main(int argc, char* argv[])
{
    rclcpp::init(argc, argv);

    double x, y, z;
    std::cout << "실수 3개(x y z)를 입력하세요: ";
    std::cin >> x >> y >> z;

    auto node = std::make_shared<rclcpp::Node>("mynode");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto pub = node->create_publisher<geometry_msgs::msg::Vector3>("mytopic", qos_profile);
    std::function<void()> fn = std::bind(callback, node, pub, x, y, z);
    auto timer = node->create_wall_timer(100ms, fn);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
