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

// 막대 그림의 손목 점은 FK 손목 위치와 같아야 한다
TEST(Kinematics, ArmChainMatchesForward)
{
  const ArmGeometry g;
  const ArmAngles a{0.3, 1.1, -1.4};
  const auto chain = arm_chain(g, a, 0.2, 0.10);
  const Vec3 w = forward(g, a);
  EXPECT_NEAR(chain[4].x, w.x, 1e-12);
  EXPECT_NEAR(chain[4].y, w.y, 1e-12);
  EXPECT_NEAR(chain[4].z, w.z, 1e-12);
  // 그리퍼 끝은 손목에서 gripper_length 만큼 떨어져 있다
  const double d = std::hypot(chain[5].x - w.x, chain[5].y - w.y, chain[5].z - w.z);
  EXPECT_NEAR(d, 0.10, 1e-12);
}
