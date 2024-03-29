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

  int GRID_SIZE = 16;
  int CURSOR_SIZE = 16;
  glm::ivec2 mouse_click = { 0, 0 };
};

struct FreeCursorComponent
{
  entt::entity line_u = entt::null;
  entt::entity line_d = entt::null;
  entt::entity line_l = entt::null;
  entt::entity line_r = entt::null;
  entt::entity backdrop = entt::null;

  int CURSOR_SIZE = 16;
  glm::ivec2 mouse_click = { 0, 0 };
};

struct AnimatedCursorClickComponent
{
  bool placeholder = true;
};

} // namespace game2d