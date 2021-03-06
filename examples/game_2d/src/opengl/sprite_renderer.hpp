#pragma once

// other project headers
#include <glm/glm.hpp>

// your project headers
#include "2d_game_object.hpp"
#include "engine/opengl/shader.hpp"

namespace game2d {

namespace sprite_renderer {

//
// V2 Renderer (Batched Draw Calls)
//

void
reset_stats();
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
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go,
                      const glm::vec2 draw_size);

void
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go,
                      const glm::vec2 draw_size,
                      const glm::vec4 colour_tl,
                      const glm::vec4 colour_tr,
                      const glm::vec4 colour_bl,
                      const glm::vec4 colour_br);

void
draw_sprite_debug(const GameObject2D& cam,
                  const glm::ivec2& screen_size,
                  fightingengine::Shader& shader,
                  const GameObject2D& go,
                  const glm::vec2 draw_size,
                  fightingengine::Shader& debug_line_shader,
                  const glm::vec4& debug_line_shader_colour);

} // namespace sprite_renderer

} // namespace game2d