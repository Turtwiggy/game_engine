// your header
#include "system.hpp"

// components/systems
#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameplay_circle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "sprites/components.hpp"

// hack
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
using namespace engine; // used for macro

// other lib
#include "imgui.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace game2d {
using namespace std::literals;

void
rebind(entt::registry& r, const SINGLETON_RendererInfo& ri)
{
  const auto& wh = ri.viewport_size_render_at;

  engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);

  engine::bind_tex(ri.tex_unit_linear_main);
  engine::update_bound_texture_size(wh);
  engine::unbind_tex();

  engine::bind_tex(ri.tex_unit_mix_lighting_and_scene);
  engine::update_bound_texture_size(wh);
  engine::unbind_tex();

  glActiveTexture(GL_TEXTURE0 + ri.tex_unit_linear_main);
  glBindTexture(GL_TEXTURE_2D, ri.tex_id_linear_main);

  glActiveTexture(GL_TEXTURE0 + ri.tex_unit_mix_lighting_and_scene);
  glBindTexture(GL_TEXTURE_2D, ri.tex_id_mix_lighting_and_scene);

  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
  }

  auto& camera = game2d::get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(wh.x, wh.y);

  const int tex_unit_kenny = search_for_texture_unit_by_path(ri, "monochrome")->unit;
  const int tex_unit_custom = search_for_texture_unit_by_path(ri, "bargame")->unit;
  const int tex_unit_muzzle = search_for_texture_unit_by_path(ri, "muzzle")->unit;

  ri.instanced.bind();
  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);
  ri.instanced.set_int("tex_custom", tex_unit_custom);
  ri.instanced.set_int("tex_muzzle", tex_unit_muzzle);

  ri.circle.bind();
  ri.circle.set_mat4("projection", camera.projection);

  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  // ri.mix_lighting_and_scene.set_int("lighting", ri.tex_unit_denoise);
  ri.mix_lighting_and_scene.set_int("scene", ri.tex_unit_linear_main);
};

void
game2d::init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const glm::ivec2 screen_wh = { app.width, app.height };
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_lighting_size = ri.viewport_size_render_at;

  // FBO textures
  Framebuffer::default_fbo();
  new_texture_to_fbo(ri.fbo_linear_main, ri.tex_id_linear_main, ri.tex_unit_linear_main, fbo_lighting_size);
  new_texture_to_fbo(
    ri.fbo_mix_lighting_and_scene, ri.tex_id_mix_lighting_and_scene, ri.tex_unit_mix_lighting_and_scene, fbo_lighting_size);

  // Load textures
  const int base_tex_unit = ri.RENDERER_TEX_UNIT_COUNT;
  int next_tex_unit = base_tex_unit;

  for (auto& tex : ri.user_textures) {
    tex.tex_unit = next_tex_unit;
    const auto loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);
    tex.tex_id.id = bind_linear_texture(loaded_tex);

    next_tex_unit++;
  }

  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  ri.circle = Shader("assets/shaders/2d_circle.vert", "assets/shaders/2d_circle.frag");

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_LINE_SMOOTH);

  print_gpu_info();

  quad_renderer::QuadRenderer::init();

  rebind(r, ri);
}

void
game2d::update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  // const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  static const engine::SRGBColour pal_background = { int(34), int(28), int(26), 1.0f };
  static const engine::LinearColour lin_pal_background = engine::SRGBToLinear(pal_background);

  const auto viewport_resized = check_if_viewport_resize(ri);
  if (viewport_resized) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;
  const glm::ivec2 fbo_lighting_size = { viewport_wh.x, viewport_wh.y };

  // Camera
  const auto camera_e = game2d::get_first<OrthographicCamera>(r);
  const auto& camera = r.get<OrthographicCamera>(camera_e);
  const auto& camera_t = r.get<TransformComponent>(camera_e);
  ri.instanced.bind();
  ri.instanced.set_mat4("view", camera.view);
  ri.circle.bind();
  ri.circle.set_mat4("view", camera.view);
  ri.circle.set_vec2("viewport_wh", ri.viewport_size_render_at);
  ri.circle.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });

  static float time = 0.0f;
  time += dt;
  ri.circle.set_float("time", time);

  // DEBUG A SHADER...
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_R)) {
    std::cout << "rebinding shader..." << std::endl;
    ri.circle.reload();
    ri.circle.bind();
    ri.circle.set_mat4("projection", camera.projection);
    ri.circle.set_mat4("view", camera.view);
  }

  // FBO: Render sprites in to this fbo with linear colour
  {
    Framebuffer::bind_fbo(ri.fbo_linear_main);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(lin_pal_background);
    RenderCommand::clear();

    // Render some quads
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // adds 0.5ms
      // const auto& group = registry.group<TransformComponent, SpriteComponent, SpriteColourComponent>();
      // sort by z-index
      // group.sort<SpriteComponent>([](const auto& a, const auto& b) { return a.render_order < b.render_order; });

      const auto& view = r.group<TransformComponent, SpriteComponent>();
      for (const auto& [entity, transform, sc] : view.each()) {
        quad_renderer::RenderDescriptor desc;
        // desc.pos_tl = camera_transform.position + transform.position - transform.scale / 2;
        desc.pos_tl = transform.position - transform.scale / 2;
        desc.size = transform.scale;
        desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
        desc.colour = sc.colour;
        desc.tex_unit = sc.tex_unit;

        desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
        desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
        desc.sprites_max = { sc.total_sx, sc.total_sy };

        quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
      }

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.instanced);
    }

    // Render some circles
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // set the positions of the player units
      {
        ri.circle.bind();

        const auto& player_view = r.view<TransformComponent, CircleComponent>();
        for (int i = 0; const auto& [e, transform, c] : player_view.each()) {

          const glm::vec2 pos = { float(transform.position.x), float(transform.position.y) };

          std::string label = "points["s + std::to_string(i) + "].pos"s;
          ri.circle.set_vec2(label, pos);

          i++;
        }

        // const auto& io = ImGui::GetIO();
        // ri.circle.set_vec2("points[4].pos", { io.MousePos.x, io.MousePos.y });
      }

      // circle post-processing
      // render completely over screen
      {
        quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.circle);
      }

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.circle);
    }
  }

  // FBO: mix lighting and scene, and convert to srgb
  {
    Framebuffer::bind_fbo(ri.fbo_mix_lighting_and_scene);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(lin_pal_background);
    RenderCommand::clear();
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();
      {
        quad_renderer::RenderDescriptor desc;
        desc.pos_tl = { 0, 0 };
        desc.size = viewport_wh;
        desc.angle_radians = 0.0f;
        // desc.colour = // not needed
        // desc.tex_unit = ri.tex_unit_main_FBO;
        // desc.sprite_offset_and_width = { 0, 0, 0, 0 };
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.mix_lighting_and_scene);
      }
      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.mix_lighting_and_scene);
    }
  }

  // Default: render_texture_to_imgui
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(pal_background);
    RenderCommand::clear();

    // Note: ImGui::Image takes in TexID not TexUnit
    ViewportInfo vi = render_texture_to_imgui_viewport(ri.tex_id_mix_lighting_and_scene);

    // If the viewport moves - viewport position will be a frame behind.
    // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
    // (or if a user is moving the viewport, they likely dont need that one frame?)
    ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
    ri.viewport_size_current = { vi.size.x, vi.size.y };
    ri.viewport_hovered = vi.hovered;
    ri.viewport_focused = vi.focused;
  }

//
// debug user textures
//
#ifdef _DEBUG
  ImVec2 viewport_size;
  int i = 0;
  for (const auto& tex : ri.user_textures) {
    const std::string label = std::string("Debug") + std::to_string(i);
    ImGui::Begin(label.c_str());
    viewport_size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)tex.tex_id.id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
    ImGui::End();

    i++;
  }
#endif
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};

} // namespace game2d
