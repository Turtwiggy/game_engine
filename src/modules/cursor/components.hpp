#pragma once

// other libs
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// std libs
#include <vector>

namespace game2d {

struct GridCursorComponent
{
  entt::entity line_u = entt::null;
  entt::entity line_d = entt::null;
  entt::entity line_l = entt::null;
  entt::entity line_r = entt::null;
  entt::entity backdrop = entt::null;
};

struct FreeCursorComponent
{
  entt::entity line_u = entt::null;
  entt::entity line_d = entt::null;
  entt::entity line_l = entt::null;
  entt::entity line_r = entt::null;
  entt::entity backdrop = entt::null;
};

struct AnimatedCursorClickComponent
{
  bool placeholder = true;
};

} // namespace game2d