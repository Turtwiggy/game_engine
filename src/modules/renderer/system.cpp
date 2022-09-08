// your header
#include "modules/renderer/system.hpp"

// components/systems
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
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

glm::mat4
calculate_projection(int x, int y)
{
  return glm::ortho(0.0f, static_cast<float>(x), static_cast<float>(y), 0.0f, -1.0f, 1.0f);
};

void
rebind(entt::registry& registry, const glm::ivec2& wh)
{
  const auto& tex = registry.ctx().at<SINGLETON_Textures>();
  auto& ri = registry.ctx().at<SINGLETON_RendererInfo>();

  for (int i = 0; i < tex.textures.size(); i++) {
    const auto& texture = tex.textures[i];
    glActiveTexture(GL_TEXTURE0 + texture.tex_unit);
    glBindTexture(GL_TEXTURE_2D, texture.tex_id);
  }

  glm::mat4 projection = calculate_projection(wh.x, wh.y);
  // const auto& camera = get_main_camera(registry);
  // if (camera != entt::null) {
  //   const auto& camera_transform = registry.get<TransformComponent>(camera);
  //   glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
  //   glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
  //   glm::vec3 cam_pos = camera_transform.position;
  //   glm::mat4 view = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
  //   projection *= view;
  // }

  {
    int textures[5] = { get_tex_unit(registry, AvailableTexture::KENNY),
                        get_tex_unit(registry, AvailableTexture::CUSTOM),
                        get_tex_unit(registry, AvailableTexture::SPROUT),
                        get_tex_unit(registry, AvailableTexture::LOGO),
                        get_tex_unit(registry, AvailableTexture::MAP_0) };
    ri.instanced.bind();
    ri.instanced.set_mat4("projection", projection);
    ri.instanced.set_int_array("textures", textures, sizeof(textures));
  }

  {
    int textures[1] = { get_tex_unit(registry, AvailableTexture::LINEAR_MAIN) };
    ri.linear_to_srgb.bind();
    ri.linear_to_srgb.set_mat4("projection", projection);
    ri.linear_to_srgb.set_int_array("textures", textures, sizeof(textures));
  }
};

void
check_if_viewport_resize(entt::registry& r, SINGLETON_RendererInfo& ri, glm::ivec2& viewport_wh)
{
  if (ri.viewport_size_current.x > 0.0f && ri.viewport_size_current.y > 0.0f &&
      (viewport_wh.x != ri.viewport_size_current.x || viewport_wh.y != ri.viewport_size_current.y)) {

    // A resize occured!

    ri.viewport_size_render_at = ri.viewport_size_current;
    viewport_wh = ri.viewport_size_render_at;

    // update fbo textures
    {
      bind_tex(get_tex_id(r, AvailableTexture::LINEAR_MAIN));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }
    {
      bind_tex(get_tex_id(r, AvailableTexture::LINEAR_LIGHTING));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }
    {
      bind_tex(get_tex_id(r, AvailableTexture::SRGB_MAIN));
      update_bound_texture_size(viewport_wh);
      unbind_tex();
    }

    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    game2d::rebind(r, viewport_wh);
  }
}

glm::mat4
get_local_model_matrix(const TransformComponent& transform, const SpriteComponent& sprite)
{
  // quat(glm::vec3(90, 45, 0))
  // gtx::quaternion::angleAxis(degrees(RotationAngle), RotationAxis);
  // glm::quaternion::toMat4(quaternion);
  // mat4 RotationMatrix = quaternion::toMat4(quaternion);
  // desc.pos_tl = camera_transform.position + transform.position - transform.scale / 2;
  // desc.angle_radians = sc.angle_radians + transform.rotation.z;

  const glm::ivec3& pos_tl = transform.position - transform.scale / 2;
  const glm::ivec3& size = transform.scale;
  glm::vec3 rot = transform.rotation_radians;
  rot.z += sprite.angle_radians;

  const glm::mat4 transform_x = glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
  const glm::mat4 transform_y = glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::mat4 transform_z = glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0.0f, 0.0f, 1.0f));
  const glm::mat4 rot_matrix = transform_y * transform_x * transform_z;

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(pos_tl.x, pos_tl.y, pos_tl.z));
  model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
  model *= rot_matrix;
  model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
  model = glm::scale(model, glm::vec3(size.x, size.y, size.z));
  return model;
};

void
render_recursively(entt::registry& r,
                   const entt::entity& entity,
                   const glm::mat4& parent_model,
                   const engine::Shader& shader)
{
  auto* transform = r.try_get<TransformComponent>(entity);
  auto* sprite = r.try_get<SpriteComponent>(entity);
  auto* children = r.try_get<EntityHierarchyComponent>(entity);

  if (transform && sprite) {

    glm::mat4 model = parent_model * get_local_model_matrix(*transform, *sprite);

    quad_renderer::RenderDescriptor desc;
    desc.colour = (*sprite).colour;
    desc.sprite_offset_and_spritesheet = { (*sprite).x, (*sprite).y, (*sprite).sx, (*sprite).sy };
    desc.tex_unit = (*sprite).tex_unit;

    // draw me
    quad_renderer::QuadRenderer::draw_sprite(desc, model, shader);

    // draw children
    // note: using this scene-hierarchy and recursive structure
    // is terrible for performance and cachin'
    // consider doing something else when needed
    if (children) {
      for (const auto& child : children->children)
        render_recursively(r, child, model, shader);
    }
  }

  // const auto& view = registry.view<const TransformComponent, const SpriteComponent>();
  // view.each([&registry, &ri](auto eid, const TransformComponent& transform, const SpriteComponent& sc) {
  //   quad_renderer::RenderDescriptor desc;
  //   desc.colour = sc.colour;
  //   desc.sprite_offset = { sc.x, sc.y };
  //   desc.tex_unit = sc.tex_unit;
  //   glm::mat4 model = get_local_model_matrix(transform);
  //
  // });
};

}; // namespace game2d

void
game2d::init_render_system(entt::registry& registry)
{
  const auto& app = registry.ctx().at<engine::SINGLETON_Application>();
  const glm::ivec2 screen_wh = { app.width, app.height };

  Framebuffer::default_fbo();
  auto& tex = registry.ctx().at<SINGLETON_Textures>();

  SINGLETON_RendererInfo ri;

  new_texture_to_fbo(ri.fbo_linear_main_scene,
                     get_tex(registry, AvailableTexture::LINEAR_MAIN).tex_id,
                     get_tex_unit(registry, AvailableTexture::LINEAR_MAIN),
                     screen_wh);
  new_texture_to_fbo(ri.fbo_linear_lighting,
                     get_tex(registry, AvailableTexture::LINEAR_LIGHTING).tex_id,
                     get_tex_unit(registry, AvailableTexture::LINEAR_LIGHTING),
                     screen_wh);
  new_texture_to_fbo(ri.fbo_srgb_main_scene,
                     get_tex(registry, AvailableTexture::SRGB_MAIN).tex_id,
                     get_tex_unit(registry, AvailableTexture::SRGB_MAIN),
                     screen_wh);

  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.fan = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.linear_to_srgb = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_linear_to_srgb.frag");
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  print_gpu_info();
  quad_renderer::QuadRenderer::init();

#ifdef _DEBUG
  CHECK_OPENGL_ERROR(0);
#endif

  registry.ctx().emplace<SINGLETON_RendererInfo>(ri);

  game2d::rebind(registry, screen_wh);
};

void
game2d::update_render_system(entt::registry& registry)
{
  auto& ri = registry.ctx().at<SINGLETON_RendererInfo>();
  const auto& tex = registry.ctx().at<SINGLETON_Textures>();
  const auto& colours = registry.ctx().at<SINGLETON_ColoursComponent>();
  const auto& background_colour = colours.background;
  const auto background_colour_linear = engine::SRGBToLinear(background_colour);
  glm::ivec2 viewport_wh = ri.viewport_size_render_at;
  check_if_viewport_resize(registry, ri, viewport_wh);

  // FBO: Render sprites in to this fbo with linear colour
  Framebuffer::bind_fbo(ri.fbo_linear_main_scene);
  RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  RenderCommand::set_clear_colour_linear(engine::SRGBToLinear(background_colour));
  RenderCommand::clear();

  // Do quad stuff
  {
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    // TODO: work out z-index
    // registry.sort<renderable>([](const auto& lhs, const auto& rhs) { return lhs.z < rhs.z; });

    const auto& h = registry.view<RootNode>().front();
    auto& hroot = registry.get<EntityHierarchyComponent>(h);

    // skip showing the root node, go to children
    for (const auto& child : hroot.children) {
      render_recursively(registry, child, glm::mat4(1.0f), ri.instanced);
    }

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
      // desc.colour = // not needed
      desc.tex_unit = get_tex_unit(registry, AvailableTexture::LINEAR_MAIN);
      desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
      glm::mat4 model = glm::mat4(1.0f);
      model = glm::scale(model, glm::vec3(viewport_wh, 1.0f));
      quad_renderer::QuadRenderer::draw_sprite(desc, model, ri.linear_to_srgb);
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
  ViewportInfo vi = render_texture_to_imgui_viewport(get_tex_id(registry, AvailableTexture::SRGB_MAIN));
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
