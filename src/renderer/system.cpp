// your header
#include "system.hpp"

// components/systems
#include "entt/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "renderer/components.hpp"
#include "renderer/helpers.hpp"
#include "renderer/helpers/batch_quad.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"

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

void
game2d::init_render_system(const engine::SINGLETON_Application& app,
                           entt::registry& r,
                           SINGLETON_RendererInfo& ri,
                           std::vector<Texture>& tex)
{
  const glm::ivec2 screen_wh = { app.width, app.height };

  Framebuffer::default_fbo();

  int index = tex.size();
  ri.tex_unit_main = request_texture(tex).unit;
  ri.tex_unit_lighting = request_texture(tex).unit;
  ri.tex_unit_srgb = request_texture(tex).unit;
  new_texture_to_fbo(ri.fbo_linear_main_scene, ri.tex_id_main, ri.tex_unit_main, screen_wh);
  new_texture_to_fbo(ri.fbo_linear_lighting, ri.tex_id_lighting, ri.tex_unit_lighting, screen_wh);
  new_texture_to_fbo(ri.fbo_srgb_main_scene, ri.tex_id_srgb, ri.tex_unit_srgb, screen_wh);
  std::cout << "set tex_unit_main id: " << ri.tex_id_main << " to unit: " << ri.tex_unit_main << "\n";
  std::cout << "set tex_unit_lighting id: " << ri.tex_id_lighting << " to unit: " << ri.tex_unit_lighting << "\n";
  std::cout << "set tex_unit_srgb id:" << ri.tex_id_srgb << " to unit: " << ri.tex_unit_srgb << "\n";
  tex[index + 0].tex_id.id = ri.tex_id_main;
  tex[index + 1].tex_id.id = ri.tex_id_lighting;
  tex[index + 2].tex_id.id = ri.tex_id_srgb;

  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.fan = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.linear_to_srgb = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_linear_to_srgb.frag");

  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;

  ri.linear_to_srgb.bind();
  ri.linear_to_srgb.set_mat4("view", glm::mat4(1.0f));

  // initialize renderer
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  // SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  // glEnable(GL_MULTISAMPLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  print_gpu_info();

  quad_renderer::QuadRenderer::init();

  game2d::rebind(ri, tex, screen_wh);
};

void
game2d::update_render_system(entt::registry& r, const std::vector<Texture>& tex)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  const auto& lin_background = colours.lin_background;
  const auto& srgb_background = colours.background;

  glm::ivec2 viewport_wh = ri.viewport_size_render_at;
  check_if_viewport_resize(ri, tex, viewport_wh);

  {
    // FBO: Render sprites in to this fbo with linear colour
    Framebuffer::bind_fbo(ri.fbo_linear_main_scene);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(*lin_background);
    RenderCommand::clear();

    // camera
    {
      const auto& camera = game2d::get_first_component<OrthographicCamera>(r);
      ri.instanced.bind();
      ri.instanced.set_mat4("view", camera.view);
    }

    // Do quad stuff
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // adds 0.5ms
      // const auto& group = registry.group<TransformComponent, SpriteComponent, SpriteColourComponent>();
      // sort by z-index
      // group.sort<SpriteComponent>([](const auto& a, const auto& b) { return a.render_order < b.render_order; });

      const auto& view = r.group<TransformComponent, SpriteComponent, SpriteColourComponent>();

      for (const auto [entity, transform, sc, scc] : view.each()) {
        quad_renderer::RenderDescriptor desc;
        // desc.pos_tl = camera_transform.position + transform.position - transform.scale / 2;
        desc.pos_tl = transform.position - transform.scale / 2;
        desc.size = transform.scale;
        desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
        desc.colour = *scc.colour;
        desc.tex_unit = sc.tex_unit;
        desc.sprite_offset_and_spritesheet = { sc.x, sc.y, sc.sx, sc.sy };
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
      }

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.instanced);
    }
  }

  // linear_to_srgb_pass
  {
    // FBO: LINEAR->SRGB
    Framebuffer::bind_fbo(ri.fbo_srgb_main_scene);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(*lin_background);
    RenderCommand::clear();

    // Render the linear colour main scene in to this texture
    // and perform the linear->srgb conversion in the fragment shader.
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();
      {
        quad_renderer::RenderDescriptor desc;
        desc.pos_tl = { 0, 0 };
        desc.size = viewport_wh;
        desc.angle_radians = 0.0f;
        // desc.colour = // not needed
        desc.tex_unit = ri.tex_unit_main;
        desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.linear_to_srgb);
      }
      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.linear_to_srgb);
    }
  }

  // render_texture_to_imgui
  {
    // default fbo
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(*srgb_background);
    RenderCommand::clear();

    // Note: ImGui::Image takes in TexID not TexUnit
    ViewportInfo vi = render_texture_to_imgui_viewport(ri.tex_id_srgb);

    // If the viewport moves - viewport position will be a frame behind.
    // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
    // (or if a user is moving the viewport, they likely dont need that one frame?)
    ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
    ri.viewport_size_current = { vi.size.x, vi.size.y };
    ri.viewport_hovered = vi.hovered;
    ri.viewport_focused = vi.focused;
  }
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};

} // namespace game2d
