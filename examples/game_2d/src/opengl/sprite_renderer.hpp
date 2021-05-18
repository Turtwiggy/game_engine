#pragma once

// other project headers
#include <glm/glm.hpp>

// your project headers
#include "engine/opengl/shader.hpp"
#include "game/2d_game_object.hpp"

namespace game2d {

namespace sprite_renderer {

//
// V2 Renderer (Batched Draw Calls)
//

static const size_t max_quad = 1000;
static const size_t max_quad_vert_count = max_quad * 4;
static const size_t max_quad_index_count = max_quad * 6;
static unsigned int VAO, dynamicVBO = 0;

// static void
// init();
// static void
// shutdown();
// static void
// begin_batch();
// static void
// end_batch();
// static void
// flush();

// int draw_calls = 0;
// int quad_count = 0;

// static void
// draw_quad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& colour);

//
// Temp
//

void
draw_instanced(fightingengine::Shader& shader);

//
// V1 Renderer (Draw Call Per Quad)
//

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
void
render_quad();

void
draw_sprite(const GameObject2D& cam,
            const glm::ivec2& screen_size,
            fightingengine::Shader& shader,
            const GameObject2D& object);

void
draw_sprite_debug(const GameObject2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  const GameObject2D& game_object,
                  fightingengine::Shader& debug_line_shader,
                  glm::vec4& debug_line_shader_colour);

} // namespace sprite_renderer

} // namespace game2d