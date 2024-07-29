#pragma once

#include <glm/glm.hpp>

#include <functional>
#include <optional>
#include <string>

namespace game2d {

struct WorldspaceTextComponent
{
  std::string text;
  glm::ivec2 offset{ 0, 0 };

  bool split_text_into_chunks = false;
  int chunk_length = 20;

  std::optional<std::function<void()>> layout = std::nullopt;
};

} // namespace game2d