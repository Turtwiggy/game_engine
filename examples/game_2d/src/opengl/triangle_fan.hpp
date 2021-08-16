#pragma once

// engine headers
#include "engine/opengl/shader.hpp"

// your project headers
#include "2d_game_object.hpp"

namespace game2d {

namespace triangle_fan_renderer {

void
reset_quad_vert_count();
void
end_frame();
int
get_draw_calls();
int
get_quad_count();

void
init();
void
shutdown();

void
end_batch();
void
flush(fightingengine::Shader& shader);
void
begin_batch();

void
add_point_to_fan(const GameObject2D& cam, fightingengine::Shader& shader, const glm::vec2 point);

} // namespace triangle_fan_renderer

} // namespace game2d