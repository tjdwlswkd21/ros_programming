#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include <memory>
#include <iostream>
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_2");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto mypub = node->create_publisher<geometry_msgs::msg::Vector3>("topic_pub1_2",
    qos_profile );
    geometry_msgs::msg::Vector3 message;
    while(rclcpp::ok())
    {
        std::cout << "실수 3개(x y z)를 입력하세요 (종료: Ctrl+D): ";
        if(!(std::cin >> message.x >> message.y >> message.z))
            break;
        RCLCPP_INFO(node->get_logger(), "Publish: x=%.2f, y=%.2f, z=%.2f",
                    message.x, message.y, message.z);
        mypub->publish(message);
    }
    rclcpp::shutdown();
    return 0;
}
