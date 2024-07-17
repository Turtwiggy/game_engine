#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite)
{
  // convert to imgui representation
  const auto [ss, frames] = find_animation(get_first_component<SINGLE_Animations>(r), sprite);
  const int size_x = ss.px_total;
  const int size_y = ss.py_total;
  const float cols_x = ss.nx;
  const float cols_y = ss.ny;
  const float pixels_x = size_x / cols_x;
  const float pixels_y = size_y / cols_y;
  const glm::ivec2 offset = { frames.animation_frames[0].x, frames.animation_frames[0].y };
  const ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f) / size_x), ((offset.y * pixels_y + 0.0f) / size_y));
  const ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_y) / size_y));
  return { tl, br };
};

} // namespace game2d