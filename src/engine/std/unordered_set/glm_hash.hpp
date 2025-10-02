#pragma once

namespace game2d {

struct vec2_hash
{
  std::size_t operator()(const glm::vec2& v) const;
};

} // namespace game2d