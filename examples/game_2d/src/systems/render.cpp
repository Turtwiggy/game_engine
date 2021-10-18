// your header
#include "systems/render.hpp"

// components
#include "components/rendering.hpp"
#include "components/resources.hpp"

// helpers
#include "helpers/renderer_interfaces/render_hexagons.hpp"
#include "helpers/renderers/batch_quad.hpp"
#include "helpers/renderers/batch_triangle.hpp"
#include "helpers/renderers/batch_triangle_fan.hpp"
#include "helpers/spritemap.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
using namespace engine;

// other lib
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// c++ lib
#include <vector>

void
game2d::init_render_system(entt::registry& registry, const glm::ivec2& screen_wh)
{
  SINGLETON_Resources r;
  r.fbo_main_scene = Framebuffer::create_fbo();
  r.fbo_lighting = Framebuffer::create_fbo();
  r.tex_id_main_scene = create_texture(screen_wh, tex_unit_main_scene, r.fbo_main_scene);
  r.tex_id_lighting = create_texture(screen_wh, tex_unit_lighting, r.fbo_lighting);
  r.instanced = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  r.fan = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");

  // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
  registry.set<SINGLETON_Resources>(r);

  // initialize renderers
  RenderCommand::init();
  RenderCommand::set_viewport(0, 0, screen_wh.x, screen_wh.y);
  print_gpu_info();
  quad_renderer::QuadRenderer::init();
  triangle_renderer::TriangleRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(screen_wh.x), static_cast<float>(screen_wh.y), 0.0f, -1.0f, 1.0f);
  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  r.instanced.bind();
  r.instanced.set_mat4("projection", projection);
  r.instanced.set_int_array("textures", textures, 3);
  // r.instanced.set_int("screen_w", screen_wh.x);
  // r.instanced.set_int("screen_h", screen_wh.y);

  // Render the HexGrid as an renderable interface
  triangle_renderer::TriangleRenderer::register_interface<RenderHexagons>();
};

void
game2d::update_render_system(entt::registry& registry)
{
  SINGLETON_Resources& r = registry.ctx<SINGLETON_Resources>();
  glm::vec4 background_colour = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);
  static glm::vec2 viewport_wh = { 1337.0f, 900.0f };

  // MAIN FBO
  Framebuffer::bind_fbo(r.fbo_main_scene);
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour(background_colour);
  RenderCommand::set_depth_testing(false);
  RenderCommand::clear();

  glm::mat4 projection =
    glm::ortho(0.0f, static_cast<float>(viewport_wh.x), static_cast<float>(viewport_wh.y), 0.0f, -1.0f, 1.0f);
  r.instanced.bind();
  r.instanced.set_mat4("projection", projection);

  // Do triangle stuff
  triangle_renderer::TriangleRenderer::draw(registry, r.instanced);

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    quad_renderer::RenderDescriptor desc;
    auto view = registry.view<const PositionInt, const Size, const Colour, const Sprite, const ZIndex>();
    view.each([&registry, &r, &desc](
                const auto entity, const auto& p, const auto& s, const auto& c, const auto& spr, const auto& z) {
      desc.pos_tl = { p.x - int(s.w / 2.0f), p.y - int(s.h / 2.0f) };
      desc.colour = c.colour;
      desc.size = { s.w, s.h };
      desc.tex_slot = tex_unit_kenny_nl;
      desc.sprite_offset = sprite::spritemap::get_sprite_offset(spr.sprite);
      desc.angle_radians = 0.0f;
      // todo: work out z-index
      quad_renderer::QuadRenderer::draw_sprite(desc, r.instanced);
    });

    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(r.instanced);
  }

  bool update_textures = false;
  // render opengl texture to imgui
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
  ImGui::Begin("Viewport");
  ImVec2 viewport_size = ImGui::GetContentRegionAvail();
  if (viewport_wh != *((glm::vec2*)&viewport_size)) {
    update_textures = true;
    viewport_wh = { viewport_size.x, viewport_size.y };
  }
  ImGui::Image((ImTextureID)tex_unit_main_scene, viewport_size, ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
  ImGui::PopStyleVar();

  if (update_textures) {
    update_texture_size(viewport_wh, tex_unit_main_scene);
    projection =
      glm::ortho(0.0f, static_cast<float>(viewport_wh.x), static_cast<float>(viewport_wh.y), 0.0f, -1.0f, 1.0f);
    r.instanced.bind();
    r.instanced.set_mat4("projection", projection);
  }

  // default fbo
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // hit damage colour
  RenderCommand::set_depth_testing(false);
  RenderCommand::clear();
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
  triangle_renderer::TriangleRenderer::end_frame();
};
