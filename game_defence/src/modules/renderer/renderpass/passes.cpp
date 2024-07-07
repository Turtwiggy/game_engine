#include "passes.hpp"

#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "imgui/helpers.hpp"
#include "modules/camera/orthographic.hpp"
#include "modules/gameplay_circle/components.hpp"
#include "modules/lighting/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/helpers/batch_quad.hpp"
#include "modules/vfx_grid/components.hpp"
#include "sprites/components.hpp"

#include "opengl/render_command.hpp"

namespace game2d {
using namespace std::literals;

const auto render_fullscreen_quad = [](const engine::Shader& shader, const glm::ivec2& size) {
  engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
  engine::quad_renderer::QuadRenderer::begin_batch();

  engine::quad_renderer::RenderDescriptor desc;
  desc.pos_tl = { 0, 0 };
  desc.size = size;
  desc.angle_radians = 0;
  engine::quad_renderer::QuadRenderer::draw_sprite(desc, shader);

  engine::quad_renderer::QuadRenderer::end_batch();
  engine::quad_renderer::QuadRenderer::flush(shader);
};

const auto get_tex_unit = [](const SINGLETON_RendererInfo& ri, const PassName& p) -> int {
  const auto linear_pass_idx = search_for_renderpass_by_name(ri, p);
  const auto& linear_pass = ri.passes[linear_pass_idx];
  return linear_pass.texs[0].tex_unit.unit;
};

void
setup_stars_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::stars);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);
    const auto& camera_t = r.get<TransformComponent>(camera_e);

    // Render stars shader
    ri.stars.bind();
    ri.stars.set_mat4("view", camera.view);
    {
      engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
      engine::quad_renderer::QuadRenderer::begin_batch();
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.stars);
      }
      engine::quad_renderer::QuadRenderer::end_batch();
      engine::quad_renderer::QuadRenderer::flush(ri.stars);
    }
  };
};

void
setup_linear_main_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::linear_main);
  auto& pass = ri.passes[pass_idx];
  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);
    const auto& camera_t = r.get<TransformComponent>(camera_e);

    // Render grid shader
    ri.grid.bind();
    ri.grid.set_mat4("view", camera.view);
    ri.grid.set_vec2("viewport_wh", ri.viewport_size_render_at);
    ri.grid.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    const auto grid_e = get_first<Effect_GridComponent>(r);
    if (grid_e != entt::null) {
      const float gridsize = r.get<Effect_GridComponent>(grid_e).gridsize;
      ri.grid.set_float("gridsize", gridsize);
    }
    if (get_first<Effect_GridComponent>(r) != entt::null) {
      engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
      engine::quad_renderer::QuadRenderer::begin_batch();

      // grid post-processing
      // render completely over screen
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.grid);
      }

      engine::quad_renderer::QuadRenderer::end_batch();
      engine::quad_renderer::QuadRenderer::flush(ri.grid);
    }

    // Render some quads
    ri.instanced.bind();
    ri.instanced.set_mat4("view", camera.view);
    ri.instanced.set_vec2("viewport_wh", ri.viewport_size_render_at);
    {
      engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
      engine::quad_renderer::QuadRenderer::begin_batch();

      const auto& group = r.group<TransformComponent, SpriteComponent>();

      // sort by z-index; adds 0.5ms
      group.sort<TransformComponent>([](const auto& a, const auto& b) { return a.position.z < b.position.z; });

      for (const auto& [entity, transform, sc] : group.each()) {
        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform.position - transform.scale / 2;
        desc.size = transform.scale;
        desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
        desc.colour = sc.colour;
        desc.tex_unit = sc.tex_unit;

        desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
        desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
        desc.sprites_max = { sc.total_sx, sc.total_sy };

        engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.instanced);
      }

      engine::quad_renderer::QuadRenderer::end_batch();
      engine::quad_renderer::QuadRenderer::flush(ri.instanced);
    }

    // Render some SDF circles
    ri.circle.bind();
    ri.circle.set_mat4("view", camera.view);
    ri.circle.set_vec2("viewport_wh", ri.viewport_size_render_at);
    ri.circle.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });

    {
      engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
      engine::quad_renderer::QuadRenderer::begin_batch();

      // set the positions of the player units
      {
        ri.circle.bind();
        const auto& player_view = r.view<TransformComponent, CircleComponent>();
        for (int i = 0; const auto& [e, transform, c] : player_view.each()) {
          const glm::vec2 pos = { float(transform.position.x), float(transform.position.y) };
          std::string label = "points["s + std::to_string(i) + "].pos"s;
          ri.circle.set_vec2(label, pos);
          i++;
        }
      }

      // circle post-processing
      // render completely over screen
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.angle_radians = 0;
        engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.circle);
      }

      engine::quad_renderer::QuadRenderer::end_batch();
      engine::quad_renderer::QuadRenderer::flush(ri.circle);
    }
  };
};

void
setup_lighting_emitters_and_occluders_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::lighting_emitters_and_occluders);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    // emitters should be anything but black
    // const engine::SRGBColour emitter_col{ 255, 0, 0, 1.0f };
    const engine::LinearColour background_col(0.0f, 0.0f, 0.0f, 0.0f);
    const engine::LinearColour occluder_col(1.0f, 1.0f, 1.0f, 1.0f);

    engine::RenderCommand::set_clear_colour_linear(background_col);

    ri.lighting_emitters_and_occluders.bind();
    ri.lighting_emitters_and_occluders.set_mat4("view", camera.view);

    {
      engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
      engine::quad_renderer::QuadRenderer::begin_batch();

      // draw occluders
      {
        const auto& occluders = r.view<const LightOccluderComponent, const TransformComponent, const SpriteComponent>();
        for (const auto& [entity, occluder, transform, sc] : occluders.each()) {
          engine::quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale / 2;
          desc.size = transform.scale;
          desc.angle_radians = sc.angle_radians + transform.rotation_radians.z;
          desc.colour = occluder_col;
          desc.tex_unit = sc.tex_unit;

          desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
          desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
          desc.sprites_max = { sc.total_sx, sc.total_sy };

          engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.lighting_emitters_and_occluders);
        }
      }
      engine::quad_renderer::QuadRenderer::end_batch();
      engine::quad_renderer::QuadRenderer::flush(ri.lighting_emitters_and_occluders);
    }
  };
};

void
setup_lighting_ambient_occlusion_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::lighting_ambient_occlusion);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

    ri.lighting_ambient_occlusion.bind();

    //

    render_fullscreen_quad(ri.lighting_ambient_occlusion, ri.viewport_size_render_at);
  };
};

void
setup_mix_lighting_and_scene_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::mix_lighting_and_scene);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    static float brightness_threshold = 0.80f;
    imgui_draw_float("brightness_threshold", brightness_threshold);

    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    // update uniforms
    ri.mix_lighting_and_scene.bind();
    ri.mix_lighting_and_scene.set_int("scene_0", get_tex_unit(ri, PassName::linear_main));
    ri.mix_lighting_and_scene.set_int("scene_1", get_tex_unit(ri, PassName::stars));

    // light_pos.x should be between 0 < viewport_wh.x
    // light_pos.y should be between 0 < viewport_wh.y
    const glm::vec2 mouse_raw = get_mouse_pos() - ri.viewport_pos;
    ri.mix_lighting_and_scene.set_vec2("light_pos", mouse_raw);
    ri.mix_lighting_and_scene.set_vec2("viewport_wh", ri.viewport_size_render_at);
    ri.mix_lighting_and_scene.set_vec2("camera_pos", { camera_t.position.x, camera_t.position.y });
    ri.mix_lighting_and_scene.set_vec2("mouse_pos", mouse_raw);
    ri.mix_lighting_and_scene.set_float("brightness_threshold", brightness_threshold);
    // ri.mix_lighting_and_scene.set_float("time", time);

    render_fullscreen_quad(ri.mix_lighting_and_scene, ri.viewport_size_render_at);
  };
};

void
setup_gaussian_blur_update(entt::registry& r)
{
  static const engine::SRGBColour black(0, 0, 0, 1.0f);
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass0_idx = search_for_renderpass_by_name(ri, PassName::blur_pingpong_0);
  const auto pass1_idx = search_for_renderpass_by_name(ri, PassName::blur_pingpong_1);
  auto& pass0 = ri.passes[pass0_idx];
  auto& pass1 = ri.passes[pass1_idx];

  pass0.update = [&r, &pass0, &pass1]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    static float blur_amount = 4;
    imgui_draw_float("blur_amount", blur_amount);

    bool horizontal = true;
    bool first_iteration = true;

    // the second texture in the brightness pass is the brightnes output colour attachment
    int last_blur_texunit = 0;

    // update uniforms
    ri.blur.bind();

    for (int i = 0; i < blur_amount; i++) {
      if (horizontal)
        engine::Framebuffer::bind_fbo(pass0.fbo);
      else
        engine::Framebuffer::bind_fbo(pass1.fbo);

      ri.blur.set_bool("horizontal", horizontal);
      if (first_iteration) {
        const auto brightness_pass_idx = search_for_renderpass_by_name(ri, PassName::mix_lighting_and_scene);
        const auto brightness_tex_unit = ri.passes[brightness_pass_idx].texs[1].tex_unit.unit;
        ri.blur.set_int("tex", brightness_tex_unit);
        first_iteration = false;
      } else {
        const int tex_unit = horizontal ? pass0.texs[0].tex_unit.unit : pass1.texs[0].tex_unit.unit;
        ri.blur.set_int("tex", tex_unit);
        last_blur_texunit = tex_unit;
      }
      horizontal = !horizontal;

      // Pass data from this render stage onwards
      const auto& blur_data_e = get_first<EffectBlurInfo>(r);
      if (blur_data_e == entt::null)
        create_empty<EffectBlurInfo>(r);
      EffectBlurInfo& blur_data_c = get_first_component<EffectBlurInfo>(r);
      blur_data_c.last_blur_texunit = last_blur_texunit;

      engine::RenderCommand::set_viewport(0, 0, ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
      engine::RenderCommand::set_clear_colour_srgb(black);
      engine::RenderCommand::clear();

      render_fullscreen_quad(ri.blur, ri.viewport_size_render_at);
    }
  };

  pass1.update = []() {}; // nothing, only used for fbo
};

void
setup_bloom_update(entt::registry& r)
{
  auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto pass_idx = search_for_renderpass_by_name(ri, PassName::bloom);
  auto& pass = ri.passes[pass_idx];
  pass.update = [&r]() {
    const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
    const bool do_bloom = get_first<Effect_DoBloom>(r) != entt::null;
    static float exposure = 1.5f;
    imgui_draw_float("exposure", exposure);

    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    ri.bloom.bind();
    ri.bloom.set_mat4("view", camera.view);
    ri.bloom.set_float("exposure", exposure);
    ri.bloom.set_bool("do_bloom", do_bloom);

    const auto& blur_data = get_first_component<EffectBlurInfo>(r);
    ri.bloom.set_int("blur_texture", blur_data.last_blur_texunit);

    // render_fullscreen_quad(ri.bloom, ri.viewport_size_render_at);

    engine::quad_renderer::QuadRenderer::reset_quad_vert_count();
    engine::quad_renderer::QuadRenderer::begin_batch();

    // render completely over screen
    {
      engine::quad_renderer::RenderDescriptor desc;
      const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
      desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
      desc.size = ri.viewport_size_render_at;
      desc.angle_radians = 0;
      engine::quad_renderer::QuadRenderer::draw_sprite(desc, ri.bloom);
    }

    engine::quad_renderer::QuadRenderer::end_batch();
    engine::quad_renderer::QuadRenderer::flush(ri.bloom);
  };
};
} // namespace game2d