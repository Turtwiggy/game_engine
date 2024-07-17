#pragma once

#include <glm/glm.hpp>

#include <string>

namespace game2d {

struct WorldspaceSpriteRequestComponent
{
  std::string text;
  glm::ivec2 offset{ 0, 0 };

  bool split_text_into_chunks = false;
  int chunk_length = 20;
};

} // namespace game2d