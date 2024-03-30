// your header
#include "system.hpp"

// components/systems
#include "entt/helpers.hpp"
#include "imgui.h"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "resources/colours.hpp"
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

  engine::bind_tex(ri.tex_id_linear_main);
  engine::update_bound_texture_size(wh);
  engine::unbind_tex();

  engine::bind_tex(ri.tex_id_lighting);
  engine::update_bound_texture_size(wh);
  engine::unbind_tex();

  engine::bind_tex(ri.tex_id_mix_lighting_and_scene);
  engine::update_bound_texture_size(wh);
  engine::unbind_tex();

  glActiveTexture(GL_TEXTURE0 + ri.tex_unit_linear_main);
  glBindTexture(GL_TEXTURE_2D, ri.tex_id_linear_main);

  glActiveTexture(GL_TEXTURE0 + ri.tex_unit_lighting);
  glBindTexture(GL_TEXTURE_2D, ri.tex_id_lighting);

  glActiveTexture(GL_TEXTURE0 + ri.tex_unit_mix_lighting_and_scene);
  glBindTexture(GL_TEXTURE_2D, ri.tex_id_mix_lighting_and_scene);

  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
  }

  auto& camera = game2d::get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(wh.x, wh.y);

  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;

  ri.instanced.bind();
  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);

  ri.lighting.bind();
  ri.lighting.set_mat4("projection", camera.projection);
  ri.lighting.set_vec2("viewport_wh", ri.viewport_size_render_at);

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
  const auto& fbo_size = ri.viewport_size_render_at;

  // FBO textures
  Framebuffer::default_fbo();
  new_texture_to_fbo(ri.fbo_linear_main, ri.tex_id_linear_main, ri.tex_unit_linear_main, fbo_size);
  new_texture_to_fbo(ri.fbo_lighting, ri.tex_id_lighting, ri.tex_unit_lighting, fbo_size);
  new_texture_to_fbo(
    ri.fbo_mix_lighting_and_scene, ri.tex_id_mix_lighting_and_scene, ri.tex_unit_mix_lighting_and_scene, fbo_size);

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
  ri.lighting = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_lighting.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  print_gpu_info();

  quad_renderer::QuadRenderer::init();

  rebind(r, ri);
}

void
game2d::update_render_system(entt::registry& r, const float dt)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  const auto viewport_resized = check_if_viewport_resize(ri);
  if (viewport_resized) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;

  // Camera
  const auto camera_e = game2d::get_first<OrthographicCamera>(r);
  const auto& camera = r.get<OrthographicCamera>(camera_e);
  const auto& camera_t = r.get<TransformComponent>(camera_e);
  ri.instanced.bind();
  // ri.instanced.set_mat4("view", camera.view);

  // DEBUG A SHADER...
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_1)) {
    std::cout << "rebinding shader..." << std::endl;
    ri.lighting.reload();
    ri.lighting.bind();
    ri.lighting.set_mat4("projection", camera.projection);
    ri.lighting.set_mat4("view", camera.view);
    ri.lighting.set_vec2("viewport_wh", viewport_wh);
    // ri.lighting.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    // ri.lighting.set_float("time", time);
  }
  ri.lighting.bind();
  ri.lighting.set_mat4("projection", camera.projection);
  ri.lighting.set_mat4("view", camera.view);
  ri.lighting.set_vec2("viewport_wh", viewport_wh);

  // Editor

  static std::vector<glm::vec2> points{
    glm::vec2(0.2, 0.8),  glm::vec2(0.35, 0.8),  glm::vec2(0.45, 0.8),   glm::vec2(0.7, 0.8),     glm::vec2(0.6, 0.8),
    glm::vec2(0.6, 0.5),  glm::vec2(0.7, 0.8),   glm::vec2(0.7, 0.5),    glm::vec2(0.6, 0.5),     glm::vec2(0.65, 0.5),
    glm::vec2(0.8, 0.35), glm::vec2(0.8, 0.15),  glm::vec2(0.8, 0.15),   glm::vec2(0.2, 0.15),    glm::vec2(0.2, 0.15),
    glm::vec2(0.2, 0.8),  glm::vec2(0.6, 0.15),  glm::vec2(0.6, 0.35),   glm::vec2(0.8, 0.35),    glm::vec2(0.65, 0.35),
    glm::vec2(0.7, 0.2),  glm::vec2(0.75, 0.22), glm::vec2(0.625, 0.55), glm::vec2(0.6875, 0.52), glm::vec2(0.6, 0.35),
    glm::vec2(0.65, 0.35)
  };
  static std::vector<glm::vec3> emissive{
    // clang-format off
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(0.0f),
    glm::vec3(1.0, 0.1, 0.3), 
    glm::vec3(0.1, 0.6, 0.9),
    glm::vec3(0.0f) // 13
    // clang-format on
  };

  const int lines = points.size() / 2;
  ri.lighting.set_int("active_lines", lines);

  ImGui::Begin("DebugLightingViewport");
  ImGui::Text("Active Lines: %i", lines);

  for (int i = 0; i < lines; i++) {
    const int a_idx = 2 * i + 0;
    const int b_idx = 2 * i + 1;
    const int e_idx = i;
    const std::string a_idx_str = std::to_string(a_idx);
    const std::string b_idx_str = std::to_string(b_idx);
    const std::string a = "points["s + a_idx_str + "].p"s;
    const std::string a_e = "points["s + a_idx_str + "].emissiveColor"s;
    const std::string b = "points["s + b_idx_str + "].p"s;
    // const std::string b_e = "points["s + b_idx_str + "].emissiveColor"s;

    imgui_draw_vec2("pA##"s + a_idx_str, points[a_idx], 0.1f);
    imgui_draw_vec2("pB##"s + b_idx_str, points[b_idx], 0.1f);

    ri.lighting.set_vec2(a, points[a_idx]);
    ri.lighting.set_vec3(a_e, emissive[e_idx]); // set emissive for a
    ri.lighting.set_vec2(b, points[b_idx]);
  }

  ImGui::End();

  // ri.lighting.set_vec3("segments[0].emissiveColor", { 0.0, 0.1, 0.3 });

  // emitters should be anything but black
  const engine::LinearColour background_col(0.0f, 0.0f, 0.0f, 0.0f);
  static engine::SRGBColour emitter_col{ 255, 0, 0, 1.0f };
  const engine::LinearColour occluder_col(0.0f, 0.0f, 0.0f, 1.0f);

  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  // const auto& lin_background = colours.lin_background;
  const auto& srgb_background = colours.background;

  // FBO: Render sprites in to this fbo with linear colour
  {
    Framebuffer::bind_fbo(ri.fbo_linear_main);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
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
  }

  // FBO: lighting
  // Use the emitters / occluders to setup the voronoi seed texture.
  {
    Framebuffer::bind_fbo(ri.fbo_lighting);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // render completely over screen
      {
        quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.lighting);
      }

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.lighting);
    }
  }

  // FBO: mix lighting and scene. Also convert to rgb
  {
    Framebuffer::bind_fbo(ri.fbo_mix_lighting_and_scene);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
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
        // desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.mix_lighting_and_scene);
      }
      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.mix_lighting_and_scene);
    }
  }

  // Default FBO: render_texture_to_imgui
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(*srgb_background);
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

  ImGui::Begin("DebugLighting");
  ImVec2 viewport_size;
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_lighting, viewport_size, ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();

// debug user textures
//
#ifdef _DEBUG
  int i = 0;
  for (const auto& tex : ri.user_textures) {
    const std::string label = std::string("Debug") + std::to_string(i);
    ImGui::Begin(label.c_str());
    viewport_size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)tex.tex_id.id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
    ImGui::End();

    i++;
  }

  // ImGui::Begin("tex_id_linear_main");
  // viewport_size = ImGui::GetContentRegionAvail();
  // ImGui::Image((ImTextureID)ri.tex_id_linear_main, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  // ImGui::End();
  // ImGui::Begin("tex_id_mix_lighting_and_scene");
  // viewport_size = ImGui::GetContentRegionAvail();
  // ImGui::Image((ImTextureID)ri.tex_id_mix_lighting_and_scene, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  // ImGui::End();
#endif
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};

} // namespace game2d
