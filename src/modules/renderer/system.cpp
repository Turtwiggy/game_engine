// your header
#include "modules/renderer/system.hpp"

// components/systems
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/helpers/helpers.hpp"
#include "modules/sprites/components.hpp"
#include "modules/sprites/helpers.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "resources/colour.hpp"
#include "resources/textures.hpp"

// engine headers
#include "engine/app/application.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
using namespace engine;

// other lib
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace game2d {

void
rebind(GameEditor& editor, const glm::ivec2& wh)
{
  const auto& tex = editor.textures;
  auto& ri = editor.renderer;

  for (const Texture& texture : tex.textures) {
    glActiveTexture(GL_TEXTURE0 + texture.tex_unit);
    glBindTexture(GL_TEXTURE_2D, texture.tex_id);
  }

  glm::mat4 projection = calculate_projection(wh.x, wh.y);

  {
    int textures[5] = { get_tex_unit(tex, AvailableTexture::kenny),
                        get_tex_unit(tex, AvailableTexture::custom),
                        get_tex_unit(tex, AvailableTexture::sprout),
                        get_tex_unit(tex, AvailableTexture::logo),
                        get_tex_unit(tex, AvailableTexture::map_0) };
    ri.instanced.bind();
    ri.instanced.set_mat4("projection", projection);
    ri.instanced.set_int_array("textures", textures, sizeof(textures));
  }

  {
    int textures[1] = { get_tex_unit(tex, AvailableTexture::linear_main) };
    ri.linear_to_srgb.bind();
    ri.linear_to_srgb.set_mat4("projection", projection);
    ri.linear_to_srgb.set_int_array("textures", textures, sizeof(textures));
  }
};

void
check_if_viewport_resize(GameEditor& editor, glm::ivec2& viewport_wh)
{
  auto& ri = editor.renderer;
  auto& tex = editor.textures;

  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (viewport_wh.x != ri.viewport_size_current.x || viewport_wh.y != ri.viewport_size_current.y)) {

    // A resize occured!

    ri.viewport_size_render_at = ri.viewport_size_current;
    viewport_wh = ri.viewport_size_render_at;

    // update fbo textures
    {
      bind_tex(get_tex_id(tex, AvailableTexture::linear_main));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }
    {
      bind_tex(get_tex_id(tex, AvailableTexture::linear_lighting));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }
    {
      bind_tex(get_tex_id(tex, AvailableTexture::srgb_main));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }

    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    game2d::rebind(editor, viewport_wh);
  }
}

}; // namespace game2d

void
game2d::init_render_system(const engine::SINGLETON_Application& app, GameEditor& editor, const Game& game)
{
  const glm::ivec2 screen_wh = { app.width, app.height };
  auto& ri = editor.renderer;
  auto& tex = editor.textures;
  auto& registry = game.state;

  Framebuffer::default_fbo();

  new_texture_to_fbo(ri.fbo_linear_main_scene,
                     get_tex(tex, AvailableTexture::linear_main).tex_id,
                     get_tex_unit(tex, AvailableTexture::linear_main),
                     screen_wh);
  new_texture_to_fbo(ri.fbo_linear_lighting,
                     get_tex(tex, AvailableTexture::linear_lighting).tex_id,
                     get_tex_unit(tex, AvailableTexture::linear_lighting),
                     screen_wh);
  new_texture_to_fbo(ri.fbo_srgb_main_scene,
                     get_tex(tex, AvailableTexture::srgb_main).tex_id,
                     get_tex_unit(tex, AvailableTexture::srgb_main),
                     screen_wh);

  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.fan = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.linear_to_srgb = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_linear_to_srgb.frag");
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;

  ri.linear_to_srgb.bind();
  ri.linear_to_srgb.set_mat4("view", glm::mat4(1.0f));

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST); // maybe not needed for 2d
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  print_gpu_info();
  quad_renderer::QuadRenderer::init();

#ifdef _DEBUG
  CHECK_OPENGL_ERROR(0);
#endif

  game2d::rebind(editor, screen_wh);
};

void
game2d::update_render_system(GameEditor& editor, Game& game)
{
  const auto& tex = editor.textures;
  const auto& colours = editor.colours;
  auto& ri = editor.renderer;
  auto& registry = game.state;

  const auto& background_colour = colours.background;
  const auto background_colour_linear = engine::SRGBToLinear(background_colour);
  glm::ivec2 viewport_wh = ri.viewport_size_render_at;
  check_if_viewport_resize(editor, viewport_wh);

  // FBO: Render sprites in to this fbo with linear colour
  Framebuffer::bind_fbo(ri.fbo_linear_main_scene);
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour_linear(engine::SRGBToLinear(background_colour));
  RenderCommand::clear();

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    // camera
    {
      const auto& camera = get_first<CameraComponent>(registry);
      const auto& view = camera.view;
      ri.instanced.bind();
      ri.instanced.set_mat4("view", view);
    }

    const auto& group = registry.group<TransformComponent, SpriteComponent, SpriteColourComponent>();

    // sort by z-index
    group.sort<SpriteComponent>([](const auto& a, const auto& b) { return a.render_order < b.render_order; });

    group.each([&ri](const auto& transform, const auto& sc, const auto& scc) {
      quad_renderer::RenderDescriptor desc;
      // desc.pos_tl = camera_transform.position + transform.position - transform.scale / 2;
      desc.pos_tl = transform.position - transform.scale / 2;
      desc.size = transform.scale;
      desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
      desc.colour = scc.colour;
      desc.tex_unit = sc.tex_unit;
      desc.sprite_offset_and_spritesheet = { sc.x, sc.y, sc.sx, sc.sy };
      quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
    });

    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(ri.instanced);
  }

  // FBO: LINEAR->SRGB
  Framebuffer::bind_fbo(ri.fbo_srgb_main_scene);
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour_linear(background_colour_linear);
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
      desc.tex_unit = get_tex_unit(tex, AvailableTexture::linear_main);
      desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
      quad_renderer::QuadRenderer::draw_sprite(desc, ri.linear_to_srgb);
    }
    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(ri.linear_to_srgb);
  }

  // default fbo
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour_srgb(background_colour);
  RenderCommand::clear();

  // Note: ImGui::Image takes in TexID not TexUnit
  ViewportInfo vi = render_texture_to_imgui_viewport(get_tex_id(tex, AvailableTexture::srgb_main));
  // If the viewport moves - viewport position will be a frame behind.
  // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
  // (or if a user is moving the viewport, they likely dont need that one frame?)
  ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
  ri.viewport_size_current = { vi.size.x, vi.size.y };
  ri.viewport_process_events = vi.hovered || (vi.focused);
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};
