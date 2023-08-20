#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct ScaleTransformByVelocity
{
  entt::entity debug_a;
  entt::entity debug_b;
  entt::entity debug_c;
  entt::entity debug_d;
};

struct RotatedSquare
{
  float theta;
  int w;
  int h;
  glm::vec2 center;
  glm::vec2 tl;
  glm::vec2 tr;
  glm::vec2 br;
  glm::vec2 bl;
};

struct PlayerCursor
{
  bool placeholder = true;
};

} // namespace game2d