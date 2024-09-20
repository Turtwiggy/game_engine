#pragma once

#include <glm/glm.hpp>
#include <imgui.h>

#include <functional>

namespace game2d {

struct WorldspaceTextComponent
{
  ImGuiWindowFlags flags = 0;
  std::function<void()> layout;
};

} // namespace game2d