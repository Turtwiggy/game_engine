// your header
#include "modules/renderer/system.hpp"

// components/systems
#include "components/singleton_resources.hpp"
#include "helpers/spritemap.hpp"
#include "modules/editor_camera/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers/helpers.hpp"
#include "modules/renderer/helpers/renderers/batch_quad.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "engine/util.hpp"
using namespace engine;

// other lib
#include <glm/gtc/type_ptr.hpp>

// c++ lib
#include <vector>

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
  RenderCommand::set_depth_testing(false);
  print_gpu_info();
  quad_renderer::QuadRenderer::init();

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
};

void
game2d::update_render_system(entt::registry& registry, engine::Application& app)
{
  SINGLETON_ResourceComponent& r = registry.ctx<SINGLETON_ResourceComponent>();
  SINGLETON_RendererInfo& ri = registry.ctx<SINGLETON_RendererInfo>();
  const glm::vec4 background_colour = glm::vec4(12.0f / 255.0f, 15.0f / 255.0f, 22.0f / 255.0f, 1.0f);

  // Resize
  auto viewport_wh = ri.viewport_size_render_at;
  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (viewport_wh.x != ri.viewport_size_current.x || viewport_wh.y != ri.viewport_size_current.y)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    viewport_wh = ri.viewport_size_render_at;

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
  RenderCommand::clear();

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    // todo: work out z-index
    // registry.sort<ZIndex>([](const auto& lhs, const auto& rhs) { return lhs.index < rhs.index; });

    quad_renderer::RenderDescriptor desc;
    const auto& view =
      registry
        .view<const PositionIntComponent, const RenderSizeComponent, const ColourComponent, const SpriteComponent>();
    view.each([&ri, &desc](const auto& p, const auto& s, const auto& c, const auto& spr) {
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
  RenderCommand::clear();

  ViewportInfo vi = render_texture_to_imgui_viewport(tex_unit_main_scene);
  // If the viewport moves - viewport position will be a frame behind.
  // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
  // (or if a user is moving the viewport, they likely dont need that one frame?)
  ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
  ri.viewport_size_current = { vi.size.x, vi.size.y };
  ri.viewport_process_events = vi.focused && vi.hovered;
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};
