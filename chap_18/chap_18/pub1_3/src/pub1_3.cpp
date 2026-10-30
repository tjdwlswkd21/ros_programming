#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <memory>
#include <iostream>
#include <termios.h>
#include <unistd.h>
int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("node_pub1_3");
    auto qos_profile = rclcpp::QoS(rclcpp::KeepLast(10));
    auto mypub = node->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel",
    qos_profile );

    // 엔터 없이 키 하나를 바로 읽도록 터미널 설정 변경
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 0;
    newt.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "f: 전진, b: 후진, l: 좌회전, r: 우회전, q: 종료" << std::endl;
    while(rclcpp::ok())
    {
        char key = 0;
        if(read(STDIN_FILENO, &key, 1) <= 0)
            continue;
        geometry_msgs::msg::Twist message;
        if(key == 'f')      message.linear.x  =  2.0;
        else if(key == 'b') message.linear.x  = -2.0;
        else if(key == 'l') message.angular.z =  2.0;
        else if(key == 'r') message.angular.z = -2.0;
        else if(key == 'q') break;
        else continue;
        RCLCPP_INFO(node->get_logger(), "Publish: linear.x=%.1f, angular.z=%.1f",
                    message.linear.x, message.angular.z);
        mypub->publish(message);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    rclcpp::shutdown();
    return 0;
}
