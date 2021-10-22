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

namespace game2d {

glm::mat4
calculate_projection(int x, int y)
{
  return glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f, -1.0f, 1.0f);
};

}

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
  print_gpu_info();
  quad_renderer::QuadRenderer::init();
  triangle_renderer::TriangleRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  r.instanced.bind();
  r.instanced.set_mat4("projection", calculate_projection(screen_wh.x, screen_wh.y));
  r.instanced.set_int_array("textures", textures, 3);

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

  // Do triangle stuff
  triangle_renderer::TriangleRenderer::draw(registry, r.instanced);

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    quad_renderer::RenderDescriptor desc;
    auto view = registry.view<const PositionInt, const Size, const Colour, const Sprite, const ZIndex>();
    view.each([&registry, &r, &desc](const auto entity, const auto& p, const auto& s, const auto& c, const auto& spr, const auto& z) {
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

  // ImGui::IsWindowFocused();
  // ImGui::IsWindowHovered();
  // ImGui::DockSpaceOverViewport()

  // default fbo
  Framebuffer::default_fbo();
  RenderCommand::set_clear_colour(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
  RenderCommand::set_depth_testing(false);
  RenderCommand::clear();

  static bool dockspace_open = true;
  static bool opt_fullscreen = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // render opengl texture to imgui
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
  ImGui::PopStyleVar();
  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // Dockspace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Options")) {
      // Disabling fullscreen would allow the window to be moved to the front of other windows,
      // which we can't undo at the moment without finer window depth/z control.
      ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
      ImGui::Separator();
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGuiWindowFlags viewport_flags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::Begin("Viewport", NULL, viewport_flags);
  {
    ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    bool update_textures = false;
    if (viewport_wh != *((glm::vec2*)&viewport_size) && viewport_size.x > 0.0f && viewport_size.y > 0.0f) {
      viewport_wh = { viewport_size.x, viewport_size.y };
      update_textures = true;
    }
    ImGui::Image((ImTextureID)tex_unit_main_scene, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

    if (update_textures) {
      bind_tex(r.tex_id_main_scene, tex_unit_main_scene);
      update_bound_texture_size(viewport_wh);
      unbind_tex();
      RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
      r.instanced.bind();
      r.instanced.set_mat4("projection", calculate_projection(viewport_wh.x, viewport_wh.y));
    }
  }
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::End();
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
  triangle_renderer::TriangleRenderer::end_frame();
};
