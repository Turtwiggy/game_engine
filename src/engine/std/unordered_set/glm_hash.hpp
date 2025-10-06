#pragma once

namespace std {

template<>
struct hash<glm::ivec2>
{
  std::size_t operator()(const glm::ivec2& k) const { return (std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1)); }
};

} // namespace std

namespace game2d {

struct vec2_hash
{
  std::size_t operator()(const glm::vec2& v) const;
};

} // namespace game2d