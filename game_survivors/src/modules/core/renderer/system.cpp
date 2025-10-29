#include "pch.hpp"

#include "system.hpp"

// components/systems
#include "components.hpp"
#include "engine/app/application.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/renderer/helpers/batch_quad.hpp"
#include "modules/core/renderer/lights/components.hpp"
#include "modules/core/renderer/renderpass/passes.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "resources/data.hpp"

// engine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/shader.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/opengl/util.hpp"
#include "modules/scene/scene_components.hpp"
#include "modules/systems/system_screenshake/components.hpp"
#include "renderpass/passes.hpp"
#include <tracy/Tracy.hpp>

using namespace engine;

namespace game2d {
using namespace std::literals;

struct UboData
{
  // glm::mat4 projection = glm::mat4(1.0f);
  glm::mat4 projection_zoomed = glm::mat4(1.0f);
  glm::mat4 view = glm::mat4(1.0f);
  glm::vec2 camera_pos{ 0, 0 };
  glm::vec2 screenshake{ 0, 0 };
  glm::vec4 light_positions[32];
  float time = 0;
  float zoom = 0;
  float tilesize = default_map_tilesize;
};
static UboData data;

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
  SDL_Log("%s", std::format("rebind...").c_str());

  // Super sampling, innit
  const auto& wh = ri.viewport_size_render_at;
  const auto double_wh = glm::vec2{ 2.0f * wh.x, 2.0f * wh.y };

  for (RenderPass& rp : ri.passes) {
    for (const auto& tex : rp.texs) {
      engine::bind_tex(tex.tex_id.id);
      engine::update_bound_texture_size(double_wh);
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

  // rebind the fluidsim textures.
  // rebind_fluidsim(r, ri.fluid_sim);

  for (const auto& tex : ri.user_textures) {
    glActiveTexture(GL_TEXTURE0 + tex.tex_unit.unit);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id.id);
    i++;
  }

  SDL_Log("%s", std::format("bound textures: {}", i).c_str());
  const int texs_used = get_renderer_tex_unit_count(ri);
  //  + get_texs_used_by_fluidsim();
  const auto get_tex_unit = [&ri](const PassName& p) -> int {
    const auto idx = get_pass_idx(ri, p);
    const auto& pass = ri.passes[idx];
    return pass.texs[0].tex_unit.unit;
  };

  // const int tex_unit_menu_fractal = get_tex_unit(PassName::menu_fractal_shader);
  const int tex_unit_linear_main = get_tex_unit(PassName::linear_main);
  const int tex_unit_water = get_tex_unit(PassName::water);
  const int tex_unit_sprites_to_outline = get_tex_unit(PassName::sprites_to_outline);
  const int tex_unit_outline = get_tex_unit(PassName::outline);
  const int tex_unit_sprites_with_shield = get_tex_unit(PassName::sprites_with_shield);
  const int tex_unit_shine_shells = get_tex_unit(PassName::shine);
  const int tex_unit_flame = get_tex_unit(PassName::flame);
  const int tex_unit_island_triangles = get_tex_unit(PassName::island_triangles);
  const int tex_unit_island_triangles_gradient = get_tex_unit(PassName::island_triangles_gradient);
  const int tex_unit_island_hidden = get_tex_unit(PassName::island_hidden);
  const int tex_unit_island_above_hidden = get_tex_unit(PassName::island_above_hidden);
  const int tex_unit_island_shore = get_tex_unit(PassName::island_shore);
  // const int tex_unit_fluid = get_tex_unit(PassName::fluid_sim);
  // const int tex_unit_voronoi_distance = get_tex_unit(PassName::voronoi_distance);
  const int tex_unit_mix_lighting_and_scene = get_tex_unit(PassName::mix_lighting_and_scene);
  // const int tex_unit_emitters_and_occluders = get_tex_unit(PassName::lighting_emitters_and_occluders);

  auto& camera = get_first_component<OrthographicCamera>(r);
  camera.projection = calculate_ortho_projection(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y, 1.0f);
  camera.projection_zoomed = camera.projection;

  // store projection ONCE in the UBO
  // glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  // glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
  // glBindBuffer(GL_UNIFORM_BUFFER, 0);

  ri.water.reload(r);
  ri.water.bind();
  ri.water.set_uniform_block_binding("Data", 0);
  ri.water.set_mat4("projection", camera.projection);
  ri.water.set_vec2("viewport_wh", wh);
  ri.water.set_float("water_safe_radius", 10'000); // basically no danger zone

  // set user textures in shaders
  const auto clean_path = [](const std::string& path) -> std::string {
    const auto last_slash = path.find_last_of("/\\");
    const auto file_name = path.substr(last_slash + 1);
    const auto last_dot = file_name.find_last_of('.');
    return file_name.substr(0, last_dot);
  };

  ri.instanced.reload(r);
  ri.instanced.bind();
  ri.instanced.set_uniform_block_binding("Data", 0);
  ri.instanced.set_int("RENDERER_TEX_UNIT_COUNT", texs_used);
  ri.instanced.set_bool("do_zoom", true);
  ri.instanced.set_mat4("projection", camera.projection);
  for (int i = 0; i < (int)ri.user_textures.size(); i++) {
    const auto& tex = ri.user_textures[i];
    ri.instanced.set_int("u_textures[" + std::to_string(i) + "]", tex.tex_unit.unit);
  }
  // ri.instanced.set_int("tex_fluid", tex_unit_fluid);
  // ri.instanced.set_int("tex_fluid_tex_unit", tex_unit_fluid);
  // ri.instanced.set_float("tex_fluid_texel_size", 1.0f / ri.fluis_sim.config_dye_resolution);

  ri.instanced_tri.reload(r);
  ri.instanced_tri.bind();
  ri.instanced_tri.set_uniform_block_binding("Data", 0);
  ri.instanced_tri.set_bool("do_zoom", true);
  ri.instanced_tri.set_mat4("projection", camera.projection);

  ri.island_tri_gradient.reload(r);
  ri.island_tri_gradient.bind();
  ri.island_tri_gradient.set_uniform_block_binding("Data", 0);
  ri.island_tri_gradient.set_mat4("projection", camera.projection);
  ri.island_tri_gradient.set_bool("is_fullscreen", true);
  ri.island_tri_gradient.set_bool("do_zoom", false);
  ri.island_tri_gradient.set_int("tex_island_triangles", tex_unit_island_triangles);
  ri.island_tri_gradient.set_vec2("screen_wh", wh);

  ri.island_tri_hidden.reload(r);
  ri.island_tri_hidden.bind();
  ri.island_tri_hidden.set_uniform_block_binding("Data", 0);
  ri.island_tri_hidden.set_bool("do_zoom", true);
  ri.island_tri_hidden.set_mat4("projection", camera.projection);

  ri.outline.reload(r);
  ri.outline.bind();
  ri.outline.set_uniform_block_binding("Data", 0);
  ri.outline.set_mat4("projection", camera.projection);
  ri.outline.set_bool("is_fullscreen", true);
  ri.outline.set_bool("do_zoom", false);
  ri.outline.set_int("tex_to_outline", tex_unit_sprites_to_outline);

  ri.island_shore.reload(r);
  ri.island_shore.bind();
  ri.island_shore.set_uniform_block_binding("Data", 0);
  ri.island_shore.set_bool("do_zoom", true);
  ri.island_shore.set_mat4("projection", camera.projection);

  ri.shine.reload(r);
  ri.shine.bind();
  ri.shine.set_uniform_block_binding("Data", 0);
  ri.shine.set_int("RENDERER_TEX_UNIT_COUNT", texs_used);
  ri.shine.set_bool("do_zoom", true);
  ri.shine.set_mat4("projection", camera.projection);
  for (int i = 0; i < (int)ri.user_textures.size(); i++) {
    const auto& tex = ri.user_textures[i];
    ri.shine.set_int("u_textures[" + std::to_string(i) + "]", tex.tex_unit.unit);
  }
  // ri.shine.set_int("tex", tex_unit_sprites_with_shield);

  ri.flame.reload(r);
  ri.flame.bind();
  ri.flame.set_int("RENDERER_TEX_UNIT_COUNT", texs_used);
  ri.flame.set_bool("do_zoom", true);
  ri.flame.set_mat4("projection", camera.projection);

  ri.outline.reload(r);
  ri.outline.bind();
  ri.outline.set_uniform_block_binding("Data", 0);
  ri.outline.set_mat4("projection", camera.projection);
  ri.outline.set_bool("is_fullscreen", true);
  ri.outline.set_bool("do_zoom", false);
  ri.outline.set_int("tex_to_outline", tex_unit_sprites_to_outline);

  ri.lighting_emitters_and_occluders.reload(r);
  ri.lighting_emitters_and_occluders.bind();
  ri.lighting_emitters_and_occluders.set_uniform_block_binding("Data", 0);
  ri.lighting_emitters_and_occluders.set_mat4("projection", camera.projection);

  ri.voronoi_seed.reload(r);
  ri.voronoi_seed.bind();
  ri.voronoi_seed.set_bool("is_fullscreen", true);
  ri.voronoi_seed.set_mat4("projection", camera.projection);
  // ri.voronoi_seed.set_int("tex", tex_unit_emitters_and_occluders);

  ri.jump_flood.reload(r);
  ri.jump_flood.bind();
  ri.jump_flood.set_bool("is_fullscreen", true);
  ri.jump_flood.set_mat4("projection", camera.projection);
  ri.jump_flood.set_vec2("screen_wh", ri.viewport_size_render_at);

  ri.voronoi_distance.reload(r);
  ri.voronoi_distance.bind();
  ri.voronoi_distance.set_bool("is_fullscreen", true);
  ri.voronoi_distance.set_mat4("projection", camera.projection);
  // ri.voronoi_distance.set_int("tex_emitters_and_occluders", tex_unit_emitters_and_occluders);
  ri.voronoi_distance.set_vec2("screen_wh", ri.viewport_size_render_at);

  ri.mix_lighting_and_scene.reload(r);
  ri.mix_lighting_and_scene.bind();
  ri.mix_lighting_and_scene.set_uniform_block_binding("Data", 0);
  ri.mix_lighting_and_scene.set_bool("is_fullscreen", true);
  ri.mix_lighting_and_scene.set_mat4("projection", camera.projection);
  ri.mix_lighting_and_scene.set_int("scene", tex_unit_linear_main);
  ri.mix_lighting_and_scene.set_int("tex_island_triangles", tex_unit_island_triangles);
  ri.mix_lighting_and_scene.set_int("tex_island_triangles_gradient", tex_unit_island_triangles_gradient);
  ri.mix_lighting_and_scene.set_int("tex_island_hidden", tex_unit_island_hidden);
  ri.mix_lighting_and_scene.set_int("tex_island_above_hidden", tex_unit_island_above_hidden);
  ri.mix_lighting_and_scene.set_int("tex_island_shore", tex_unit_island_shore);
  ri.mix_lighting_and_scene.set_int("tex_scene_0", tex_unit_linear_main);
  ri.mix_lighting_and_scene.set_int("tex_unit_water", tex_unit_water);
  ri.mix_lighting_and_scene.set_int("tex_outline", tex_unit_outline);
  ri.mix_lighting_and_scene.set_int("tex_shine_shells", tex_unit_shine_shells);
  ri.mix_lighting_and_scene.set_int("tex_flame", tex_unit_flame);
  ri.mix_lighting_and_scene.set_vec2("viewport_wh", wh);
  ri.mix_lighting_and_scene.set_bool("add_grid", true);
  ri.mix_lighting_and_scene.set_bool("add_vignette", true);
  // ri.mix_lighting_and_scene.set_int("tex_fluid", tex_unit_fluid);

  const auto& camera_c = get_first_component<OrthographicCamera>(r);
  ri.mix_lighting_and_scene.set_float("zoom", camera_c.zoom_nonlinear);

  //
  // bind fluidsim data
  //

  /*
    ri.fluid_sim.splatProgram.reload(r);
    ri.fluid_sim.splatProgram.bind();
    ri.fluid_sim.advectProgram.reload(r);
    ri.fluid_sim.advectProgram.bind();
    ri.fluid_sim.curlProgram.reload(r);
    ri.fluid_sim.curlProgram.bind();
    ri.fluid_sim.vorticityProgram.reload(r);
    ri.fluid_sim.vorticityProgram.bind();
    ri.fluid_sim.divergenceProgram.reload(r);
    ri.fluid_sim.divergenceProgram.bind();
    ri.fluid_sim.pressureProgram.reload(r);
    ri.fluid_sim.pressureProgram.bind();
    ri.fluid_sim.gradientSubtractProgram.reload(r);
    ri.fluid_sim.gradientSubtractProgram.bind();
    ri.fluid_sim.textureProgram.reload(r);
    ri.fluid_sim.textureProgram.bind();
    CHECK_OPENGL_ERROR(13);
    */

  // ri.blur.reload(r);
  // ri.blur.bind();
  // ri.blur.set_mat4("projection", camera.projection);

  // ri.bloom.reload(r);
  // ri.bloom.bind();
  // ri.bloom.set_mat4("projection", camera.projection);
  // ri.bloom.set_int("scene_texture", tex_unit_mix_lighting_and_scene);
  // ri.bloom.set_int("blur_texture", tex_unit_blur_pingpong_1);
};

void
init_render_system(const glm::vec2 screen_wh, entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  ri.viewport_size_render_at = screen_wh;
  ri.viewport_size_current = screen_wh;
  const auto& fbo_size = ri.viewport_size_render_at;

  // const int max_dim = glm::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  // const int n_jumpflood_passes = (int)(glm::ceil(glm::log(max_dim) / std::log(2.0f)));
  // SDL_Log("%s", std::format("jumpflood passes... {}", n_jumpflood_passes).c_str());

  // FBO textures
  Framebuffer::default_fbo();
  RenderCommand::set_viewport(0, 0, ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  RenderCommand::set_clear_colour_srgb({ 0.0f, 0.0f, 0.0f, 0.0f });
  RenderCommand::clear();

  // ri.passes.push_back(RenderPass(PassName::menu_fractal_shader));
  ri.passes.push_back(RenderPass(PassName::water));
  ri.passes.push_back(RenderPass(PassName::island_triangles));
  ri.passes.push_back(RenderPass(PassName::island_triangles_gradient));
  ri.passes.push_back(RenderPass(PassName::island_hidden));
  ri.passes.push_back(RenderPass(PassName::island_above_hidden));
  ri.passes.push_back(RenderPass(PassName::island_shore));
  // ri.passes.push_back(RenderPass(PassName::fluid_sim));
  ri.passes.push_back(RenderPass(PassName::linear_main));
  ri.passes.push_back(RenderPass(PassName::sprites_to_outline));
  ri.passes.push_back(RenderPass(PassName::outline));
  ri.passes.push_back(RenderPass(PassName::sprites_with_shield));
  ri.passes.push_back(RenderPass(PassName::shine));
  ri.passes.push_back(RenderPass(PassName::flame));
  // ri.passes.push_back(RenderPass(PassName::lighting_emitters_and_occluders));
  // // Use the Jump flood algorithm to generate a voroi diagram,
  // // then convert that in to a distance field
  // ri.passes.push_back(RenderPass(PassName::voronoi_seed));
  // ri.passes.push_back(RenderPass(PassName::jump_flood));
  // ri.passes.push_back(RenderPass(PassName::voronoi_distance));
  ri.passes.push_back(RenderPass(PassName::mix_lighting_and_scene));
  // ri.passes.push_back(RenderPass(PassName::blur_pingpong_0));
  // ri.passes.push_back(RenderPass(PassName::blur_pingpong_1));
  // ri.passes.push_back(RenderPass(PassName::bloom));

  // Super sampling, innit
  const auto double_wh = glm::vec2{ 2.0f * fbo_size.x, 2.0f * fbo_size.y };

  for (auto& rp : ri.passes)
    rp.setup(double_wh);

  // Load fluidsim shaders/textures
  int used_tex_units = get_renderer_tex_unit_count(ri);
  // load_fluidsim(r, ri.fluid_sim, used_tex_units);

  // Load user textures
  for (int i = 0; i < (int)ri.user_textures.size(); i++) {
    auto& tex = ri.user_textures[i];
    tex.tex_unit.unit = used_tex_units + i;
    const LinearTexture loaded_tex = engine::load_texture_linear(tex.tex_unit.unit, tex.path);
    tex.tex_id.id = setup_linear_texture(loaded_tex);
    tex.size = glm::vec2{ loaded_tex.width, loaded_tex.height };
    SDL_Log("%s", std::format("loaded texture... {}, ncomp: {}", tex.path, loaded_tex.nr_components).c_str());
  }

  ri.water = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_worley_noise_water.frag");
  ri.instanced = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_instanced.frag");
  ri.instanced_tri = Shader(r, "assets/shaders/2d_instanced_tri.vert", "assets/shaders/2d_instanced_tri.frag");
  ri.island_tri_gradient = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_island_tri_gradient.frag");
  ri.island_tri_hidden = Shader(r, "assets/shaders/2d_instanced_tri.vert", "assets/shaders/2d_island_tri_hidden.frag");
  ri.island_shore = Shader(r, "assets/shaders/2d_instanced_tri.vert", "assets/shaders/2d_island_shore.frag");
  ri.shine = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_shine.frag");
  ri.flame = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_flame.frag");
  ri.outline = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_outline.frag");
  ri.lighting_emitters_and_occluders =
    Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_emitters_and_occluders.frag");
  ri.voronoi_seed = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_seed.frag");
  ri.jump_flood = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_jump_flood.frag");
  ri.voronoi_distance = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_voronoi_distance.frag");
  ri.mix_lighting_and_scene = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_mix_lighting_and_scene.frag");
  // ri.crt = Shader(r, "assets/shaders/2d_instanced.vert", "assets/shaders/2d_crt_effect.frag");
  // ri.blur = Shader(r, "assets/shaders/bloom.vert", "assets/shaders/blur.frag");
  // ri.bloom = Shader(r, "assets/shaders/bloom.vert", "assets/shaders/bloom.frag");

  // initialize renderer
#if !defined(__EMSCRIPTEN__)
  glEnable(GL_MULTISAMPLE);
#endif

  // glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  engine::print_gpu_info();

  // init(): create a dynamic VBO
  ri.renderer.init();
  ri.tri_renderer.init();

  // generate ubo
  {
    GLuint ubo;
    glGenBuffers(1, &ubo);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UboData), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // set as binding point 0
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, sizeof(UboData));

    ri.tex_unit_ubo_data = ubo;
  }

  // update ubo data
  {
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera_c = r.get<OrthographicCamera>(camera_e);
    data.time = 0;
    data.view = camera_c.view;
    data.camera_pos = { camera_t.position.x, camera_t.position.y };
    data.zoom = camera_c.zoom_nonlinear;
    auto grid_e = get_first<Effect_GridComponent>(r);
    if (grid_e != entt::null)
      data.tilesize = r.get<Effect_GridComponent>(grid_e).gridsize;

    glBindBuffer(GL_UNIFORM_BUFFER, ri.tex_unit_ubo_data);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UboData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
  }

  rebind(r, ri);

  // adds the update() for each renderpass
  setup_water_update(r);
  setup_island_triangles_update(r);
  setup_island_triangles_gradient_update(r);
  setup_island_hidden_update(r);
  setup_island_above_hidden_update(r);
  setup_island_shore_update(r);
  setup_linear_main_update(r);
  setup_sprites_to_outline_update(r);
  setup_outline_update(r);
  setup_sprites_with_shield_update(r);
  setup_shine_update(r);
  setup_flame_update(r);
  setup_mix_lighting_and_scene_update(r);
  // setup_lighting_emitters_and_occluders_update(r);
  // setup_voronoi_seed_update(r);
  // setup_jump_flood_pass(r);
  // setup_voronoi_distance_field_update(r);
  // setup_fluidsim_update(r);
  // setup_crt_effect_update(r);
  // setup_gaussian_blur_update(r);
  // setup_bloom_update(r);

  // validate that update() been set...
  for (const auto& pass : ri.passes) {
    const auto type_name = std::string(magic_enum::enum_name(pass.pass));
    if (!pass.update) {
      SDL_Log("%s", std::format("ERROR! RenderPass Update() not set for {}", type_name).c_str());
      exit(1); // explode
    }
  }

  CHECK_OPENGL_ERROR(3);
};

void
update_render_system(entt::registry& r, const float dt, const glm::vec2& mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& scene = SINGLE_CurrentScene::instance;
  auto& ri = SINGLE_RendererInfo::instance;
  static const engine::SRGBColour black(0, 0, 0, 0);

  // #if defined(_DEBUG)
  //   CHECK_OPENGL_ERROR(1337); // check a unique error code every update()
  // #endif

  static float time = 0.0f;
  {
    ZoneScopedN("IncrementTime");
    time += dt;
  }

  ri.viewport_size_current = { ImGui::GetMainViewport()->WorkSize.x, ImGui::GetMainViewport()->WorkSize.y };
  if (check_if_viewport_resize(ri))
    rebind(r, ri);

#if defined(_DEBUG)
  // reload all shaders
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input, SDL_SCANCODE_0)) {
    SDL_Log("(DEBUG) Reloading shaders");
    rebind(r, ri);
  }
#endif

  const auto double_wh = glm::vec2{ 2.0f * ri.viewport_size_render_at.x, 2.0f * ri.viewport_size_render_at.y };
  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_t = r.get<TransformComponent>(camera_e);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto& screenshake_c = SINGLE_ScreenshakeComponent::instance;

  // update ubo data
  {
#if defined(_DEBUG)
    ZoneScopedN("UpdateUBOData");
#endif
    data.projection_zoomed = camera_c.projection_zoomed;
    data.view = camera_c.view;
    data.camera_pos = { camera_t.position.x, camera_t.position.y };
    data.time = time;
    data.zoom = camera_c.zoom_nonlinear;
    data.screenshake = screenshake_c.strength;
  }

  {
#if defined(_DEBUG)
    ZoneScopedN("UpdateLights");
#endif

    // .w as 0 indicates light inactive.
    const int n_lights = 32;
    for (int i = 0; i < n_lights; i++) {
      data.light_positions[i].x = 0.0f;
      data.light_positions[i].y = 0.0f;
      data.light_positions[i].z = 0.0f;
      data.light_positions[i].w = 0.0f;
    }

    // update light emitters (that arnt players)
    int i = 0;

    const auto view0 = r.view<const LightEmitterComponent, const TransformComponent, const LightTypeWedge>();
    for (const auto& [e, light_c, t_c, type_c] : view0.each()) {
      if (i == n_lights)
        break;
      const auto dir = angle_radians_to_direction(t_c.rotation_radians.z);
      const float offset = 0.0f;
      data.light_positions[i].x = t_c.position.x + dir.x * offset;
      data.light_positions[i].y = t_c.position.y + dir.y * offset;
      data.light_positions[i].z = engine::clamp_axis(t_c.rotation_radians.z);
      data.light_positions[i].w = 1.0f;
      i++;
    }

    const auto view1 = r.view<const LightEmitterComponent, const TransformComponent, const LightTypeCircle>();
    for (const auto& [e, light_c, t_c, type_c] : view1.each()) {
      if (i == n_lights)
        break;
      data.light_positions[i].x = t_c.position.x;
      data.light_positions[i].y = t_c.position.y;
      data.light_positions[i].z = -10.0f; // -10 is < [-pi, pi], meaning it cant be an angle
      data.light_positions[i].w = 1.0f;
      i++;
    }
  }

  // Note: this updates the entire array.
  // We could update only the parts that change
  {
#if defined(_DEBUG)
    ZoneScopedN("UpdateUBO-OpenGLCalls");
#endif
    glBindBuffer(GL_UNIFORM_BUFFER, ri.tex_unit_ubo_data);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UboData), &data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  for (const auto& pass : ri.passes) {
    Framebuffer::bind_fbo(pass.fbos[0]);
    RenderCommand::set_viewport(0, 0, double_wh.x, double_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    pass.update(r, dt, mouse_pos);
  }

  // Default: render_texture_to_imgui
  // Render the last renderpass texture to the final output
  {
#if defined(_DEBUG)
    ZoneScopedN("UpdateLastRenderPass");
#endif

    // last stage, dont double the framebuffer
    const auto viewport_wh = ri.viewport_size_render_at;

    Framebuffer::default_fbo();
    RenderCommand::set_viewport(0, 0, viewport_wh.x, viewport_wh.y);
    RenderCommand::set_clear_colour_srgb(black);
    RenderCommand::clear();

    // Which pass to render finally?
    PassName p = PassName::mix_lighting_and_scene;

    // Note: ImGui::Image takes in TexID not TexUnit
    const auto& pass = ri.passes[(int)p];
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
    ZoneScopedN("UpdateDebugTextures");

    auto& state_c = get_first_component<SINGLE_DebugMenuBar>(r);
    const bool show_debug_textures = gesert_menubar_state(state_c, "DebugTextures").enabled;
    if (show_debug_textures) {
      // Debug Passes
      for (const auto& rp : ri.passes) {
        const auto pass_name = std::string(magic_enum::enum_name(rp.pass));
        for (const auto& tex : rp.texs) {
          const std::string label = std::format("TexUnit: {}, Tex: {}, Id: {}", tex.tex_unit.unit, pass_name, tex.tex_id.id);
          ImGui::Begin(label.c_str());
          const ImVec2 viewport_size = ImGui::GetContentRegionAvail();
          const uint64_t id = tex.tex_id.id;
          ImGui::Image((ImTextureID)id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
          ImGui::End();
        }
      }
      // Debug user Texture
      for (const auto& tex : ri.user_textures) {
        const std::string label = std::format("TexUnit: {}, Tex: {}, Id: {}", tex.tex_unit.unit, tex.path, tex.tex_id.id);
        ImGui::Begin(label.c_str());
        ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        const uint64_t id = tex.tex_id.id;
        ImGui::Image((ImTextureID)id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
        ImGui::End();
      }
    }
    const bool show_debug_fluid_textures = gesert_menubar_state(state_c, "DebugFluidTextures").enabled;
    if (show_debug_fluid_textures) {
      const auto debug_texture = [](const std::string title, TextureId id) {
        ImGuiWindowFlags flags = 0;
        ImGui::SetNextWindowSizeConstraints({ 100, 100 }, { FLT_MAX, FLT_MAX });
        ImGui::Begin(title.c_str(), NULL, flags);
        const ImVec2 viewport_size = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)id.id, viewport_size, ImVec2(0, 0), ImVec2(1, 1));
        ImGui::End();
      };
      // Debug fluidsim textures.
      const auto& fluidsim_data = ri.fluid_sim;
      debug_texture({ "dye-r" }, fluidsim_data.dye.read().tex.tex_id);
      debug_texture({ "dye-w" }, fluidsim_data.dye.write().tex.tex_id);
      debug_texture({ "vel-r" }, fluidsim_data.velocity.read().tex.tex_id);
      debug_texture({ "vel-w" }, fluidsim_data.velocity.write().tex.tex_id);
      debug_texture({ "curl" }, fluidsim_data.curl.info.tex.tex_id);
      debug_texture({ "divergence" }, fluidsim_data.divergence.info.tex.tex_id);
      debug_texture({ "pressure-r" }, fluidsim_data.pressure.read().tex.tex_id);
      debug_texture({ "pressure-w" }, fluidsim_data.pressure.write().tex.tex_id);
    }
  }
#endif
};

void
end_frame_render_system(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  ri.renderer.end_frame();
  ri.tri_renderer.end_frame();
};

} // namespace game2d
