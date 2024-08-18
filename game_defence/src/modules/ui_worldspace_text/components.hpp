#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <string>

namespace game2d {

struct WorldspaceTextComponent
{
  std::string text;
  glm::vec2 offset{ 0, 0 };

  bool split_text_into_lines = false;
  int line_length = 20;

  bool requires_hovered = false;
  bool requires_visible = false;

  std::optional<std::function<void()>> layout = std::nullopt;
};

} // namespace game2d