#include <gtest/gtest.h>

#include <cmath>

#include "so101_teleop/kinematics.hpp"

using namespace so101_teleop;

// IK로 구한 각도를 FK에 넣으면 원래 위치가 나와야 한다
TEST(Kinematics, InverseThenForwardRoundTrip)
{
  const ArmGeometry g;
  for (double x : {0.10, 0.18, 0.24}) {
    for (double y : {-0.10, 0.0, 0.10}) {
      for (double z : {0.05, 0.12, 0.20}) {
        const Vec3 p{x, y, z};
        const auto a = inverse(g, p);
        ASSERT_TRUE(a.has_value()) << x << " " << y << " " << z;
        const Vec3 q = forward(g, *a);
        EXPECT_NEAR(q.x, p.x, 1e-9);
        EXPECT_NEAR(q.y, p.y, 1e-9);
        EXPECT_NEAR(q.z, p.z, 1e-9);
        EXPECT_LE(a->elbow, 0.0);  // 팔꿈치가 위로 가는 해
      }
    }
  }
}

TEST(Kinematics, UnreachableReturnsNullopt)
{
  const ArmGeometry g;
  EXPECT_FALSE(inverse(g, {1.0, 0.0, 0.1}).has_value());
  EXPECT_FALSE(inverse(g, {0.0, 0.0, 0.2}).has_value());
}

TEST(Kinematics, HomePose)
{
  // 상완 수직, 전완 수평 앞
  const ArmGeometry g;
  const Vec3 p = forward(g, {0.0, M_PI / 2, -M_PI / 2});
  EXPECT_NEAR(p.x, g.shoulder_offset + g.forearm, 1e-9);
  EXPECT_NEAR(p.y, 0.0, 1e-9);
  EXPECT_NEAR(p.z, g.shoulder_height + g.upper_arm, 1e-9);
}
