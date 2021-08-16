#pragma once

// c++ headers
#include <vector>

// other project headers
#include <glm/glm.hpp>

// engine headers
#include "engine/opengl/shader.hpp"

// your project headers
#include "2d_game_object.hpp"

namespace game2d {

namespace sprite_renderer {

//
// V2 Renderer (Batched Draw Calls)
//

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
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go);

void
draw_instanced_sprite(const GameObject2D& cam,
                      const glm::ivec2& screen_size,
                      fightingengine::Shader& shader,
                      const GameObject2D& go,
                      const glm::vec4 colour_tl,
                      const glm::vec4 colour_tr,
                      const glm::vec4 colour_bl,
                      const glm::vec4 colour_br);

void
draw_sprites_debug(const GameObject2D& cam,
                   const glm::ivec2& screen_size,
                   const std::vector<std::reference_wrapper<GameObject2D>>& game_objects,
                   fightingengine::Shader& debug_line_shader,
                   const glm::vec4& debug_line_shader_colour);

} // namespace sprite_renderer

} // namespace game2d