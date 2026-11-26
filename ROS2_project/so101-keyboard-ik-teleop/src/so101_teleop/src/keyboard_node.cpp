// keyboard_node: B1 키보드입력 + B2 입력해석
//
// GLFW 창(WSLg로 Windows 화면에 표시)에 포커스가 있을 때 눌린 키만 읽는다.
// 창이 포커스를 잃으면 모든 키를 뗀 것으로 보고 정지 명령을 보낸다.
//
//   W/S        : x 앞/뒤          A/D        : y 왼쪽/오른쪽     ↑/↓ : z 위/아래
//   Ctrl+W/S   : 목표 기울기 +/-  Ctrl+A/D   : 손목 회전 반시계/시계
//   ←/→        : 그리퍼 닫기/열기  T          : 기울기 유지 켜기/끄기
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

#include "geometry_msgs/msg/vector3.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int8.hpp"
#include "std_msgs/msg/int8_multi_array.hpp"

namespace
{

// B1 출력 (key_state): 눌린 키 + Ctrl 상태
struct KeyState
{
  bool w{false}, s{false}, a{false}, d{false};
  bool up{false}, down{false}, left{false}, right{false};
  bool t{false}, ctrl{false};
  bool focused{false};
};

// B2 출력: 각 명령은 -1 / 0 / +1
struct Commands
{
  int move_x{0}, move_y{0}, move_z{0};
  int tilt{0}, roll{0}, gripper{0};

  bool any() const {return move_x || move_y || move_z || tilt || roll || gripper;}
};

int axis(bool plus, bool minus) {return (plus ? 1 : 0) - (minus ? 1 : 0);}

std::string sign_str(int v) {return v > 0 ? "+" : (v < 0 ? "-" : "0");}

}  // namespace

class KeyboardNode : public rclcpp::Node
{
public:
  KeyboardNode()
  : Node("keyboard_node")
  {
    rate_hz_ = declare_parameter("rate_hz", 50.0);
    hold_ = declare_parameter("tilt_hold_default", true);

    move_pub_ = create_publisher<geometry_msgs::msg::Vector3>("move_cmd", 10);
    tilt_pub_ = create_publisher<std_msgs::msg::Int8MultiArray>("tilt_cmd", 10);
    roll_pub_ = create_publisher<std_msgs::msg::Int8>("roll_cmd", 10);
    gripper_pub_ = create_publisher<std_msgs::msg::Int8>("gripper_cmd", 10);
  }

  int run()
  {
    glfwSetErrorCallback([](int code, const char * desc) {
        std::fprintf(stderr, "[keyboard_node] GLFW 오류 %d: %s\n", code, desc);
      });
    if (!glfwInit()) {
      RCLCPP_FATAL(get_logger(), "GLFW 초기화 실패. WSLg가 켜져 있는지(DISPLAY=%s) 확인하세요.",
        std::getenv("DISPLAY") ? std::getenv("DISPLAY") : "(없음)");
      return 1;
    }
    GLFWwindow * win = glfwCreateWindow(520, 180, "SO-101 teleop", nullptr, nullptr);
    if (!win) {
      RCLCPP_FATAL(get_logger(), "키 입력 창을 만들지 못했습니다.");
      glfwTerminate();
      return 1;
    }
    glfwMakeContextCurrent(win);
    // 두 번 읽는 사이에 눌렀다 뗀 짧은 입력도 한 번은 눌림으로 읽히게 한다
    glfwSetInputMode(win, GLFW_STICKY_KEYS, GLFW_TRUE);

    RCLCPP_INFO(get_logger(),
      "키 입력 창을 클릭해 포커스를 준 뒤 조작하세요.\n"
      "  W/S: 앞/뒤  A/D: 왼쪽/오른쪽  ↑/↓: 위/아래\n"
      "  Ctrl+W/S: 목표 기울기  Ctrl+A/D: 손목 회전  ←/→: 그리퍼 닫기/열기  T: 기울기 유지 토글\n"
      "  창을 닫으면 전체가 종료됩니다.");

    rclcpp::Rate rate(rate_hz_);
    bool prev_t = false;
    while (rclcpp::ok() && !glfwWindowShouldClose(win)) {
      glfwPollEvents();
      const KeyState k = read_keys(win);

      // T는 누르는 순간에만 토글
      if (k.t && !prev_t) {
        hold_ = !hold_;
        RCLCPP_INFO(get_logger(), "기울기 유지: %s", hold_ ? "ON" : "OFF");
      }
      prev_t = k.t;

      const Commands c = interpret(k);
      publish(c);
      draw(win, k, c);
      rate.sleep();
    }

    publish(Commands{});  // 종료 전에 정지
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
  }

private:
  // B1: 창에 포커스가 있을 때만 키를 읽는다
  KeyState read_keys(GLFWwindow * win) const
  {
    KeyState k;
    k.focused = glfwGetWindowAttrib(win, GLFW_FOCUSED);
    if (!k.focused) {
      return k;  // 포커스를 잃으면 모든 키를 뗀 것으로 처리
    }
    auto down = [win](int key) {return glfwGetKey(win, key) == GLFW_PRESS;};
    k.w = down(GLFW_KEY_W);
    k.s = down(GLFW_KEY_S);
    k.a = down(GLFW_KEY_A);
    k.d = down(GLFW_KEY_D);
    k.up = down(GLFW_KEY_UP);
    k.down = down(GLFW_KEY_DOWN);
    k.left = down(GLFW_KEY_LEFT);
    k.right = down(GLFW_KEY_RIGHT);
    k.t = down(GLFW_KEY_T);
    k.ctrl = down(GLFW_KEY_LEFT_CONTROL) || down(GLFW_KEY_RIGHT_CONTROL);
    return k;
  }

  // B2: Ctrl 조합을 먼저 가른다
  static Commands interpret(const KeyState & k)
  {
    Commands c;
    if (k.ctrl) {
      c.tilt = axis(k.w, k.s);
      c.roll = axis(k.a, k.d);  // Ctrl+A = 반시계(+), Ctrl+D = 시계(-)
    } else {
      c.move_x = axis(k.w, k.s);
      c.move_y = axis(k.a, k.d);  // A = 왼쪽(+y)
    }
    c.move_z = axis(k.up, k.down);
    c.gripper = axis(k.right, k.left);  // → 열기(+), ← 닫기(-)
    return c;
  }

  void publish(const Commands & c)
  {
    geometry_msgs::msg::Vector3 move;
    move.x = c.move_x;
    move.y = c.move_y;
    move.z = c.move_z;
    move_pub_->publish(move);

    std_msgs::msg::Int8MultiArray tilt;
    tilt.data = {static_cast<int8_t>(c.tilt), static_cast<int8_t>(hold_ ? 1 : 0)};
    tilt_pub_->publish(tilt);

    std_msgs::msg::Int8 roll;
    roll.data = static_cast<int8_t>(c.roll);
    roll_pub_->publish(roll);

    std_msgs::msg::Int8 gripper;
    gripper.data = static_cast<int8_t>(c.gripper);
    gripper_pub_->publish(gripper);
  }

  // 창 색과 제목으로 상태를 보여준다 (회색 = 포커스 없음, 파랑 = 대기, 초록 = 동작 중)
  void draw(GLFWwindow * win, const KeyState & k, const Commands & c)
  {
    if (!k.focused) {
      glClearColor(0.35f, 0.35f, 0.35f, 1.0f);
    } else if (c.any()) {
      glClearColor(0.15f, 0.55f, 0.25f, 1.0f);
    } else {
      glClearColor(0.12f, 0.22f, 0.45f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(win);

    // WSLg 제목 표시줄 글꼴에는 한글이 없어서 제목은 영어로만 쓴다
    std::string title = "SO-101 | tilt hold " + std::string(hold_ ? "ON" : "OFF");
    if (!k.focused) {
      title += " | CLICK HERE to control";
    } else {
      title += " | xyz " + sign_str(c.move_x) + sign_str(c.move_y) + sign_str(c.move_z) +
        "  tilt " + sign_str(c.tilt) + "  roll " + sign_str(c.roll) +
        "  grip " + sign_str(c.gripper);
    }
    if (title != last_title_) {
      glfwSetWindowTitle(win, title.c_str());
      last_title_ = title;
    }
  }

  double rate_hz_;
  bool hold_;
  std::string last_title_;
  rclcpp::Publisher<geometry_msgs::msg::Vector3>::SharedPtr move_pub_;
  rclcpp::Publisher<std_msgs::msg::Int8MultiArray>::SharedPtr tilt_pub_;
  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr roll_pub_;
  rclcpp::Publisher<std_msgs::msg::Int8>::SharedPtr gripper_pub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  int rc = std::make_shared<KeyboardNode>()->run();
  rclcpp::shutdown();
  return rc;
}
