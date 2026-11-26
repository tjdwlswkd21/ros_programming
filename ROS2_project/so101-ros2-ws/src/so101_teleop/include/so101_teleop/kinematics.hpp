// SO-101 팔(pan, lift, elbow)의 순기구학/역기구학 (B4, B11)
//
// 좌표계: 로봇 베이스 기준 x = 앞, y = 왼쪽, z = 위 (m)
// 각도 규약 (rad):
//   pan   : 0 = +x 방향, +는 반시계(왼쪽)
//   lift  : 상완이 수평에서 올라간 각도 (0 = 앞으로 수평, pi/2 = 수직)
//   elbow : 상완 대비 전완의 상대각 (+는 lift와 같은 방향). 팔꿈치가 위로 가는 해는 elbow < 0
//   wrist_flex : 전완 대비 손목의 상대각  ->  손목 기울기 = lift + elbow + wrist_flex
#pragma once

#include <array>
#include <cmath>
#include <optional>

namespace so101_teleop
{

struct Vec3
{
  double x{0.0};
  double y{0.0};
  double z{0.0};
};

struct ArmAngles
{
  double pan{0.0};
  double lift{0.0};
  double elbow{0.0};
};

struct ArmGeometry
{
  double shoulder_height{0.119};  // 베이스 바닥 -> shoulder_lift 축 높이
  double shoulder_offset{0.030};  // pan 축 -> shoulder_lift 축 수평 거리 (앞쪽이 +)
  double upper_arm{0.116};        // shoulder_lift 축 -> elbow 축
  double forearm{0.135};          // elbow 축 -> wrist_flex 축 (손목 점)
};

// 관절각 -> 손목 점 위치
inline Vec3 forward(const ArmGeometry & g, const ArmAngles & a)
{
  const double r = g.shoulder_offset + g.upper_arm * std::cos(a.lift) +
    g.forearm * std::cos(a.lift + a.elbow);
  const double z = g.shoulder_height + g.upper_arm * std::sin(a.lift) +
    g.forearm * std::sin(a.lift + a.elbow);
  return {r * std::cos(a.pan), r * std::sin(a.pan), z};
}

// 팔 모양 그리기용 (B12): 베이스 -> 어깨 축 높이 -> 어깨(lift 축) -> 팔꿈치 -> 손목 -> 그리퍼 끝
inline std::array<Vec3, 6> arm_chain(
  const ArmGeometry & g, const ArmAngles & a, double wrist_flex, double gripper_length)
{
  // (r, z) 평면의 점을 pan 방향으로 돌린다
  auto at = [&a](double r, double z) {
      return Vec3{r * std::cos(a.pan), r * std::sin(a.pan), z};
    };
  const double r1 = g.shoulder_offset;
  const double z1 = g.shoulder_height;
  const double r2 = r1 + g.upper_arm * std::cos(a.lift);
  const double z2 = z1 + g.upper_arm * std::sin(a.lift);
  const double r3 = r2 + g.forearm * std::cos(a.lift + a.elbow);
  const double z3 = z2 + g.forearm * std::sin(a.lift + a.elbow);
  const double tilt = a.lift + a.elbow + wrist_flex;
  const double r4 = r3 + gripper_length * std::cos(tilt);
  const double z4 = z3 + gripper_length * std::sin(tilt);
  return {Vec3{0.0, 0.0, 0.0}, Vec3{0.0, 0.0, z1}, at(r1, z1), at(r2, z2), at(r3, z3), at(r4, z4)};
}

// 손목 점 위치 -> 관절각 (팔꿈치가 위로 가는 해). 닿지 않으면 nullopt
inline std::optional<ArmAngles> inverse(const ArmGeometry & g, const Vec3 & p)
{
  const double r = std::hypot(p.x, p.y);
  if (r < 1e-6) {
    return std::nullopt;  // 베이스 바로 위: pan이 정해지지 않음
  }
  const double pan = std::atan2(p.y, p.x);

  // (r, z) 평면에서 shoulder_lift 축 기준 좌표
  const double rr = r - g.shoulder_offset;
  const double zz = p.z - g.shoulder_height;
  const double l1 = g.upper_arm;
  const double l2 = g.forearm;

  // 코사인 법칙
  const double c = (rr * rr + zz * zz - l1 * l1 - l2 * l2) / (2.0 * l1 * l2);
  if (c < -1.0 || c > 1.0) {
    return std::nullopt;
  }
  const double elbow = -std::acos(c);
  const double lift = std::atan2(zz, rr) -
    std::atan2(l2 * std::sin(elbow), l1 + l2 * std::cos(elbow));
  return ArmAngles{pan, lift, elbow};
}

}  // namespace so101_teleop
