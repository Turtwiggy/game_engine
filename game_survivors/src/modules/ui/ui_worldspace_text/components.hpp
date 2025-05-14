#pragma once

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>
#include <imgui.h>

#include <functional>

namespace game2d {

struct WorldspaceTextComponent
{
  std::function<void(entt::registry&, entt::entity e, WorldspaceTextComponent)> layout;
  ImVec2 offset{ 0.0f, 0.0f }; // transform + offset
  std::string text = "placeholder";
};

} // namespace game2d