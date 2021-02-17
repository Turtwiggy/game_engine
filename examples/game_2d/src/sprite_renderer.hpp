#pragma once

// your project headers (engine)
#include "engine/opengl/shader.hpp"

// your prject headers (game)
#include "game.hpp"

namespace game2d {

namespace sprite_renderer {

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void
render_quad();

void
draw_sprite(Shader& shader, GameObject& game_object);
void
draw_sprite(Shader& shader, Transform& t, int tex_slot = 0);
void
draw_sprite(Shader& shader, glm::vec2 position, glm::vec2 size, float angle, glm::vec3 color, int tex_slot = 0);

} // namespace sprite_renderer

} // namespace game2d