#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>

namespace game2d {

struct WorldspaceTextComponent
{
  ImGuiWindowFlags flags = 0;
  std::function<void(entt::registry&)> layout;
  ImVec2 offset{ 0.0f, 0.0f }; // transform + offset
  ImVec2 size{ 0.0f, 0.0f };
  float alpha = 1.0f;
  bool display = true;
};

} // namespace game2d