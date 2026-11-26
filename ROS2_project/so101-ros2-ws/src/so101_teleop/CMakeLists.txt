cmake_minimum_required(VERSION 3.8)
project(so101_teleop)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(glfw3 REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

include_directories(include)

set(NODES keyboard_node ik_node wrist_node direct_node merge_node)
foreach(node ${NODES})
  add_executable(${node} src/${node}.cpp)
  target_compile_features(${node} PUBLIC cxx_std_17)
  ament_target_dependencies(${node} rclcpp std_msgs geometry_msgs sensor_msgs)
  install(TARGETS ${node} DESTINATION lib/${PROJECT_NAME})
endforeach()
target_link_libraries(keyboard_node glfw OpenGL::GL)

install(DIRECTORY launch config DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_kinematics test/test_kinematics.cpp)
  target_compile_features(test_kinematics PUBLIC cxx_std_17)
endif()

ament_package()
