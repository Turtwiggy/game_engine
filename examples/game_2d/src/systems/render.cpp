// your header
#include "systems/render.hpp"

// components
#include "components/rendering.hpp"
#include "components/singleton_renderer.hpp"
#include "components/singleton_resources.hpp"

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
#include "engine/util.hpp"
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

} // namespace game2d

void
game2d::init_render_system(entt::registry& registry, const glm::ivec2& screen_wh)
{
  SINGLETON_RendererInfo ri;
  ri.fbo_main_scene = Framebuffer::create_fbo();
  ri.fbo_lighting = Framebuffer::create_fbo();
  ri.tex_id_main_scene = create_texture(screen_wh, tex_unit_main_scene, ri.fbo_main_scene);
  ri.tex_id_lighting = create_texture(screen_wh, tex_unit_lighting, ri.fbo_lighting);
  ri.instanced = Shader("2d_game/shaders/2d_instanced.vert", "2d_game/shaders/2d_instanced.frag");
  ri.fan = Shader("2d_game/shaders/2d_basic_with_proj.vert", "2d_game/shaders/2d_colour.frag");
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;

  // initialize renderers
  RenderCommand::init();
  print_gpu_info();
  quad_renderer::QuadRenderer::init();
  triangle_renderer::TriangleRenderer::init();
  triangle_fan_renderer::TriangleFanRenderer::init();

  // sprites
  const std::string path_to_kennynl = "assets/2d_game/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  std::vector<std::pair<int, std::string>> textures_to_load;
  textures_to_load.emplace_back(tex_unit_kenny_nl, path_to_kennynl);
  ri.loaded_texture_ids = load_textures_threaded(textures_to_load);

  // textures
  int textures[3] = { tex_unit_kenny_nl, tex_unit_main_scene, tex_unit_lighting };
  ri.instanced.bind();
  ri.instanced.set_mat4("projection", calculate_projection(screen_wh.x, screen_wh.y));
  ri.instanced.set_int_array("textures", textures, 3);

  // https://github.com/skypjack/entt/wiki/Crash-Course:-entity-component-system#context-variables
  registry.set<SINGLETON_RendererInfo>(ri);

  // Render the HexGrid as an renderable interface
  triangle_renderer::TriangleRenderer::register_interface<RenderHexagons>();
};

void
game2d::update_render_system(entt::registry& registry, engine::Application& app)
{
  SINGLETON_ResourceComponent& r = registry.ctx<SINGLETON_ResourceComponent>();
  SINGLETON_RendererInfo& ri = registry.ctx<SINGLETON_RendererInfo>();
  glm::vec4 background_colour = glm::vec4(57.0f / 255.0f, 62.0f / 255.0f, 70.0f / 255.0f, 1.0f);

  // Resize
  auto viewport_wh = ri.viewport_size_render_at;
  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (ri.viewport_size_render_at.x != ri.viewport_size_current.x ||
       ri.viewport_size_render_at.y != ri.viewport_size_current.y)) {
    viewport_wh = ri.viewport_size_current;
    ri.viewport_size_render_at = viewport_wh;

    // update texture
    bind_tex(ri.tex_id_main_scene, tex_unit_main_scene);
    update_bound_texture_size(viewport_wh);
    unbind_tex();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    ri.instanced.bind();
    ri.instanced.set_mat4("projection", calculate_projection(viewport_wh.x, viewport_wh.y));
  }

  // MAIN FBO
  Framebuffer::bind_fbo(ri.fbo_main_scene);
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour(background_colour);
  RenderCommand::set_depth_testing(false);
  RenderCommand::clear();

  // Do triangle stuff
  triangle_renderer::TriangleRenderer::draw(registry, ri.instanced);

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    // todo: work out z-index
    // registry.sort<ZIndex>([](const auto& lhs, const auto& rhs) { return lhs.index < rhs.index; });

    quad_renderer::RenderDescriptor desc;
    const auto& view =
      registry.view<const PositionIntComponent, const SizeComponent, const ColourComponent, const SpriteComponent>();
    view.each([&registry, &ri, &desc](const auto entity, const auto& p, const auto& s, const auto& c, const auto& spr) {
      desc.pos_tl = { p.x - int(s.w / 2.0f), p.y - int(s.h / 2.0f) };
      desc.colour = c.colour;
      desc.size = { s.w, s.h };
      desc.tex_slot = tex_unit_kenny_nl;
      desc.sprite_offset = sprite::spritemap::get_sprite_offset(spr.sprite);
      desc.angle_radians = 0.0f;
      quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
    });

    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(ri.instanced);
  }

  // default fbo
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
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
    window_flags |=
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  } else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

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

  ImGuiWindowFlags viewport_flags = ImGuiWindowFlags_NoFocusOnAppearing;
  viewport_flags |= ImGuiWindowFlags_NoTitleBar;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Viewport", NULL, viewport_flags);
  ImGui::PopStyleVar();

  ri.viewport_process_events = ImGui::IsWindowFocused() && ImGui::IsWindowHovered();

  ImVec2 viewport_size = ImGui::GetContentRegionAvail();
  ri.viewport_size_current = { viewport_size.x, viewport_size.y };

  // If the viewport moves - input will be a frame behind.
  // This would mainly affect an editor
  // As a game's viewport probably(?) wouldn't move that much
  // (or if a user is moving the viewport, they likely dont need that one frame?)
  const auto& wpos = ImGui::GetWindowPos();
  ri.viewport_pos = glm::vec2(wpos.x, wpos.y);

  // render opengl texture to imgui
  ImGui::Image((ImTextureID)tex_unit_main_scene, viewport_size, ImVec2(0, 1), ImVec2(1, 0));

  ImGui::End();

  ImGui::End();
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
  triangle_renderer::TriangleRenderer::end_frame();
};
