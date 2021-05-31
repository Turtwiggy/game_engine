#pragma once

// other project headers
#include <glm/glm.hpp>

// your project headers
#include "2d_game_object.hpp"
#include "engine/opengl/shader.hpp"

namespace game2d {

namespace sprite_renderer {

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

} // namespace sprite_renderer

} // namespace game2d