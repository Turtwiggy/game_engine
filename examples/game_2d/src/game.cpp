// your header
#include "game.hpp"

// c++ headers
#include <iostream>
#include <vector>

// other headers
#include <glm/glm.hpp>

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "engine/util.hpp"

// game headers
#include "components/resources.hpp"
#include "components/screen_shake_timer.hpp"
#include "constants.hpp"
#include "systems/screen_shake_system.hpp"

namespace game2d {

void
game::init(entt::registry& registry, glm::ivec2 screen_wh, std::chrono::steady_clock::time_point app_start)
{
  // Load resources
  {
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_kenny_nl,
                                  "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png");

    Resources r(fightingengine::Shader("2d_game/shaders/2d_basic.vert", "2d_game/shaders/2d_colour.frag"),
                fightingengine::Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag"),
                fightingengine::Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag"));
    // textures
    r.fbo_lighting = fightingengine::Framebuffer::create_fbo();
    r.fbo_main_scene = fightingengine::Framebuffer::create_fbo();
    r.tex_id_main_scene = fightingengine::create_texture(screen_wh, tex_unit_main_scene, r.fbo_main_scene);
    r.tex_id_lighting = fightingengine::create_texture(screen_wh, tex_unit_lighting, r.fbo_lighting);
    r.texture_ids = fightingengine::load_textures_threaded(textures_to_load, app_start);

    // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
    registry.set<Resources>(r);
  }

  {
    ScreenShakeTimer t;
    registry.set<ScreenShakeTimer>(t);
  }
}

void
game::update_sim(entt::registry& registry, float dt)
{
  //
}

void
game::update(entt::registry& registry, float dt)
{
  update_screen_shake_system(registry, dt);
}

void
game::render(entt::registry& registry)
{
  //
}

} // namespace game2d