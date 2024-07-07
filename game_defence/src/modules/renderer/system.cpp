// your header
#include "system.hpp"

// components/systems#
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/renderpass/passes.hpp"

// engine headers
#include "modules/scene/components.hpp"
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
rebind(entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const auto& wh = ri.viewport_size_render_at;
  engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);

  for (RenderPass& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      const glm::ivec2 scaled_size = { wh.x * rp.texture_scale_factor, wh.y * rp.texture_scale_factor };
      engine::bind_tex(tex.tex_id.id);
      engine::update_bound_texture_size(scaled_size);
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
  const int tex_unit_spacestation_0 = search_for_texture_unit_by_texture_path(ri, "spacestation_0")->unit;
  const int tex_unit_studio_logo = search_for_texture_unit_by_texture_path(ri, "blueberry")->unit;

  // ri.instanced.reload();
  ri.instanced.bind();
  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_int("RENDERER_TEX_UNIT_COUNT", get_renderer_tex_unit_count(ri));
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);
  ri.instanced.set_int("tex_gameicons", tex_unit_gameicons);
  ri.instanced.set_int("tex_unit_spacestation_0", tex_unit_spacestation_0);
  ri.instanced.set_int("tex_unit_studio_logo", tex_unit_studio_logo);

  const auto get_tex_unit = [&ri](const PassName& p) -> int {
    const auto linear_pass_idx = search_for_renderpass_by_name(ri, p);
    const auto& linear_pass = ri.passes[linear_pass_idx];
    return linear_pass.texs[0].tex_unit.unit;
  };
  const int tex_unit_linear_main = get_tex_unit(PassName::linear_main);
  const int tex_unit_mix_lighting_and_scene = get_tex_unit(PassName::mix_lighting_and_scene);
  const int tex_unit_blur_pingpong_1 = get_tex_unit(PassName::blur_pingpong_1);
  const int tex_unit_lighting_AO = get_tex_unit(PassName::lighting_ambient_occlusion);
  const int tex_unit_emitters_and_occluders = get_tex_unit(PassName::lighting_emitters_and_occluders);

  ri.stars.reload();
  ri.stars.bind();
  ri.stars.set_mat4("projection", camera.projection);
  ri.stars.set_int("tex", tex_unit_emitters_and_occluders);

  // ri.lighting_emitters_and_occluders.reload();
  ri.lighting_emitters_and_occluders.bind();
  ri.lighting_emitters_and_occluders.set_mat4("projection", camera.projection);

  // ri.lighting_ambient_occlusion.reload();
  ri.lighting_ambient_occlusion.bind();
  ri.lighting_ambient_occlusion.set_mat4("projection", camera.projection);
  ri.lighting_ambient_occlusion.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.lighting_ambient_occlusion.set_int("tex", tex_unit_emitters_and_occluders);
  ri.lighting_ambient_occlusion.set_vec2("viewport_wh", ri.viewport_size_render_at);

  // ri.mix_lighting_and_scene.reload();
  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.mix_lighting_and_scene.set_int("lighting", tex_unit_lighting_AO);
  ri.mix_lighting_and_scene.set_int("scene", tex_unit_linear_main);

  // ri.circle.reload();
  ri.circle.bind();
  ri.circle.set_mat4("projection", camera.projection);

  // ri.blur.reload();
  ri.blur.bind();
  ri.blur.set_mat4("projection", camera.projection);
  ri.blur.set_mat4("view", glm::mat4(1.0f)); // whole texture

  // ri.bloom.reload();
  ri.bloom.bind();
  ri.bloom.set_mat4("projection", camera.projection);
  ri.bloom.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.bloom.set_int("scene_texture", tex_unit_mix_lighting_and_scene);
  ri.bloom.set_int("blur_texture", tex_unit_blur_pingpong_1);

  // ri.grid.reload();
  ri.grid.bind();
  ri.grid.set_mat4("projection", camera.projection);
};

void
init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const glm::ivec2 screen_wh = app.window.get_size();

  // add camera
  OrthographicCamera camera_info;
  camera_info.projection = calculate_ortho_projection(screen_wh.x, screen_wh.y);
  const auto camera_e = create_empty<OrthographicCamera>(r, camera_info);
  r.emplace<TransformComponent>(camera_e);

  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  // FBO textures
  Framebuffer::default_fbo();
  ri.passes.push_back(RenderPass(PassName::stars));
  ri.passes.push_back(RenderPass(PassName::linear_main));
  ri.passes.push_back(RenderPass(PassName::lighting_emitters_and_occluders));
  ri.passes.push_back(RenderPass(PassName::lighting_ambient_occlusion));
  ri.passes.push_back(RenderPass(PassName::mix_lighting_and_scene, 2));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_0));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_1));
  ri.passes.push_back(RenderPass(PassName::bloom));
  for (auto& rp : ri.passes) {
    if (rp.pass == PassName::stars) {
      rp.texture_scale_factor = 1.0f;
      const glm::ivec2 scaled_size = { screen_wh.x * rp.texture_scale_factor, screen_wh.y * rp.texture_scale_factor };
      rp.setup(scaled_size);
      continue;
    }
    rp.setup(fbo_size);
  }

  // Load user textures
  const int base_tex_unit = get_renderer_tex_unit_count(ri);
  int next_tex_unit = base_tex_unit;
  for (Texture& tex : ri.user_textures) {
    tex.tex_unit.unit = next_tex_unit;
    const auto loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);
    tex.tex_id.id = bind_linear_texture(loaded_tex);
    next_tex_unit++;
  }

  ri.stars = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/star_nest.frag");
  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.lighting_emitters_and_occluders =
    Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_emitters_and_occluders.frag");
  ri.lighting_ambient_occlusion = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_ambient_occlusion.frag");
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
  setup_stars_update(r);
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

  static float time = 0.0f;
  time += dt;

  const auto& s = get_first_component<SINGLETON_CurrentScene>(r);
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  if (check_if_viewport_resize(ri)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;

  // reload all shaders
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_0))
    rebind(r, ri);

  // Do things with the star shader
  {

    ri.stars.bind();
    static glm::vec3 current_pos{ 0, 0, 0 };
    static glm::vec3 target_pos{ 0, 0, 0 };
    static glm::vec2 offset{ 0, 0 };
    static bool generate_offset = true;
    static bool has_player = false;

    if (generate_offset) {
      generate_offset = false;
      static engine::RandomState rnd;
      offset.x = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);
      offset.y = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);

      target_pos.x = offset.x;
      target_pos.y = offset.y;
    }

    const auto& player_e = get_first<PlayerComponent>(r);
    if (player_e != entt::null && s.s == Scene::overworld) {
      has_player = true;

      // set the star shader to the player position, plus an offset
      const auto pos = get_position(r, player_e);
      target_pos.x = pos.x + offset.x;
      target_pos.y = pos.y + offset.y;
    } else {
      if (has_player) {
        has_player = false;
        generate_offset = true;
      }

      // scroll the starfield down and right
      const glm::vec2 dir{ 1.0f, 1.0f };
      target_pos.x += dir.x * dt * 10.0f;
      target_pos.y += dir.y * dt * 10.0f;
    }

    const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
      return b + (a - b) * glm::exp(-decay * dt);
    };
    current_pos.x = exp_decay(current_pos.x, target_pos.x, 32, dt);
    current_pos.y = exp_decay(current_pos.y, target_pos.y, 32, dt);
    // current_pos.y = exp_decay(current_pos.z, target_pos.z, 32, dt);

    ri.stars.set_vec2("player_position", { current_pos.x, current_pos.y });

    const bool in_overworld = s.s == Scene::overworld;
    const bool in_splashscreen = s.s == Scene::splashscreen;
    const bool put_starshader_behind = in_overworld || in_splashscreen;
    ri.mix_lighting_and_scene.bind();
    ri.mix_lighting_and_scene.set_bool("put_starshader_behind", put_starshader_behind);
  }

  ImGui::Begin("DebugRenderPasses");

  for (auto& pass : ri.passes) {
    const auto pass_name = std::string(magic_enum::enum_name(pass.pass));

    const auto& wh = ri.viewport_size_render_at;
    const auto& rp = pass;
    const glm::ivec2 scaled = { wh.x * rp.texture_scale_factor, wh.y * rp.texture_scale_factor };

    Framebuffer::bind_fbo(pass.fbo);
    RenderCommand::set_viewport(0, 0, scaled.x, scaled.y);
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

    PassName p = PassName::bloom;
    if (s.s == Scene::menu)
      p = PassName::stars;
    const auto& pass = ri.passes[search_for_renderpass_by_name(ri, p)];
    const auto tex_id = pass.texs[0].tex_id.id;

    const auto vi = render_texture_to_imgui_viewport(tex_id);

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
