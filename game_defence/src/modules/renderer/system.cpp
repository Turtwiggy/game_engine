// your header
#include "system.hpp"

// components/systems#
#include "components.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/renderpass/passes.hpp"
#include "modules/renderer/shaders/helpers.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "modules/scene/components.hpp"

using namespace engine;

// other lib
#include "imgui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <magic_enum.hpp>

namespace game2d {
using namespace std::literals;

int
get_renderer_tex_unit_count(const SINGLE_RendererInfo& ri)
{
  int i = 0;
  for (const auto& p : ri.passes)
    i += int(p.texs.size());
  return i;
};

void
rebind(entt::registry& r, SINGLE_RendererInfo& ri)
{
  fmt::println("rebind...");

  const auto& wh = ri.viewport_size_render_at;

  for (RenderPass& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      engine::bind_tex(tex.tex_id.id);
      engine::update_bound_texture_size(ri.viewport_size_render_at);
      engine::unbind_tex();
    }
  }

  int i = 0;
  for (auto& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
      glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
      i++;
    }
  }
  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
    i++;
  }

  // TBO for quadrenderer...
  int tex_buffer_unit = i++;
  glActiveTexture(GL_TEXTURE0 + tex_buffer_unit);
  glBindTexture(GL_TEXTURE_BUFFER, ri.renderer.data.TEX);
  ri.renderer.data.tex_unit = tex_buffer_unit;
  fmt::println("tbo (circles) tex_unit... {}", ri.renderer.data.tex_unit);
  fmt::println("bound textures: {}", i);

  {
    const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;
    const int tex_unit_gameicons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;
    const int tex_unit_organic2 = search_for_texture_unit_by_texture_path(ri, "organic2")->unit;
    const int tex_unit_studio_logo = search_for_texture_unit_by_texture_path(ri, "blueberry")->unit;
    const int tex_unit_text_logo = search_for_texture_unit_by_texture_path(ri, "text_logo")->unit;
    const int tex_unit_custom = search_for_texture_unit_by_texture_path(ri, "custom")->unit;
  }
  const int texs_used_by_renderer = get_renderer_tex_unit_count(ri);

  const auto get_tex_unit = [&ri](const PassName& p) -> int {
    const auto linear_pass_idx = search_for_renderpass_by_name(ri, p);
    const auto& linear_pass = ri.passes[linear_pass_idx];
    return linear_pass.texs[0].tex_unit.unit;
  };

  const int tex_unit_linear_main = get_tex_unit(PassName::linear_main);
  const int tex_unit_stars = get_tex_unit(PassName::stars);
  const int tex_unit_debris = get_tex_unit(PassName::debris);
  const int tex_unit_floor_mask = get_tex_unit(PassName::floor_mask);
  const int tex_unit_voronoi_distance = get_tex_unit(PassName::voronoi_distance);
  const int tex_unit_mix_lighting_and_scene = get_tex_unit(PassName::mix_lighting_and_scene);
  const int tex_unit_emitters_and_occluders = get_tex_unit(PassName::lighting_emitters_and_occluders);

  auto& camera = get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y, 1.0f);
  camera.projection_zoomed = camera.projection;

  ri.stars.reload();
  ri.stars.bind();
  ri.stars.set_int("tex", tex_unit_emitters_and_occluders);
  ri.stars.set_mat4("projection", camera.projection);
  ri.stars.set_vec2("viewport_wh", ri.viewport_size_render_at);

  const int tex_unit_organic2 = search_for_texture_unit_by_texture_path(ri, "organic2")->unit;
  ri.debris.reload();
  ri.debris.bind();
  ri.debris.set_int("tex", tex_unit_organic2);
  ri.debris.set_mat4("projection", camera.projection);
  ri.debris.set_vec2("viewport_wh", ri.viewport_size_render_at);

  ri.instanced.reload();
  ri.instanced.bind();
  ri.instanced.set_int("RENDERER_TEX_UNIT_COUNT", texs_used_by_renderer);

  // set user textures in ri.instanced
  const auto clean_path = [](const std::string& path) -> std::string {
    const auto last_slash = path.find_last_of("/\\");
    const auto file_name = path.substr(last_slash + 1);
    const auto last_dot = file_name.find_last_of('.');
    return (last_dot == std::string::npos) ? file_name : file_name.substr(0, last_dot);
  };
  for (const auto& tex : ri.user_textures) {
    const auto key = "tex_" + clean_path(tex.path);
    fmt::println("renderer user tex key: {}", key);
    ri.instanced.set_int(key, tex.tex_unit.unit);
  }

  ri.instanced.set_mat4("projection", camera.projection);
  ri.instanced.set_vec2("viewport_wh", ri.viewport_size_render_at);

  ri.lighting_emitters_and_occluders.reload();
  ri.lighting_emitters_and_occluders.bind();
  ri.lighting_emitters_and_occluders.set_mat4("projection", camera.projection);

  ri.voronoi_seed.reload();
  ri.voronoi_seed.bind();
  ri.voronoi_seed.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.voronoi_seed.set_mat4("projection", camera.projection);
  ri.voronoi_seed.set_int("tex", tex_unit_emitters_and_occluders);

  ri.jump_flood.reload();
  ri.jump_flood.bind();
  ri.jump_flood.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.jump_flood.set_mat4("projection", camera.projection);
  ri.jump_flood.set_vec2("screen_wh", ri.viewport_size_render_at);

  ri.voronoi_distance.reload();
  ri.voronoi_distance.bind();
  ri.voronoi_distance.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.voronoi_distance.set_mat4("projection", camera.projection);
  ri.voronoi_distance.set_int("tex_emitters_and_occluders", tex_unit_emitters_and_occluders);
  ri.voronoi_distance.set_vec2("screen_wh", ri.viewport_size_render_at);

  ri.mix_lighting_and_scene.reload();
  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_int("scene", tex_unit_linear_main);
  ri.mix_lighting_and_scene.set_bool("add_grid", false);
  ri.mix_lighting_and_scene.set_vec2("viewport_wh", ri.viewport_size_render_at);
  ri.mix_lighting_and_scene.set_int("scene_0", tex_unit_linear_main);
  ri.mix_lighting_and_scene.set_int("scene_1", tex_unit_stars);
  ri.mix_lighting_and_scene.set_int("tex_unit_debris", tex_unit_debris);
  ri.mix_lighting_and_scene.set_int("tex_unit_floor_mask", tex_unit_floor_mask);
  ri.mix_lighting_and_scene.set_int("u_distance_data", tex_unit_voronoi_distance);
  ri.mix_lighting_and_scene.set_int("circleBuffer", ri.renderer.data.tex_unit);

  // ri.blur.reload();
  // ri.blur.bind();
  // ri.blur.set_mat4("view", glm::mat4(1.0f)); // whole texture
  // ri.blur.set_mat4("projection", camera.projection);

  // ri.bloom.reload();
  // ri.bloom.bind();
  // ri.bloom.set_mat4("view", glm::mat4(1.0f)); // whole texture
  // ri.bloom.set_mat4("projection", camera.projection);
  // ri.bloom.set_int("scene_texture", tex_unit_mix_lighting_and_scene);
  // ri.bloom.set_int("blur_texture", tex_unit_blur_pingpong_1);
};

void
init_render_system(const engine::SINGLE_Application& app, entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  const glm::ivec2 screen_wh = app.window.get_size();
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  const int max_dim = glm::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const int n_jumpflood_passes = (int)(glm::ceil(glm::log(max_dim) / std::log(2.0f)));
  fmt::println("jumpflood passes... {}", n_jumpflood_passes);

  // FBO textures
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  RenderCommand::set_clear_colour_srgb({ 0.0f, 0.0f, 0.0f, 1.0f });
  RenderCommand::clear();

  ri.passes.push_back(RenderPass(PassName::stars));
  ri.passes.push_back(RenderPass(PassName::debris));
  ri.passes.push_back(RenderPass(PassName::floor_mask));
  ri.passes.push_back(RenderPass(PassName::linear_main));
  ri.passes.push_back(RenderPass(PassName::lighting_emitters_and_occluders));
  // Use the Jump flood algorithm to generate a voroi diagram,
  // then convert that in to a distance field
  ri.passes.push_back(RenderPass(PassName::voronoi_seed));
  ri.passes.push_back(RenderPass(PassName::jump_flood));
  ri.passes.push_back(RenderPass(PassName::voronoi_distance));
  ri.passes.push_back(RenderPass(PassName::mix_lighting_and_scene));
  // ri.passes.push_back(RenderPass(PassName::blur_pingpong_0));
  // ri.passes.push_back(RenderPass(PassName::blur_pingpong_1));
  // ri.passes.push_back(RenderPass(PassName::bloom));

  for (auto& rp : ri.passes) {
    if (rp.pass == PassName::jump_flood)
      rp.setup(fbo_size, 2);
    else
      rp.setup(fbo_size);
  }

  // Load user textures
  const int base_tex_unit = get_renderer_tex_unit_count(ri);
  int next_tex_unit = base_tex_unit;
  for (Texture& tex : ri.user_textures) {
    tex.tex_unit.unit = next_tex_unit;

    auto loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);

    // HACK... should do something better than this
    if (loaded_tex.path.find("organic2") != std::string::npos) {
      // loaded_tex.texture_min_filter = GL_NEAREST_MIPMAP_NEAREST;
      // loaded_tex.texture_max_filter = GL_NEAREST;
    }

    tex.tex_id.id = bind_linear_texture(loaded_tex);
    next_tex_unit++;
    fmt::println("loaded texture... {}", tex.path);
  }

  ri.stars = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/star_nest.frag");
  ri.debris = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_debris.frag");
  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.lighting_emitters_and_occluders =
    Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_emitters_and_occluders.frag");
  ri.voronoi_seed = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_seed.frag");
  ri.jump_flood = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_jump_flood.frag");
  ri.voronoi_distance = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_distance.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  // ri.blur = Shader("assets/shaders/bloom.vert", "assets/shaders/blur.frag");
  // ri.bloom = Shader("assets/shaders/bloom.vert", "assets/shaders/bloom.frag");

  // initialize renderer
#if !defined(__EMSCRIPTEN__)
  // glEnable(GL_MULTISAMPLE);
#endif

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  engine::print_gpu_info();

  // init(): create a dynamic VBO
  ri.renderer.init();

  rebind(r, ri);

  // adds the update() for each renderpass
  setup_stars_update(r);
  setup_floor_mask_update(r);
  setup_debris_update(r);
  setup_linear_main_update(r);
  setup_lighting_emitters_and_occluders_update(r);
  setup_voronoi_seed_update(r);
  setup_jump_flood_pass(r);
  setup_voronoi_distance_field_update(r);
  setup_mix_lighting_and_scene_update(r);
  // setup_gaussian_blur_update(r);
  // setup_bloom_update(r);

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

#if defined(_DEBUG)
  CHECK_OPENGL_ERROR(1337); // check a unique error code every update()
#endif

  static float time = 0.0f;
  time += dt;

  const auto& scene = get_first_component<SINGLE_CurrentScene>(r);
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  if (check_if_viewport_resize(ri)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;

#if defined(_DEBUG)
  // reload all shaders
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_0))
    rebind(r, ri);
#endif

  // debris shader
  ri.debris.bind();
  ri.debris.set_float("iTime", time);

  const auto s_stars = std::vector<Scene>{ Scene::splashscreen, Scene::menu, Scene::overworld };
  const bool in_stars_scene = std::find(s_stars.begin(), s_stars.end(), scene.s) != s_stars.end();

  const auto s_jumpflood = std::vector<Scene>{ Scene::dungeon_designer };
  const bool in_jumpflood_scene = std::find(s_jumpflood.begin(), s_jumpflood.end(), scene.s) != s_jumpflood.end();

  const auto jflood_pass = std::vector<PassName>{
    PassName::voronoi_seed,
    PassName::jump_flood,
    PassName::voronoi_distance,
    PassName::debris,
  };
  const auto in_jumpflood_pass = [&jflood_pass](const PassName& p) {
    return std::find(jflood_pass.begin(), jflood_pass.end(), p) != jflood_pass.end();
  };

  update_stars_shader(r, ri, in_stars_scene, dt);
  update_lights(r, ri);

  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_bool("put_starshader_behind", in_stars_scene);
  ri.mix_lighting_and_scene.set_bool("add_grid", get_first<Effect_GridComponent>(r) != entt::null);

#if defined(_DEBUG)
  ImGui::Begin("DebugRenderPasses");
#endif

  for (auto& pass : ri.passes) {
    const auto pass_name = std::string(magic_enum::enum_name(pass.pass));
    const auto& pass_enum = pass.pass;

#if defined(_DEBUG)
    ImGui::Text("Pass: %s", pass_name.c_str());
#endif

    // Optimisation:
    // avoid some heavy passes on scene that doesnt need them.
    // There's probably a better way to do this.

    if (pass_enum == PassName::stars && !in_stars_scene) {
#if defined(_DEBUG)
      ImGui::SameLine();
      ImGui::Text("(Skipped)");
#endif
      continue;
    }

    if (in_jumpflood_pass(pass_enum) && !in_jumpflood_scene) {
#if defined(_DEBUG)
      ImGui::SameLine();
      ImGui::Text("(Skipped)");
#endif
      continue;
    }

    const auto& wh = ri.viewport_size_render_at;
    Framebuffer::bind_fbo(pass.fbos[0]);
    RenderCommand::set_viewport(0, 0, wh.x, wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    pass.update(r);
  }

#if defined(_DEBUG)
  ImGui::End();
#endif

  // Default: render_texture_to_imgui
  // Render the last renderpass texture to the final output
  {
    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    // Note: ImGui::Image takes in TexID not TexUnit

    const PassName p = PassName::mix_lighting_and_scene;
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
    const bool hide_debug_textures = true;
    if (hide_debug_textures)
      return;

    // Debug Passes
    for (const auto& rp : ri.passes) {
      const auto pass_name = std::string(magic_enum::enum_name(rp.pass));

      for (const auto& tex : rp.texs) {
        const std::string label = fmt::format("TexUnit: {}, Tex: {}, Id: {}", tex.tex_unit.unit, pass_name, tex.tex_id.id);
        ImGui::Begin(label.c_str());
        const ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        const uint64_t id = tex.tex_id.id;
        ImGui::Image((ImTextureID)id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
        ImGui::End();
      }
    }

    // Debug user Texture
    for (int i = 0; const auto& tex : ri.user_textures) {
      const std::string label = std::string("Debug") + std::to_string(i++);
      ImGui::Begin(label.c_str());
      ImVec2 viewport_size = ImGui::GetContentRegionAvail();
      const uint64_t id = tex.tex_id.id;
      ImGui::Image((ImTextureID)id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
      ImGui::End();
    }
  }
#endif
};

void
end_frame_render_system(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  ri.renderer.end_frame();
};

} // namespace game2d
