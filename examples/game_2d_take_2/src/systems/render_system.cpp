// your header
#include "systems/render_system.hpp"

// game headers
#include "constants.hpp"

// components
#include "components/colour.hpp"
#include "components/global_resources.hpp"
#include "components/position.hpp"
#include "components/size.hpp"
#include "components/sprite.hpp"

// helpers
#include "helpers/renderers/batch_sprite.hpp"
#include "helpers/renderers/batch_triangle_fan.hpp"
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
using namespace engine;

void
game2d::init_render_system(entt::registry& registry, const glm::ivec2& screen_wh)
{
  Resources r;
  r.fbo_main_scene = Framebuffer::create_fbo();
  r.fbo_lighting = Framebuffer::create_fbo();
  r.tex_id_main_scene = create_texture(screen_wh, tex_unit_main_scene, r.fbo_main_scene);
  r.tex_id_lighting = create_texture(screen_wh, tex_unit_lighting, r.fbo_lighting);
  r.instanced = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  r.fan = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");

  // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
  registry.set<Resources>(r);

  // initialize renderers
  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  print_gpu_info();
  sprite_renderer::SpriteBatchRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  static glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);
  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  r.instanced.bind();
  r.instanced.set_mat4("projection", projection);
  r.instanced.set_int_array("textures", textures, 3);
  r.instanced.set_int("screen_w", screen_wh.x);
  r.instanced.set_int("screen_h", screen_wh.y);
};

void
game2d::update_render_system(entt::registry& registry)
{
  Resources& r = registry.ctx<Resources>();

  Framebuffer::default_fbo();
  glm::vec4 background_colour = glm::vec4(1.0f);
  RenderCommand::set_clear_colour(background_colour);
  RenderCommand::clear();

  sprite_renderer::SpriteBatchRenderer::reset_quad_vert_count();
  sprite_renderer::SpriteBatchRenderer::begin_batch();

  sprite_renderer::RenderDescriptor desc;
  auto view = registry.view<const Position, const Size, const Colour, const Sprite>();
  view.each([&r, &desc](const auto& p, const auto& s, const auto& c, const auto& spr) {
    desc.pos_tl = { p.x - int(s.x / 2.0f), p.y - int(s.y / 2.0f) };
    desc.colour = { c.r, c.g, c.b, c.a };
    desc.size = { s.x, s.y };
    desc.tex_slot = tex_unit_kenny_nl;
    desc.sprite_offset = sprite::spritemap::get_sprite_offset(spr.sprite);
    desc.angle_radians = 0.0f;
    sprite_renderer::SpriteBatchRenderer::draw_sprite(desc, r.instanced);
  });

  sprite_renderer::SpriteBatchRenderer::end_batch();
  sprite_renderer::SpriteBatchRenderer::flush(r.instanced);
};
