// your header
#include "system.hpp"

// components/systems#
#include "actors/helpers.hpp"
#include "colour/colour.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "imgui/helpers.hpp"
#include "lights/helpers.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gen_dungeons/components.hpp"
#include "modules/gen_dungeons/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/lights/components.hpp"
#include "modules/renderer/renderpass/passes.hpp"
#include "modules/scene/components.hpp"
#include "modules/vfx_grid/components.hpp"

// engine headers
#include "opengl/framebuffer.hpp"
#include "opengl/render_command.hpp"
#include "opengl/shader.hpp"
#include "opengl/texture.hpp"
#include "opengl/util.hpp"
#include "renderer/transform.hpp"
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
  fmt::println("bound textures: {}", i);

  const int tex_unit_kenny = search_for_texture_unit_by_texture_path(ri, "monochrome")->unit;
  const int tex_unit_gameicons = search_for_texture_unit_by_texture_path(ri, "gameicons")->unit;
  const int tex_unit_spacestation_0 = search_for_texture_unit_by_texture_path(ri, "spacestation_0")->unit;
  const int tex_unit_studio_logo = search_for_texture_unit_by_texture_path(ri, "blueberry")->unit;
  const int tex_unit_custom = search_for_texture_unit_by_texture_path(ri, "custom")->unit;
  const int texs_used_by_renderer = get_renderer_tex_unit_count(ri);

  const auto get_tex_unit = [&ri](const PassName& p) -> int {
    const auto linear_pass_idx = search_for_renderpass_by_name(ri, p);
    const auto& linear_pass = ri.passes[linear_pass_idx];
    return linear_pass.texs[0].tex_unit.unit;
  };
  const int tex_unit_linear_main = get_tex_unit(PassName::linear_main);
  const int tex_unit_mix_lighting_and_scene = get_tex_unit(PassName::mix_lighting_and_scene);
  const int tex_unit_blur_pingpong_1 = get_tex_unit(PassName::blur_pingpong_1);
  const int tex_unit_emitters_and_occluders = get_tex_unit(PassName::lighting_emitters_and_occluders);

  ri.stars.reload();
  ri.stars.bind();
  ri.stars.set_int("tex", tex_unit_emitters_and_occluders);

  ri.instanced.reload();
  ri.instanced.bind();
  ri.instanced.set_int("RENDERER_TEX_UNIT_COUNT", texs_used_by_renderer);
  ri.instanced.set_int("tex_kenny", tex_unit_kenny);
  ri.instanced.set_int("tex_gameicons", tex_unit_gameicons);
  ri.instanced.set_int("tex_unit_spacestation_0", tex_unit_spacestation_0);
  ri.instanced.set_int("tex_unit_studio_logo", tex_unit_studio_logo);
  ri.instanced.set_int("tex_unit_custom", tex_unit_custom);

  ri.lighting_emitters_and_occluders.reload();
  ri.lighting_emitters_and_occluders.bind();

  ri.voronoi_seed.reload();
  ri.voronoi_seed.bind();
  ri.voronoi_seed.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.voronoi_seed.set_int("tex", tex_unit_emitters_and_occluders);

  ri.jump_flood.reload();
  ri.jump_flood.bind();
  ri.jump_flood.set_mat4("view", glm::mat4(1.0f)); // whole texture

  ri.voronoi_distance.reload();
  ri.voronoi_distance.bind();
  ri.voronoi_distance.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.voronoi_distance.set_int("tex_emitters_and_occluders", tex_unit_emitters_and_occluders);
  ri.voronoi_distance.set_vec2("screen_wh", ri.viewport_size_render_at);

  ri.mix_lighting_and_scene.reload();
  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.mix_lighting_and_scene.set_int("scene", tex_unit_linear_main);
  ri.mix_lighting_and_scene.set_bool("add_grid", false);

  ri.circle.reload();
  ri.circle.bind();

  ri.blur.reload();
  ri.blur.bind();
  ri.blur.set_mat4("view", glm::mat4(1.0f)); // whole texture

  // ri.bloom.reload();
  ri.bloom.bind();
  ri.bloom.set_mat4("view", glm::mat4(1.0f)); // whole texture
  ri.bloom.set_int("scene_texture", tex_unit_mix_lighting_and_scene);
  ri.bloom.set_int("blur_texture", tex_unit_blur_pingpong_1);
};

void
init_render_system(const engine::SINGLETON_Application& app, entt::registry& r, SINGLETON_RendererInfo& ri)
{
  const glm::ivec2 screen_wh = app.window.get_size();
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  // calculate view after updating postiion
  const auto camera_e = create_empty<OrthographicCamera>(r);
  r.emplace<TransformComponent>(camera_e);

  const int max_dim = glm::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const int n_jumpflood_passes = (int)(glm::ceil(glm::log(max_dim) / std::log(2.0f)));
  fmt::println("jumpflood passes... {}", n_jumpflood_passes);

  // FBO textures
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  RenderCommand::set_clear_colour_srgb({ 0.0f, 0.0f, 0.0f, 1.0f });
  RenderCommand::clear();

  ri.passes.push_back(RenderPass(PassName::stars));
  ri.passes.push_back(RenderPass(PassName::linear_main));
  ri.passes.push_back(RenderPass(PassName::lighting_emitters_and_occluders));
  // Use the Jump flood algorithm to generate a voroi diagram,
  // then convert that in to a distance field
  ri.passes.push_back(RenderPass(PassName::voronoi_seed));
  ri.passes.push_back(RenderPass(PassName::jump_flood));
  ri.passes.push_back(RenderPass(PassName::voronoi_distance));
  ri.passes.push_back(RenderPass(PassName::mix_lighting_and_scene));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_0));
  ri.passes.push_back(RenderPass(PassName::blur_pingpong_1));
  ri.passes.push_back(RenderPass(PassName::bloom));

  for (auto& rp : ri.passes) {
    if (rp.pass == PassName::jump_flood)
      rp.setup(fbo_size / 100, n_jumpflood_passes);
    else
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
    fmt::println("loaded texture... {}", tex.path);
  }

  ri.stars = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/star_nest.frag");
  ri.instanced = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.lighting_emitters_and_occluders =
    Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_emitters_and_occluders.frag");
  ri.voronoi_seed = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_seed.frag");
  ri.jump_flood = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_jump_flood.frag");
  ri.voronoi_distance = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_distance.frag");
  ri.mix_lighting_and_scene = Shader("assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  ri.circle = Shader("assets/shaders/2d_circle.vert", "assets/shaders/2d_circle.frag");
  ri.blur = Shader("assets/shaders/bloom.vert", "assets/shaders/blur.frag");
  ri.bloom = Shader("assets/shaders/bloom.vert", "assets/shaders/bloom.frag");

  // initialize renderer
#if !defined(__EMSCRIPTEN__)
  // glEnable(GL_MULTISAMPLE);
#endif

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  engine::print_gpu_info();
  engine::quad_renderer::QuadRenderer::init();

  rebind(r, ri);

  // adds the update() for each renderpass

  setup_stars_update(r);
  setup_linear_main_update(r);
  setup_lighting_emitters_and_occluders_update(r);
  setup_voronoi_seed_update(r);
  setup_jump_flood_pass(r);
  setup_voronoi_distance_field_update(r);
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

  const auto& scene = get_first_component<SINGLETON_CurrentScene>(r);
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  if (check_if_viewport_resize(ri)) {
    ri.viewport_size_render_at = ri.viewport_size_current;
    rebind(r, ri);
  }
  const auto viewport_wh = ri.viewport_size_render_at;

#if defined(_DEBUG)
  // reload all shaders
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_0))
    rebind(r, ri);
#endif

  auto& camera = get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(viewport_wh.x, viewport_wh.y, dt);

  const auto scenes_to_render_stars = std::vector<Scene>{
    Scene::splashscreen,
    Scene::menu,
    Scene::overworld_revamped,
  };
  const bool in_stars_scene =
    std::find(scenes_to_render_stars.begin(), scenes_to_render_stars.end(), scene.s) != scenes_to_render_stars.end();

  const auto scenes_to_jumpflood = std::vector<Scene>{ Scene::dungeon_designer };
  const bool in_jumpflood_scene =
    std::find(scenes_to_jumpflood.begin(), scenes_to_jumpflood.end(), scene.s) != scenes_to_jumpflood.end();

  const std::vector<PassName> jumpflood_passes{ PassName::voronoi_seed, PassName::jump_flood, PassName::voronoi_distance };
  const auto in_jumpflood_pass = [jumpflood_passes](const PassName& pass) {
    return std::find(jumpflood_passes.begin(), jumpflood_passes.end(), pass) != jumpflood_passes.end();
  };

  // Do things with the star shader
  {
    ri.stars.bind();
    static glm::vec3 current_pos{ 0, 0, 0 };
    static glm::vec3 target_pos{ 0, 0, 0 };
    static glm::vec2 offset{ 0, 0 };
    const auto camera_e = get_first<OrthographicCamera>(r);
    const bool has_player = camera_e != entt::null;

    // generate an offset as (0, 0) in the fractal looks weird
    static bool generate_offset = true;
    if (generate_offset) {
      generate_offset = false;
      static engine::RandomState rnd;
      offset.x = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);
      offset.y = engine::rand_det_s(rnd.rng, -5000.0f, 5000.0f);
      target_pos.x = offset.x;
      target_pos.y = offset.y;
    }

    // starfield to follow player position
    if (has_player && in_stars_scene) {
      const auto pos = get_position(r, camera_e);
      target_pos.x = pos.x + offset.x;
      target_pos.y = pos.y + offset.y;
    }

    const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
      return b + (a - b) * glm::exp(-decay * dt);
    };
    current_pos.x = exp_decay(current_pos.x, target_pos.x, 32, dt);
    current_pos.y = exp_decay(current_pos.y, target_pos.y, 32, dt);
    // current_pos.y = exp_decay(current_pos.z, target_pos.z, 32, dt);

    ri.stars.set_vec2("player_position", { current_pos.x, current_pos.y });
  }

  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_bool("put_starshader_behind", in_stars_scene);
  ri.mix_lighting_and_scene.set_float("iTime", time);
  ri.mix_lighting_and_scene.set_bool("add_grid", get_first<Effect_GridComponent>(r) != entt::null);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_t = r.get<TransformComponent>(camera_e);

  const int max_lights = 32;
  static std::vector<Light> lights(max_lights);

  // disable lights every frame?
  for (auto& l : lights) {
    l.enabled = false;
    l.luminence = 0.0f;
  }

  // update the first light position to the first player position.
  const auto& first_player = get_first<PlayerComponent>(r);
  if (first_player != entt::null) {
    glm::vec2 hmm = get_position(r, first_player);

    // worldspace to screenspace
    const auto& wh = ri.viewport_size_render_at;
    hmm -= glm::vec2{ camera_t.position.x, camera_t.position.y };
    hmm += glm::vec2{ wh.x / 2.0f, wh.y / 2.0f };

    // player light; blueish?
    lights[0].pos = hmm;
    lights[0].enabled = true;
    lights[0].colour = { 0.5f, 0.75f, 1.0f, 1.0f };
    lights[0].luminence = 0.6f;
  }

  // HACK: try adding lights to interesting map features
  const auto& map_e = get_first<MapComponent>(r);
  const auto& results_e = get_first<DungeonGenerationResults>(r);
  {
    if (map_e != entt::null && first_player != entt::null) {
      const auto& map = r.get<MapComponent>(map_e);
      const auto& results = r.get<DungeonGenerationResults>(results_e);

      // if player is in the room, light it up
      const auto player_pos = get_position(r, first_player);
      const auto player_gridpos = engine::grid::worldspace_to_grid_space(player_pos, map.tilesize);
      const auto [in_room, room] = inside_room(map, results.rooms, player_gridpos);
      const auto tunnels = inside_tunnels(results.tunnels, player_gridpos);
      const bool in_tunnel = tunnels.size() > 0;
      const bool inside_spaceship = in_room || in_tunnel;

      ri.mix_lighting_and_scene.bind();
      ri.mix_lighting_and_scene.set_bool("inside_spaceship", inside_spaceship);

      // increase player brightness outside spaceship.
      if (!inside_spaceship)
        lights[0].luminence = 1.5f;

      // increase player brightness in tunnel (no room lights)
      if (in_tunnel && !in_room)
        lights[0].luminence = 1.25f;

      if (in_room) {
        for (int i = 1; const auto& [e, room_c] : r.view<Room>().each()) {
          if (room->tl != room_c.tl)
            continue;
          light_up_room(lights, i, room_c, ri, camera_t);
        }
      }
    }
  }

  ri.mix_lighting_and_scene.bind();

  // update lighting uniforms

  for (size_t i = 0; i < lights.size(); i++) {
    const std::string label = "lights["s + std::to_string(i) + "]."s;

    const auto& l = lights[i];
    ri.mix_lighting_and_scene.set_bool(label + "enabled"s, l.enabled);
    if (!l.enabled)
      continue;
    ri.mix_lighting_and_scene.set_vec2(label + "position"s, l.pos);
    ri.mix_lighting_and_scene.set_vec4(label + "colour"s,
                                       { l.colour.r / 255.0f, l.colour.g / 255.0f, l.colour.b / 255.0f, l.colour.a });
    ri.mix_lighting_and_scene.set_float(label + "luminance"s, l.luminence);

    if (i >= max_lights)
      break;
  }

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

    pass.update();
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
        const ImTextureID id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex.tex_id.id));
        ImGui::Image(id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
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
