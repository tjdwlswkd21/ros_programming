#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include <memory>
#include <chrono>
#include <functional>

using namespace std::chrono_literals;

void mypub_callback(rclcpp::Node::SharedPtr node,
                    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub)
{
    std_msgs::msg::String message;
    message.data = "Hello world!";
    RCLCPP_INFO(node->get_logger(), "Publish: %s", message.data.c_str());
    pub->publish(message);
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_1");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto mypub = node->create_publisher<std_msgs::msg::String>("topic_pub1_1", qos_profile);
    std::function<void()> fn = std::bind(mypub_callback, node, mypub);
    auto timer = node->create_wall_timer(100ms, fn);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
