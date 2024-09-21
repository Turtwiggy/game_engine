#include "passes.hpp"

#include "actors/helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/map/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/renderer/lights/components.hpp"

#include "engine/deps/opengl.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"

namespace game2d {
using namespace engine;
using namespace std::literals;

int
get_tex_unit(const SINGLE_RendererInfo& ri, const PassName& p)
{
  const auto linear_pass_idx = search_for_renderpass_by_name(ri, p);
  const auto& linear_pass = ri.passes[linear_pass_idx];
  return linear_pass.texs[0].tex_unit.unit;
};

const auto render_fullscreen_quad = [](entt::registry& r, const engine::Shader& shader, const glm::ivec2& size) {
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ri.renderer.reset_quad_vert_count();
  ri.renderer.begin_batch();

  engine::quad_renderer::RenderDescriptor desc;
  desc.pos_tl = { 0, 0 };
  desc.size = size;
  desc.angle_radians = 0;
  ri.renderer.draw_sprite(desc, shader);

  ri.renderer.end_batch();
  ri.renderer.flush(shader);
};

void
setup_stars_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::stars);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);
    const auto& camera_t = r.get<TransformComponent>(camera_e);

    engine::RenderCommand::set_clear_colour_linear({ 0.0f, 0.0f, 0.0f, 1.0f });
    engine::RenderCommand::clear();

    // Render stars shader
    ri.stars.bind();
    ri.stars.set_mat4("view", camera.view);

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        ri.renderer.draw_sprite(desc, ri.stars);
      }
      ri.renderer.end_batch();
      ri.renderer.flush(ri.stars);
    }
  };
};

void
setup_debris_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::debris);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera_c = r.get<OrthographicCamera>(camera_e);

    ri.debris.bind();
    ri.debris.set_mat4("view", camera_c.view);
    ri.debris.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    ri.debris.set_float("zoom", camera_c.zoom_nonlinear);

    ImGui::Text("zoom l %f", camera_c.zoom_linear);
    ImGui::Text("zoom nl %f", camera_c.zoom_nonlinear);

    // ri.debris.set_mat4("projection", camera_c.projection_zoomed);

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        ri.renderer.draw_sprite(desc, ri.debris);
      }
      ri.renderer.end_batch();
      ri.renderer.flush(ri.debris);
    }
  };
};

void
setup_floor_mask_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::floor_mask);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto& camera_c = get_first_component<OrthographicCamera>(r);

    ri.instanced.bind();
    ri.instanced.set_mat4("view", camera_c.view);

    // Render floor quads in to floor-mask texture.
    engine::LinearColour mask_colour = engine::LinearColour(1.0f, 1.0f, 1.0f, 1.0f);

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      const auto& view = r.view<TransformComponent, SpriteComponent, FloorComponent>();

      for (const auto& [e, transform, sc, floor_c] : view.each()) {
        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform.position - (transform.scale * 0.5f);
        desc.size = transform.scale;
        desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
        desc.colour = mask_colour;
        desc.tex_unit = sc.tex_unit;

        desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
        desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
        desc.sprites_max = { sc.total_sx, sc.total_sy };

        ri.renderer.draw_sprite(desc, ri.instanced);
      }

      ri.renderer.end_batch();
      ri.renderer.flush(ri.instanced);
    }
  };
};

void
setup_linear_main_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::linear_main);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera_c = r.get<OrthographicCamera>(camera_e);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    ri.instanced.bind();
    ri.instanced.set_mat4("view", camera_c.view);
    ri.instanced.set_mat4("projection", camera_c.projection_zoomed);

    // set the positions of the units with circles units & update TBO
    {
      static std::vector<CircleComponent> points(100);
      const auto view = r.view<TransformComponent, CircleComponent>();
      for (int i = 0; const auto& [e, transform_c, circle_c] : view.each()) {
        if (i > 100)
          break;
        circle_c.shader_pos = get_position(r, e);
        points[i] = circle_c;
        i++;
      }
      // Update for TBO
      glBindBuffer(GL_TEXTURE_BUFFER, ri.renderer.data.TBO);
      glBufferSubData(GL_TEXTURE_BUFFER, 0, sizeof(CircleComponent) * points.size(), points.data());
      glBindTexture(GL_TEXTURE_BUFFER, ri.renderer.data.TEX);
      glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, ri.renderer.data.TBO);
    }

    // Render some quads
    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      const auto& group = r.group<TransformComponent, SpriteComponent>();

      // sort by z-index; adds 0.5ms
      group.sort<TransformComponent>([](const auto& a, const auto& b) { return a.z_index < b.z_index; });

      for (const auto& [e, transform, sc] : group.each()) {
        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform.position - (transform.scale * 0.5f);
        desc.size = transform.scale;
        desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
        desc.colour = sc.colour;
        desc.tex_unit = sc.tex_unit;

        desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
        desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
        desc.sprites_max = { sc.total_sx, sc.total_sy };

        ri.renderer.draw_sprite(desc, ri.instanced);
      }

      ri.renderer.end_batch();
      ri.renderer.flush(ri.instanced);
    }
  };
};

void
setup_lighting_emitters_and_occluders_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::lighting_emitters_and_occluders);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    // emitters should be anything but black (i.e. scene lighting)
    const engine::LinearColour emitter_col = engine::SRGBToLinear({ 255, 0, 0, 1.0f });
    const engine::LinearColour occluder_col(0.0f, 0.0f, 0.0f, 1.0f);

    ri.lighting_emitters_and_occluders.bind();
    ri.lighting_emitters_and_occluders.set_mat4("view", camera.view);

    {
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0.0f });
      engine::RenderCommand::clear();

      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      /*
      {
        const auto& emitters = r.view<const LightEmitterComponent, const TransformComponent, const SpriteComponent>();
        for (const auto& [entity, emitter, transform, sc] : emitters.each()) {
          engine::quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale * 0.5f;
          desc.size = transform.scale;
          desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
          desc.colour = emitter_col;
          desc.tex_unit = sc.tex_unit;

          desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
          desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
          desc.sprites_max = { sc.total_sx, sc.total_sy };

          ri.renderer.draw_sprite(desc, ri.lighting_emitters_and_occluders);
        }
      }
      */

      // draw occluders
      {
        const auto& occluders = r.view<const LightOccluderComponent, const TransformComponent, const SpriteComponent>();
        for (const auto& [entity, occluder, transform, sc] : occluders.each()) {
          engine::quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale * 0.5f;
          desc.size = transform.scale;
          desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
          desc.colour = occluder_col;
          desc.tex_unit = sc.tex_unit;

          desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
          desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
          desc.sprites_max = { sc.total_sx, sc.total_sy };

          ri.renderer.draw_sprite(desc, ri.lighting_emitters_and_occluders);
        }
      }
      ri.renderer.end_batch();
      ri.renderer.flush(ri.lighting_emitters_and_occluders);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
  };
};

void
setup_voronoi_seed_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::voronoi_seed);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&ri](entt::registry& r) {
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    const int tex_unit_emitters_and_occluders = get_tex_unit(ri, PassName::lighting_emitters_and_occluders);

    ri.voronoi_seed.bind();

    render_fullscreen_quad(r, ri.voronoi_seed, ri.viewport_size_render_at);
  };
};

void
setup_jump_flood_pass(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::jump_flood);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&pass](entt::registry& r) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    ri.jump_flood.bind();

    const auto wh = ri.viewport_size_render_at;
    const int max_dim = glm::max(wh.x, wh.y);
    const int n_jumpflood_passes = (int)(glm::ceil(glm::log(max_dim) / std::log(2.0f)));

    for (int i = 0; i < n_jumpflood_passes; i++) {

      engine::Framebuffer::bind_fbo(pass.fbos[i]);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0.0f });
      engine::RenderCommand::clear();

      // offset for each pass is half the previous one, starting at half the square resolution rounded up to nearest
      // power 2. #i.e.for 768x512 we round up to 1024x1024 and the offset for the first pass is 512x512, then 256x256,
      // etc.
      const float offset = (float)std::pow(2, n_jumpflood_passes - i - 1);

      int tex_unit = get_tex_unit(ri, PassName::voronoi_seed);
      if (i > 0)
        tex_unit = pass.texs[i - 1].tex_unit.unit;

      ri.jump_flood.set_int("tex", tex_unit);
      ri.jump_flood.set_float("u_offset", offset);

      render_fullscreen_quad(r, ri.jump_flood, ri.viewport_size_render_at);
    }
  };
};

void
setup_voronoi_distance_field_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::voronoi_distance);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto jflood_pass_idx = search_for_renderpass_by_name(ri, PassName::jump_flood);
    const auto& jflood_pass = ri.passes[jflood_pass_idx];
    const auto& jflood_texs = jflood_pass.texs;
    const auto last_jflood_texunit = jflood_texs[jflood_texs.size() - 1].tex_unit.unit;
    const auto tex_unit = get_tex_unit(ri, PassName::lighting_emitters_and_occluders);

    ri.voronoi_distance.bind();
    ri.voronoi_distance.set_int("tex_jflood", last_jflood_texunit);
    ri.voronoi_distance.set_int("tex_emitters_and_occluders", tex_unit);

    render_fullscreen_quad(r, ri.voronoi_distance, ri.viewport_size_render_at);
  };
};

void
setup_mix_lighting_and_scene_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::mix_lighting_and_scene);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    static float brightness_threshold = 0.80f;

#if defined(_DEBUG)
    imgui_draw_float("brightness_threshold", brightness_threshold);
#endif

    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera_c = r.get<OrthographicCamera>(camera_e);

    // update uniforms
    ri.mix_lighting_and_scene.bind();

    const glm::vec2 mouse_raw = get_mouse_pos() - ri.viewport_pos;
    ri.mix_lighting_and_scene.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    // ri.mix_lighting_and_scene.set_float("brightness_threshold", brightness_threshold);

    if (camera_c.zoom_nonlinear != 0.0f)
      ri.mix_lighting_and_scene.set_float("zoom", camera_c.zoom_nonlinear);

    float tilesize = 50;
    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null)
      tilesize = r.get<MapComponent>(map_e).tilesize;
    ri.mix_lighting_and_scene.set_float("tilesize", tilesize);

    render_fullscreen_quad(r, ri.mix_lighting_and_scene, ri.viewport_size_render_at);
  };
};

void
setup_gaussian_blur_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass0_idx = search_for_renderpass_by_name(ri, PassName::blur_pingpong_0);
  const auto pass1_idx = search_for_renderpass_by_name(ri, PassName::blur_pingpong_1);
  auto& pass0 = ri.passes[pass0_idx];
  auto& pass1 = ri.passes[pass1_idx];

  pass0.update = [&pass0, &pass1](entt::registry& r) {
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    static float blur_amount = 4;
#if defined(_DEBUG)
    imgui_draw_float("blur_amount", blur_amount);
#endif

    bool horizontal = true;
    bool first_iteration = true;

    // the second texture in the brightness pass is the brightnes output colour attachment
    int last_blur_texunit = 0;

    // update uniforms
    ri.blur.bind();

    for (int i = 0; i < blur_amount; i++) {
      if (horizontal)
        engine::Framebuffer::bind_fbo(pass0.fbos[0]);
      else
        engine::Framebuffer::bind_fbo(pass1.fbos[0]);

      ri.blur.set_bool("horizontal", horizontal);
      if (first_iteration) {
        const auto tex_unit = get_tex_unit(ri, PassName::lighting_emitters_and_occluders);
        ri.blur.set_int("tex", tex_unit);
        first_iteration = false;
      } else {

        const int tex_unit =
          horizontal ? get_tex_unit(ri, PassName::blur_pingpong_0) : get_tex_unit(ri, PassName::blur_pingpong_1);
        ri.blur.set_int("tex", tex_unit);

        last_blur_texunit = tex_unit;
      }
      horizontal = !horizontal;

      // Pass data from this render stage onwards
      const auto& blur_data_e = get_first<Effect_BlurInfo>(r);
      if (blur_data_e == entt::null)
        create_empty<Effect_BlurInfo>(r);
      Effect_BlurInfo& blur_data_c = get_first_component<Effect_BlurInfo>(r);
      blur_data_c.last_blur_texunit = last_blur_texunit;

      engine::RenderCommand::set_viewport(0, 0, ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0.0f });
      engine::RenderCommand::clear();

      render_fullscreen_quad(r, ri.blur, ri.viewport_size_render_at);
    }
  };

  pass1.update = [](entt::registry& r) {}; // nothing, only used for fbo
};

void
setup_bloom_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::bloom);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r) {
    auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const bool do_bloom = get_first<Effect_DoBloom>(r) != entt::null;
    static float exposure = 1.5f;
#if defined(_DEBUG)
    imgui_draw_float("exposure", exposure);
#endif
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    ri.bloom.bind();
    ri.bloom.set_mat4("view", camera.view);
    ri.bloom.set_float("exposure", exposure);
    ri.bloom.set_bool("do_bloom", do_bloom);

    const auto& blur_data = get_first_component<Effect_BlurInfo>(r);
    ri.bloom.set_int("blur_texture", blur_data.last_blur_texunit);

    // render_fullscreen_quad(r, ri.bloom, ri.viewport_size_render_at);

    ri.renderer.reset_quad_vert_count();
    ri.renderer.begin_batch();

    // render completely over screen
    {
      engine::quad_renderer::RenderDescriptor desc;
      const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
      desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
      desc.size = ri.viewport_size_render_at;
      desc.angle_radians = 0;
      ri.renderer.draw_sprite(desc, ri.bloom);
    }

    ri.renderer.end_batch();
    ri.renderer.flush(ri.bloom);
  };
};

} // namespace game2d