#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct WorldspaceText
{
  glm::vec2 worldspace_position;
  std::string text = "";
};

void
draw_text(entt::registry& r, const WorldspaceText& text);

} // namespace game2dz