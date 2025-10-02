#include "pch.hpp"

#include "engine/std/unordered_set/glm_hash.hpp"

namespace game2d {

std::size_t
vec2_hash::operator()(const glm::vec2& v) const
{
  return std::hash<float>{}(v.x) ^ std::hash<float>{}(v.y);
}

} // namespace game2d