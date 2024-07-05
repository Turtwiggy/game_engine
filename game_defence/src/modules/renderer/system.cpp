// your header
#include "system.hpp"

// components/systems#
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/renderpass/passes.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
#include <SDL_scancode.h>
using namespace engine;

// other lib
#include "imgui.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>

namespace game2d {
using namespace std::literals;

int
get_renderer_tex_unit_count(const SINGLETON_RendererInfo& ri)
{
  int i = 0;
  for (const auto& p : ri.passes)
    i += int(p.texs.size());
  return i;
};

void
rebind(entt::registry& r, const SINGLETON_RendererInfo& ri)
{
  const auto& wh = ri.viewport_size_render_at;
  engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);

  for (auto& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      engine::bind_tex(tex.tex_id.id);
      engine::update_bound_texture_size(wh);
      engine::unbind_tex();
    }
  }

  for (auto& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
      glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
    }
  }

  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
  }

  auto& camera = get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(wh.x, wh.y);

  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;
  const int tex_unit_gameicons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;
  const int tex_unit_space_background = search_for_texture_unit_by_texture_path(ri, "space_background")->unit;
  const int tex_unit_mainmenu_background = search_for_texture_unit_by_texture_path(ri, "background_mainmenu")->unit;
  const int tex_unit_spacestation_0 = search_for_texture_unit_by_texture_path(ri, "spacestation_0")->unit;
  const int tex_unit_studio_logo = search_for_texture_unit_by_texture_path(ri, "blueberry")->unit;

  ri.instanced.bind();
  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_int("RENDERER_TEX_UNIT_COUNT", get_renderer_tex_unit_count(ri));
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);
  ri.instanced.set_int("tex_gameicons", tex_unit_gameicons);
  ri.instanced.set_int("tex_unit_space_background", tex_unit_space_background);
  ri.instanced.set_int("tex_unit_mainmenu_background", tex_unit_mainmenu_background);
  ri.instanced.set_int("tex_unit_spacestation_0", tex_unit_spacestation_0);
  ri.instanced.set_int("tex_unit_studio_logo", tex_unit_studio_logo);

  const auto linear_pass_idx = search_for_renderpass_by_name(ri, PassName::linear_main);
  const auto& linear_pass = ri.passes[linear_pass_idx];
  const int tex_unit_linear_main = linear_pass.texs[0].tex_unit.unit;

  const auto lighting_and_scene_idx = search_for_renderpass_by_name(ri, PassName::mix_lighting_and_scene);
  const auto& lighting_and_scene_pass = ri.passes[lighting_and_scene_idx];
  const int tex_unit_mix_lighting_and_scene = lighting_and_scene_pass.texs[0].tex_unit.unit;

  const auto blur_pingpong_1_idx = search_for_renderpass_by_name(ri, PassName::blur_pingpong_1);
  const auto& blur_pingpong_1_pass = ri.passes[blur_pingpong_1_idx];
  const int tex_unit_blur_pingpong_1 = blur_pingpong_1_pass.texs[0].tex_unit.unit;

  const auto lighting_AO_pass_idx = search_for_renderpass_by_name(ri, PassName::lighting_ambient_occlusion);
  const auto& lighting_AO_pass = ri.passes[lighting_AO_pass_idx];
  const int tex_unit_lighting_AO = lighting_AO_pass.texs[0].tex_unit.unit;

  ri.lighting_emitters_and_occluders.bind();
  ri.lighting_emitters_and_occluders.set_mat4("projection", camera.projection);
  ri.lighting_emitters_and_occluders.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.lighting_emitters_and_occluders.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  ri.lighting_ambient_occlusion.bind();
  ri.lighting_ambient_occlusion.set_mat4("projection", camera.projection);
  ri.lighting_ambient_occlusion.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.lighting_ambient_occlusion.set_vec4("colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.mix_lighting_and_scene.set_int("lighting", tex_unit_lighting_AO);
  ri.mix_lighting_and_scene.set_int("scene", tex_unit_linear_main);

  ri.circle.bind();
  ri.circle.set_mat4("projection", camera.projection);

  ri.blur.bind();
  ri.blur.set_mat4("projection", camera.projection);
  ri.blur.set_mat4("view", glm::mat4(1.0f)); // whole texture

  ri.bloom.bind();
  ri.bloom.set_mat4("projection", camera.projection);
  ri.bloom.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.bloom.set_int("scene_texture", tex_unit_mix_lighting_and_scene);
  ri.bloom.set_int("blur_texture", tex_unit_blur_pingpong_1);

  ri.grid.bind();
  ri.grid.set_mat4("projection", camera.projection);
};

void
init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const glm::ivec2 screen_wh = { app.width, app.height };
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  // FBO textures
  Framebuffer::default_fbo();
  ri.passes.push_back(RenderPass(PassName::linear_main));
  ri.passes.push_back(RenderPass(PassName::lighting_emitters_and_occluders));
  ri.passes.push_back(RenderPass(PassName::lighting_ambient_occlusion));
  ri.passes.push_back(RenderPass(PassName::mix_lighting_and_scene, 2));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_0));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_1));
  ri.passes.push_back(RenderPass(PassName::bloom));
  for (auto& rp : ri.passes)
    rp.setup(fbo_size);

  // Load user textures
  const int base_tex_unit = get_renderer_tex_unit_count(ri);
  int next_tex_unit = base_tex_unit;
  for (Texture& tex : ri.user_textures) {
    tex.tex_unit.unit = next_tex_unit;
    const auto loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);
    tex.tex_id.id = bind_linear_texture(loaded_tex);
    next_tex_unit++;
  }

  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.lighting_emitters_and_occluders = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.lighting_ambient_occlusion = Shader("assets/shaders/2d_basic_with_proj.vert", "assets/shaders/2d_colour.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  ri.circle = Shader("assets/shaders/2d_circle.vert", "assets/shaders/2d_circle.frag");
  ri.grid = Shader("assets/shaders/2d_grid.vert", "assets/shaders/2d_grid.frag");
  ri.blur = Shader("assets/shaders/bloom.vert", "assets/shaders/blur.frag");
  ri.bloom = Shader("assets/shaders/bloom.vert", "assets/shaders/bloom.frag");

  // initialize renderer
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
  glEnable(GL_MULTISAMPLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  engine::print_gpu_info();
  engine::quad_renderer::QuadRenderer::init();

  rebind(r, ri);

  // adds the update() for each renderpass
  setup_linear_main_update(r);
  // setup_lighting_main_update(r);
  setup_lighting_emitters_and_occluders_update(r);
  setup_lighting_ambient_occlusion_update(r);
  setup_mix_lighting_and_scene_update(r);
  setup_gaussian_blur_update(r);
  setup_bloom_update(r);

  // validate that update() been set...
  for (const auto& pass : ri.passes) {
    const auto type_name = std::string(magic_enum::enum_name(pass.pass));
    if (!pass.update) {
      fmt::println(stderr, "ERROR! RenderPass Update() not set for {}", type_name);
      exit(1); // explode
    }
    // fmt::println("RenderPass {} tex_size: {}", type_name, pass.texs.size());
    // for (const auto& tex : pass.texs)
    //   fmt::println("Unit: {}, Id: {}", tex.tex_unit.unit, tex.tex_id.id);
  }
  // for (auto& tex : ri.user_textures)
  //   fmt::println("User Texture, Unit: {}, Id: {}", tex.tex_unit.unit, tex.tex_id.id);

  CHECK_OPENGL_ERROR(3);
};

void
update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos)
{
  static const engine::SRGBColour black(0, 0, 0, 1.0f);

  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  if (check_if_viewport_resize(ri)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;

  ImGui::Begin("DebugRenderPasses");

  for (const auto& pass : ri.passes) {
    const auto pass_name = std::string(magic_enum::enum_name(pass.pass));
    ImGui::Text("Pass: %s", pass_name.c_str());

    Framebuffer::bind_fbo(pass.fbo);
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    pass.update();
  }

  ImGui::End();

  // Default: render_texture_to_imgui
  // Render the last renderpass texture to the final output
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    // Note: ImGui::Image takes in TexID not TexUnit
    const auto pass_idx = search_for_renderpass_by_name(ri, PassName::bloom);
    const auto& pass = ri.passes[pass_idx];
    const auto vi = render_texture_to_imgui_viewport(pass.texs[0].tex_id.id);

    // If the viewport moves - viewport position will be a frame behind.
    // This would mainly affect an editor, a game viewport probably(?) wouldn't move that much
    // (or if a user is moving the viewport, they likely dont need that one frame?)
    ri.viewport_pos = glm::vec2(vi.pos.x, vi.pos.y);
    ri.viewport_size_current = { vi.size.x, vi.size.y };
    ri.viewport_hovered = vi.hovered;
    ri.viewport_focused = vi.focused;
  }

#if defined(_DEBUG)
  {
    const bool hide_debug_textures = false;
    if (hide_debug_textures)
      return;

    // Debug Passes
    for (const auto& rp : ri.passes) {
      const auto pass_name = std::string(magic_enum::enum_name(rp.pass));

      for (int i = 0; const auto& tex : rp.texs) {
        const std::string label = "DebugTexture"s + pass_name + std::to_string(i++);
        ImGui::Begin(label.c_str());
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)tex.tex_id.id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
        ImGui::End();
      }
    }

    // Debug user Texture
    // for (int i = 0; const auto& tex : ri.user_textures) {
    //   const std::string label = std::string("Debug") + std::to_string(i++);
    //   ImGui::Begin(label.c_str());
    //   ImVec2 viewport_size = ImGui::GetContentRegionAvail();
    //   ImGui::Image((ImTextureID)tex.tex_id.id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
    //   ImGui::End();
    // }
  }
#endif
};

void
end_frame_render_system(entt::registry& registry)
{
  quad_renderer::QuadRenderer::end_frame();
};

} // namespace game2d
