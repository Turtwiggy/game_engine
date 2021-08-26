#pragma once

// c++ lib header
#include <tuple>
#include <vector>

// engine headers
#include "engine/opengl/shader.hpp"

// game headers
#include "2d_game_object.hpp"

namespace game2d {

void
generate_intersections(GameObject2D& camera,
                       glm::ivec2& light_pos,
                       const std::vector<std::reference_wrapper<GameObject2D>>& ents,
                       const glm::ivec2& screen_wh,
                       std::vector<std::tuple<float, float, float>>& intersections);

} // namespace game2d
