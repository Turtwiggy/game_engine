// your header
#include "system.hpp"

// components/systems
#include "entt/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer//components.hpp"
#include "modules/renderer//helpers.hpp"
#include "modules/renderer//helpers/batch_quad.hpp"
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
rebind(entt::registry& r, const SINGLETON_RendererInfo& ri)
{
  const auto wh = ri.viewport_size_render_at;

  // engine::bind_tex(ri.tex_id_main);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // engine::bind_tex(ri.tex_id_srgb);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // engine::bind_tex(ri.tex_id_emitters_and_occluders);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // engine::bind_tex(ri.tex_id_voronoi_seed);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // engine::bind_tex(ri.tex_id_voronoi_distance);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // engine::bind_tex(ri.tex_id_gi);
  // engine::update_bound_texture_size(wh);
  // engine::unbind_tex();

  // // bug: ri.passes doesnt update on screensize, when it should
  // for (int i = 0; i < ri.passes; i++) {
  //   engine::bind_tex(ri.tex_ids_jump_flood[i]);
  //   engine::update_bound_texture_size(wh);
  //   engine::unbind_tex();
  // }

  // engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);

  // // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_kennynl);
  // // glBindTexture(GL_TEXTURE_2D, ri.tex_id_kenny);

  // // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_main_FBO);
  // // glBindTexture(GL_TEXTURE_2D, ri.tex_id_main);

  // // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_srgb_FBO);
  // // glBindTexture(GL_TEXTURE_2D, ri.tex_id_srgb);

  // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_emitters_and_occluders);
  // glBindTexture(GL_TEXTURE_2D, ri.tex_id_emitters_and_occluders);

  // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_voronoi_seed);
  // glBindTexture(GL_TEXTURE_2D, ri.tex_id_voronoi_seed);

  // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_voronoi_distance);
  // glBindTexture(GL_TEXTURE_2D, ri.tex_id_voronoi_distance);

  // glActiveTexture(GL_TEXTURE0 + ri.tex_unit_gi);
  // glBindTexture(GL_TEXTURE_2D, ri.tex_id_gi);

  // // bug: ri.passes doesnt update on screensize, when it should
  // for (int i = 0; i < ri.passes; i++) {
  //   glActiveTexture(GL_TEXTURE0 + ri.tex_units_jump_flood[i]);
  //   glBindTexture(GL_TEXTURE_2D, ri.tex_ids_jump_flood[i]);
  // }

  auto& camera = game2d::get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(wh.x, wh.y);

  // // ri.instanced.bind();
  // // ri.instanced.set_mat4("projection", camera.projection);
  // // ri.instanced.set_int("tex", ri.tex_unit_kennynl);

  // // ri.linear_to_srgb.bind();
  // // ri.linear_to_srgb.set_mat4("projection", camera.projection);
  // // ri.linear_to_srgb.set_mat4("view", glm::mat4(1.0f)); // whole texture
  // // ri.linear_to_srgb.set_int("tex", ri.tex_unit_main_FBO);

  ri.emitters_and_occluders.bind();
  ri.emitters_and_occluders.set_mat4("projection", camera.projection);

  ri.voronoi_seed.bind();
  ri.voronoi_seed.set_mat4("projection", camera.projection);
  ri.voronoi_seed.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.voronoi_seed.set_int("tex", ri.tex_unit_emitters_and_occluders);

  ri.jump_flood.bind();
  ri.jump_flood.set_mat4("projection", camera.projection);
  ri.jump_flood.set_mat4("view", glm::mat4(1.0f)); // whole texture

  ri.voronoi_distance.bind();
  ri.voronoi_distance.set_mat4("projection", camera.projection);
  ri.voronoi_distance.set_mat4("view", glm::mat4(1.0f)); // whole texture

  ri.gi.bind();
  ri.gi.set_mat4("projection", camera.projection);
  ri.gi.set_mat4("view", glm::mat4(1.0f)); // whole texture
};

void
game2d::init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const glm::ivec2 screen_wh = { app.width, app.height };

  Framebuffer::default_fbo();

  // number of passes required is the log2 of the largest viewport
  // dimension rounded up to the nearest power of 2.
  // i.e. 768x512 is log2(1024) == 10
  ri.passes = static_cast<int>(glm::ceil(glm::log(glm::max(screen_wh.x, screen_wh.y) / log(2.0))));

  // assign texture units
  int tex_units = 0;
  ri.tex_unit_emitters_and_occluders = tex_units++;
  ri.tex_unit_voronoi_seed = tex_units++;
  ri.tex_unit_voronoi_distance = tex_units++;
  ri.tex_unit_gi = tex_units++;

  // load texture
  // const auto kenny_path = "assets/textures/kennynl_1bit_pack/monochrome_transparent_packed.png";
  // const auto kenny_texture = engine::load_texture_linear(ri.tex_unit_kennynl, kenny_path);
  // ri.tex_id_kenny = bind_linear_texture(kenny_texture);

  // create FBO textures
  new_texture_to_fbo(
    ri.fbo_emitters_and_occluders, ri.tex_id_emitters_and_occluders, ri.tex_unit_emitters_and_occluders, screen_wh);
  new_texture_to_fbo(ri.fbo_voronoi_distance, ri.tex_id_voronoi_distance, ri.tex_unit_voronoi_distance, screen_wh);
  new_texture_to_fbo(ri.fbo_gi, ri.tex_id_gi, ri.tex_unit_gi, screen_wh);
  new_texture_to_fbo(ri.fbo_voronoi_seed, ri.tex_id_voronoi_seed, ri.tex_unit_voronoi_seed, screen_wh);
  for (int i = 0; i < ri.passes; i++) {
    ri.tex_units_jump_flood.push_back(tex_units++);
    ri.fbos_jump_flood.push_back(0);
    ri.tex_ids_jump_flood.push_back(0);
    new_texture_to_fbo(ri.fbos_jump_flood[i], ri.tex_ids_jump_flood[i], ri.tex_units_jump_flood[i], screen_wh);
  }

  // ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  // ri.linear_to_srgb = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_linear_to_srgb.frag");
  ri.emitters_and_occluders = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_lighting.frag");
  ri.voronoi_seed = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_seed.frag");
  ri.jump_flood = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_jump_flood.frag");
  ri.voronoi_distance = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_distance.frag");
  ri.gi = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_gi.frag");

  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  print_gpu_info();

  quad_renderer::QuadRenderer::init();

  rebind(r, ri);
}

// https://forum.gamemaker.io/index.php?threads/all-about-2d-global-illumination.97941/

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
  {
    const auto& camera = game2d::get_first_component<OrthographicCamera>(r);
    ri.emitters_and_occluders.bind();
    ri.emitters_and_occluders.set_mat4("view", camera.view);
  }

  // emitters should be anything but black
  const engine::LinearColour background_col(0.0f, 0.0f, 0.0f, 0.0f);
  const engine::LinearColour emitter_col(248 / 255.0f, 237 / 255.0f, 244 / 255.0f, 1.0f);
  const engine::LinearColour occluder_col(0.0f, 0.0f, 0.0f, 1.0f);

  // FBO: lighting. Generate emitters/occluders texture
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  {
    Framebuffer::bind_fbo(ri.fbo_emitters_and_occluders);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
    {
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // draw emitters
      {
        const auto& emitters = r.view<const LightEmitterComponent,
                                      const TransformComponent,
                                      const SpriteComponent,
                                      const SpriteColourComponent>();
        for (const auto& [entity, emitter, transform, sc, scc] : emitters.each()) {
          quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale / 2;
          desc.size = transform.scale;
          desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
          desc.colour = emitter_col;
          desc.tex_unit = sc.tex_unit;
          desc.sprite_offset_and_spritesheet = { sc.x, sc.y, sc.sx, sc.sy };
          quad_renderer::QuadRenderer::draw_sprite(desc, ri.emitters_and_occluders);
        }
      }
      // draw occluders
      {
        const auto& occluders = r.view<const LightOccluderComponent,
                                       const TransformComponent,
                                       const SpriteComponent,
                                       const SpriteColourComponent>();
        for (const auto& [entity, occluder, transform, sc, scc] : occluders.each()) {
          quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale / 2;
          desc.size = transform.scale;
          desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
          desc.colour = occluder_col;
          desc.tex_unit = sc.tex_unit;
          desc.sprite_offset_and_spritesheet = { sc.x, sc.y, sc.sx, sc.sy };
          quad_renderer::QuadRenderer::draw_sprite(desc, ri.emitters_and_occluders);
        }
      }

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.emitters_and_occluders);
    }
  }
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  // FBO: voronoi_seed.
  // Use the emitters / occluders to setup the voronoi seed texture.
  {
    Framebuffer::bind_fbo(ri.fbo_voronoi_seed);
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
        // desc.colour = background_col; // not needed
        // desc.tex_unit = // not needed
        desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
        quad_renderer::QuadRenderer::draw_sprite(desc, ri.voronoi_seed);
      }
      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.voronoi_seed);
    }
  }

  // FBO: Jump Flood Pass
  {
    const int ideal_passes = glm::ceil(glm::log(glm::max(viewport_wh.x, viewport_wh.y) / log(2.0)));
    if (ideal_passes != ri.passes)
      std::cerr << "need to handle viewport resize for JFA\n";

    for (int i = 0; i < ri.passes; i++) {

      Framebuffer::bind_fbo(ri.fbos_jump_flood[i]);
      RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
      RenderCommand::set_clear_colour_linear(background_col);
      RenderCommand::clear();
      quad_renderer::QuadRenderer::reset_quad_vert_count();
      quad_renderer::QuadRenderer::begin_batch();

      // offset for each pass is half the previous one, starting at half the square resolution rounded up to nearest
      // power 2. #i.e.for 768x512 we round up to 1024x1024 and the offset for the first pass is 512x512, then 256x256,
      // etc.
      const auto offset = glm::pow(2, ri.passes - i - 1);

      int tex_unit = ri.tex_unit_voronoi_seed;
      if (i > 0)
        tex_unit = ri.tex_units_jump_flood[i - 1];

      ri.jump_flood.bind();
      ri.jump_flood.set_int("tex", tex_unit);
      ri.jump_flood.set_vec2("screen_wh", viewport_wh);
      ri.jump_flood.set_float("u_offset", offset);

      // draw a quad
      quad_renderer::RenderDescriptor desc;
      desc.pos_tl = { 0, 0 };
      desc.size = viewport_wh;
      desc.angle_radians = 0.0f;
      // desc.colour = // not needed
      // desc.tex_unit = // not needed
      desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
      quad_renderer::QuadRenderer::draw_sprite(desc, ri.jump_flood);

      quad_renderer::QuadRenderer::end_batch();
      quad_renderer::QuadRenderer::flush(ri.jump_flood);
    } // end jumpflood pass
  }

  // FBO: voronoi distance field pass
  {
    Framebuffer::bind_fbo(ri.fbo_voronoi_distance);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    ri.voronoi_distance.bind();
    ri.voronoi_distance.set_int("tex", ri.tex_units_jump_flood[ri.passes - 1]);

    // draw a quad
    quad_renderer::RenderDescriptor desc;
    desc.pos_tl = { 0, 0 };
    desc.size = viewport_wh;
    desc.angle_radians = 0.0f;
    // desc.colour = // not needed
    // desc.tex_unit = // not needed
    desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
    quad_renderer::QuadRenderer::draw_sprite(desc, ri.voronoi_distance);

    // drawcall
    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(ri.voronoi_distance);
  }

  // FBO: 2d gi
  {
    Framebuffer::bind_fbo(ri.fbo_gi);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_linear(background_col);
    RenderCommand::clear();
    quad_renderer::QuadRenderer::reset_quad_vert_count();
    quad_renderer::QuadRenderer::begin_batch();

    ri.gi.bind();
    ri.gi.set_int("u_distance_data", ri.tex_unit_voronoi_distance);
    ri.gi.set_int("u_scene_data", ri.tex_unit_emitters_and_occluders);
    ri.gi.set_vec2("screen_wh", viewport_wh);

    // HACK
    static float time = 0.0f;
    time += dt;
    ri.gi.set_float("time", time);

    quad_renderer::RenderDescriptor desc;
    desc.pos_tl = { 0, 0 };
    desc.size = viewport_wh;
    desc.angle_radians = 0.0f;
    // desc.colour = // not needed
    // desc.tex_unit = // not needed
    desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
    quad_renderer::QuadRenderer::draw_sprite(desc, ri.gi);

    quad_renderer::QuadRenderer::end_batch();
    quad_renderer::QuadRenderer::flush(ri.gi);
  }

  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  const auto& lin_background = colours.lin_background;
  const auto& srgb_background = colours.background;

  // // FBO: Render sprites in to this fbo with linear colour
  // {
  //   Framebuffer::bind_fbo(ri.fbo_linear_main_scene);
  //   RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  //   RenderCommand::set_clear_colour_linear(*lin_background);
  //   RenderCommand::clear();
  //   {
  //     quad_renderer::QuadRenderer::reset_quad_vert_count();
  //     quad_renderer::QuadRenderer::begin_batch();

  //     // adds 0.5ms
  //     // const auto& group = registry.group<TransformComponent, SpriteComponent, SpriteColourComponent>();
  //     // sort by z-index
  //     // group.sort<SpriteComponent>([](const auto& a, const auto& b) { return a.render_order < b.render_order; });

  //     const auto& view = r.group<TransformComponent, SpriteComponent, SpriteColourComponent>();

  //     for (const auto& [entity, transform, sc, scc] : view.each()) {
  //       quad_renderer::RenderDescriptor desc;
  //       // desc.pos_tl = camera_transform.position + transform.position - transform.scale / 2;
  //       desc.pos_tl = transform.position - transform.scale / 2;
  //       desc.size = transform.scale;
  //       desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
  //       desc.colour = *scc.colour;
  //       desc.tex_unit = sc.tex_unit;
  //       desc.sprite_offset_and_spritesheet = { sc.x, sc.y, sc.sx, sc.sy };
  //       quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
  //     }

  //     quad_renderer::QuadRenderer::end_batch();
  //     quad_renderer::QuadRenderer::flush(ri.instanced);
  //   }
  // }

  // // FBO: linear_to_srgb_pass
  // {
  //   Framebuffer::bind_fbo(ri.fbo_srgb_main_scene);
  //   RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
  //   RenderCommand::set_clear_colour_linear(*lin_background);
  //   RenderCommand::clear();

  //   // Render the linear colour main scene in to this texture
  //   // and perform the linear->srgb conversion in the fragment shader.
  //   {
  //     quad_renderer::QuadRenderer::reset_quad_vert_count();
  //     quad_renderer::QuadRenderer::begin_batch();
  //     {
  //       quad_renderer::RenderDescriptor desc;
  //       desc.pos_tl = { 0, 0 };
  //       desc.size = viewport_wh;
  //       desc.angle_radians = 0.0f;
  //       // desc.colour = // not needed
  //       // desc.tex_unit = ri.tex_unit_main_FBO;
  //       desc.sprite_offset_and_spritesheet = { 0, 0, 0, 0 };
  //       quad_renderer::QuadRenderer::draw_sprite(desc, ri.linear_to_srgb);
  //     }
  //     quad_renderer::QuadRenderer::end_batch();
  //     quad_renderer::QuadRenderer::flush(ri.linear_to_srgb);
  //   }
  // }

  // Default FBO: render_texture_to_imgui
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(*srgb_background);
    RenderCommand::clear();

    // Note: ImGui::Image takes in TexID not TexUnit
    ViewportInfo vi = render_texture_to_imgui_viewport(ri.tex_id_emitters_and_occluders);

    // If the viewport moves - viewport position will be a frame behind.
    // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
    // (or if a user is moving the viewport, they likely dont need that one frame?)
    ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
    ri.viewport_size_current = { vi.size.x, vi.size.y };
    ri.viewport_hovered = vi.hovered;
    ri.viewport_focused = vi.focused;
  }

  //
  // Debug Textures
  //

  ImVec2 viewport_size;

  ImGui::Begin("DebugLighting");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_emitters_and_occluders, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();

  ImGui::Begin("DebugVoronoiSeed");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_voronoi_seed, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();

  for (int i = 0; i < ri.passes; i++) {
    std::string label = "DebugJumpFloodPass" + std::to_string(i);
    ImGui::Begin(label.c_str());
    viewport_size = ImGui::GetContentRegionAvail();
    ImGui::Image((ImTextureID)ri.tex_ids_jump_flood[i], viewport_size, ImVec2(0, 0), ImVec2(1, 1));
    ImGui::End();
  }

  ImGui::Begin("DebugVoronoiDistance");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_voronoi_distance, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();

  ImGui::Begin("Debug2DGi");
  viewport_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)ri.tex_id_gi, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
  ImGui::End();
};

void
game2d::end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};

} // namespace game2d
