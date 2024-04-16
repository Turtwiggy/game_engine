// your header
#include "system.hpp"

// components/systems
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameplay_circle/components.hpp"
#include "modules/lighting/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/helpers/batch_triangle_fan.hpp"
#include "modules/ui_colours/helpers.hpp"
#include "modules/vfx_grid/components.hpp"
#include "physics/components.hpp"
#include "renderer/transform.hpp"
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
  const int tex_unit_custom = search_for_texture_unit_by_texture_path(ri, "bargame")->unit;
  const int tex_unit_particles = search_for_texture_unit_by_texture_path(ri, "particles")->unit;
  const int tex_unit_gameicons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;
  const int tex_unit_car0 = search_for_texture_unit_by_texture_path(ri, "voxel")->unit;

  ri.instanced.bind();
  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);
  ri.instanced.set_int("tex_custom", tex_unit_custom);
  ri.instanced.set_int("tex_particles", tex_unit_particles);
  ri.instanced.set_int("tex_gameicons", tex_unit_gameicons);
  ri.instanced.set_int("tex_voxel", tex_unit_car0);

  ri.lighting.bind();
  ri.lighting.set_mat4("view", camera.view);
  ri.lighting.set_mat4("projection", camera.projection);
  ri.lighting.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  ri.circle.bind();
  ri.circle.set_mat4("projection", camera.projection);

  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.mix_lighting_and_scene.set_int("lighting", ri.tex_unit_lighting);
  ri.mix_lighting_and_scene.set_int("scene", ri.tex_unit_linear_main);

  const auto grid_e = get_first<Effect_GridComponent>(r);
  ri.grid.bind();
  ri.grid.set_mat4("projection", camera.projection);
  if (grid_e != entt::null) {
    const float gridsize = r.get<Effect_GridComponent>(grid_e).gridsize;
    ri.grid.set_float("gridsize", gridsize);
  }
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
  ri.lighting = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  ri.circle = Shader("assets/shaders/2d_circle.vert", "assets/shaders/2d_circle.frag");
  ri.grid = Shader("assets/shaders/2d_grid.vert", "assets/shaders/2d_grid.frag");

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
  // glEnable(GL_DEPTH_TEST);
  // glEnable(GL_LINE_SMOOTH);

  print_gpu_info();

  quad_renderer::QuadRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  rebind(r, ri);
}

void
game2d::update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  static const engine::SRGBColour black(0, 0, 0, 1.0f);

  static float time = 0.0f;
  time += dt;

  if (check_if_viewport_resize(ri)) {
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

  ri.lighting.bind();
  ri.lighting.set_mat4("view", camera.view);

  ri.circle.bind();
  ri.circle.set_mat4("view", camera.view);
  ri.circle.set_vec2("viewport_wh", ri.viewport_size_render_at);
  ri.circle.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });

  ri.grid.bind();
  ri.grid.set_mat4("view", camera.view);
  ri.grid.set_vec2("viewport_wh", ri.viewport_size_render_at);
  ri.grid.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
  const auto grid_e = get_first<Effect_GridComponent>(r);
  if (grid_e != entt::null) {
    const float gridsize = r.get<Effect_GridComponent>(grid_e).gridsize;
    ri.grid.set_float("gridsize", gridsize);
  }

  ri.mix_lighting_and_scene.bind();
  // const glm::vec2 screen_tl = { 0, 0 };
  // const glm::vec2 screen_br = ri.viewport_size_render_at;
  // const glm::vec2 ri_size = ri.viewport_size_render_at;
  // TODO...convert light_pos to screen_space
  const auto mouse_raw = get_mouse_pos() - ri.viewport_pos;
  const glm::vec2 light_pos_in_screenspace = { mouse_raw.x, mouse_raw.y };
  // const glm::vec2 light_pos = { camera_t.position.x, camera_t.position.y };
  const glm::vec2 light_pos = mouse_pos;
  // light_pos.x should be 0 < viewport_wh.x
  // light_pos.y should be 0 < viewport_wh.y
  // const glm::vec2 middle_of_screen = { viewport_wh.x / 2.0f, viewport_wh.y / 2.0f };
  // const glm::vec2 light_pos_in_screenspace = middle_of_screen;

  ImGui::Text("raw %f %f", light_pos_in_screenspace.x, light_pos_in_screenspace.y);
  ri.mix_lighting_and_scene.set_vec2("light_pos", light_pos_in_screenspace);

// DEBUG A SHADER...
#ifdef _DEBUG
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_1)) {
    std::cout << "rebinding shader..." << std::endl;
    ri.grid.reload();
    ri.grid.bind();
    ri.grid.set_mat4("projection", camera.projection);
    ri.grid.set_mat4("view", camera.view);
    ri.grid.set_vec2("viewport_wh", ri.viewport_size_render_at);
    ri.grid.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    const auto grid_e = get_first<Effect_GridComponent>(r);
    if (grid_e != entt::null) {
      const float gridsize = r.get<Effect_GridComponent>(grid_e).gridsize;
      ri.grid.set_float("gridsize", gridsize);
    }
  }
#endif

  // FBO: Render sprites in to this fbo with linear colour
  {
    Framebuffer::bind_fbo(ri.fbo_linear_main);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    // Render grid shader
    {
      const auto& grid_effect_e = get_first<Effect_GridComponent>(r);
      if (grid_effect_e != entt::null) {
        quad_renderer::QuadRenderer::reset_quad_vert_count();
        quad_renderer::QuadRenderer::begin_batch();

        // grid post-processing
        // render completely over screen
        {
          quad_renderer::RenderDescriptor desc;
          const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
          desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
          desc.size = ri.viewport_size_render_at;
          desc.angle_radians = 0;
          quad_renderer::QuadRenderer::draw_sprite(desc, ri.grid);
        }

        quad_renderer::QuadRenderer::end_batch();
        quad_renderer::QuadRenderer::flush(ri.grid);
      }
    }

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

  // FBO: lighting: simple shadowcasting 2d
  {
    Framebuffer::bind_fbo(ri.fbo_lighting);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();
    {
      engine::triangle_fan_renderer::TriangleFanRenderer::reset_quad_vert_count();
      engine::triangle_fan_renderer::TriangleFanRenderer::begin_batch();
      {
        // Create a triangle fan
        //
        // hack: get position of first player

        std::vector<std::tuple<float, float, float>> intersections;
        generate_intersections(r, light_pos, intersections);

        if (intersections.size() > 0) {
          // start point
          engine::triangle_fan_renderer::TriangleFanRenderer::add_point_to_fan(light_pos, ri.lighting);

          // lighting points
          for (int i = 0; i < intersections.size(); i++) {
            const auto pos = glm::vec2(std::get<1>(intersections[i]), std::get<2>(intersections[i]));
            engine::triangle_fan_renderer::TriangleFanRenderer::add_point_to_fan(pos, ri.lighting);
          }

          // end point
          const auto pos = glm::vec2(std::get<1>(intersections[0]), std::get<2>(intersections[0]));
          engine::triangle_fan_renderer::TriangleFanRenderer::add_point_to_fan(pos, ri.lighting);
        }
      }
    }
    engine::triangle_fan_renderer::TriangleFanRenderer::end_batch();
    engine::triangle_fan_renderer::TriangleFanRenderer::flush(ri.lighting);
  }

  // FBO: mix lighting and scene, and convert to srgb
  {
    Framebuffer::bind_fbo(ri.fbo_mix_lighting_and_scene);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();
      {
        quad_renderer::RenderDescriptor desc;
        desc.pos_tl = { 0, 0 };
        desc.size = viewport_wh;
        desc.angle_radians = 0.0f;

        RenderCommand::set_clear_colour_srgb(black);
        const auto lin_pal_background = get_lin_colour_by_tag(r, "background");
        desc.colour = lin_pal_background;

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
    RenderCommand::set_clear_colour_srgb(black);
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

  ImGui::Begin("DebugLighting");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_lighting, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();
  ImGui::Begin("DebugScene");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_linear_main, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();

#endif
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
  triangle_fan_renderer::TriangleFanRenderer::end_frame();
};

} // namespace game2d
