// your header
#include "game.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/global_resources.hpp"

// systems

// helpers

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "engine/opengl_renderers/batch_renderers/sprite.hpp"
#include "engine/opengl_renderers/batch_renderers/triangle_fan.hpp"
#include "engine/util.hpp"
using namespace engine;

// c++ lib headers
#include <string>
#include <utility>
#include <vector>

void
game2d::init(entt::registry& registry, glm::ivec2 screen_wh)
{
  { // initialize renderers
    RenderCommand::init();
    RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
    print_gpu_info();
    sprite_renderer::SpriteBatchRenderer::init();
    triangle_fan_renderer::TriangleFanRenderer::init();
  }

  { // initialize resources
    std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
    std::vector<std::pair<int, std::string>> textures_to_load;
    textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);

    Resources r;
    r.fbo_main_scene = Framebuffer::create_fbo();
    r.fbo_lighting = Framebuffer::create_fbo();
    r.tex_id_main_scene = create_texture(screen_wh, tex_unit_main_scene, r.fbo_main_scene);
    r.tex_id_lighting = create_texture(screen_wh, tex_unit_lighting, r.fbo_lighting);
    r.loaded_texture_ids = load_textures_threaded(textures_to_load);
    r.instanced = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
    r.fan = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");

    // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
    registry.set<Resources>(r);
  };
};