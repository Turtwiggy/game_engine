#include "passes.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/core/camera/orthographic.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/renderer/helpers/batch_quad.hpp"
#include "modules/core/renderer/helpers/batch_triangle.hpp"
#include "modules/effects_outline/outline_components.hpp"
#include "modules/systems/system_above_fog/above_fog_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"

namespace game2d {
using namespace engine;
using namespace std::literals;

const auto render_fullscreen_quad = [](entt::registry& r, const engine::Shader& shader, const glm::ivec2& size) {
  auto& ri = SINGLE_RendererInfo::instance;

  ri.renderer.reset_quad_vert_count();
  ri.renderer.begin_batch();

  engine::quad_renderer::RenderDescriptor desc;
  desc.pos_tl = { 0, 0 };
  desc.size = size;
  desc.yaw_pitch_roll_radians = { 0, 0, 0 };

  ri.renderer.draw_sprite(desc, shader);

  ri.renderer.end_batch();
  ri.renderer.flush(shader);
};

void
setup_menu_fractal_update(entt::registry& r)
{
  /*
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::menu_fractal_shader);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;

    ri.menu_fractal.bind();
    render_fullscreen_quad(r, ri.menu_fractal, ri.viewport_size_render_at);
  };
  */
}

void
setup_water_heightmap_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::water_heightmap);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);

    SINGLE_Islands island_c = SINGLE_Islands::instance;
    if (island_c.generated.empty())
      return;

    ri.water_heightmap.bind();

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      // Render exactly one quad
      {
        engine::quad_renderer::RenderDescriptor desc;

        const auto size = island_c.tilesize;
        const auto wh = island_c.wh;

        static auto pos_tl = glm::vec2{ -size * wh * 0.5f, -size * wh * 0.5f } - glm::vec2{ 0.5 * size, 0.5 * size };
        static auto pos_wh = glm::vec2{ size * wh, size * wh };

        // imgui_draw_vec2("pos_tl", pos_tl);
        // imgui_draw_vec2("pos_wh", pos_wh);

        desc.pos_tl = pos_tl;
        desc.size = pos_wh;

        ri.renderer.draw_sprite(desc, ri.water_heightmap);
      }

      ri.renderer.end_batch();
      ri.renderer.flush(ri.water_heightmap);
    }
  };
}

void
setup_water_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::water);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_t = r.get<TransformComponent>(camera_e);

    ri.water.bind();

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();
      {
        engine::quad_renderer::RenderDescriptor desc;
        const glm::vec2 offset = { ri.viewport_size_render_at.x / 2.0, ri.viewport_size_render_at.y / 2.0f };
        desc.pos_tl = glm::vec2(camera_t.position.x, camera_t.position.y) - offset;
        desc.size = ri.viewport_size_render_at;
        desc.yaw_pitch_roll_radians = { 0, 0, 0 };
        ri.renderer.draw_sprite(desc, ri.water);
      }
      ri.renderer.end_batch();
      ri.renderer.flush(ri.water);
    }
  };
};

void
setup_floor_mask_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::floor_mask);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    // const auto& camera_c = get_first_component<OrthographicCamera>(r);

    // Render floor quads in to floor-mask texture.
    engine::LinearColour mask_colour = engine::LinearColour(1.0f, 1.0f, 1.0f, 1.0f);

    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      const auto& view = r.view<const TransformComponent, const SpriteComponent, const FloorComponent>();

      for (const auto& [e, transform, sc, floor_c] : view.each()) {
        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform.position - (transform.scale * 0.5f);
        desc.size = transform.scale;
        desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                        transform.rotation_radians.y,
                                        sc.angle_radians + transform.rotation_radians.z };
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
setup_island_triangles_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::island_triangles);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;
    // const auto camera_e = get_first<OrthographicCamera>(r);
    // const auto& camera_t = r.get<const TransformComponent>(camera_e);
    // const auto& camera_c = r.get<const OrthographicCamera>(camera_e);

    // render triangles
    {
      ri.tri_renderer.reset_vert_count();
      ri.tri_renderer.begin_batch();

      auto view = r.view<const SpriteTriangleComponent, const HasParentComponent>(entt::exclude<IslandHiddenComponent>);
      for (const auto& [e, sc_c, par_c] : view.each()) {

        // todo: verify that parent is an island

        engine::tri_renderer::TriangleDescriptor desc;
        desc.point_0 = sc_c.a;
        desc.point_1 = sc_c.b;
        desc.point_2 = sc_c.c;
        desc.point_0_colour = sc_c.a_colour;
        desc.point_1_colour = sc_c.b_colour;
        desc.point_2_colour = sc_c.c_colour;
        desc.uv_0 = sc_c.uv_0;
        desc.uv_1 = sc_c.uv_1;
        desc.uv_2 = sc_c.uv_2;

        ri.tri_renderer.draw_sprite(desc, ri.instanced_tri);
      }

      ri.tri_renderer.end_batch();
      ri.tri_renderer.flush(ri.instanced_tri);
    }
  };
};

void
setup_island_triangles_gradient_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::island_triangles_gradient);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;

    ri.island_tri_gradient.bind();
    render_fullscreen_quad(r, ri.island_tri_gradient, ri.viewport_size_render_at);
  };
};

void
setup_island_hidden_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::island_hidden);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;

    // render triangles
    {
      ri.tri_renderer.reset_vert_count();
      ri.tri_renderer.begin_batch();

      auto view = r.view<const SpriteTriangleComponent, const IslandHiddenComponent>();
      for (const auto& [e, sc_c, hidden_c] : view.each()) {

        engine::tri_renderer::TriangleDescriptor desc;
        desc.point_0 = sc_c.a;
        desc.point_1 = sc_c.b;
        desc.point_2 = sc_c.c;
        desc.point_0_colour = sc_c.a_colour;
        desc.point_1_colour = sc_c.b_colour;
        desc.point_2_colour = sc_c.c_colour;
        desc.uv_0 = sc_c.uv_0;
        desc.uv_1 = sc_c.uv_1;
        desc.uv_2 = sc_c.uv_2;

        ri.tri_renderer.draw_sprite(desc, ri.island_tri_hidden);
      }

      ri.tri_renderer.end_batch();
      ri.tri_renderer.flush(ri.island_tri_hidden);
    }
  };
};

void
setup_island_above_hidden_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::island_above_hidden);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;

    // Render some quads
    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      auto view = r.view<const TransformComponent, const SpriteComponent, const AboveHiddenComponent>();

      // Render in sorted order
      for (const auto& [e, transform, sc, above_c] : view.each()) {

        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform.position - (transform.scale * 0.5f);
        desc.size = transform.scale;
        desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                        transform.rotation_radians.y,
                                        sc.angle_radians + transform.rotation_radians.z };
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
setup_island_shore_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::island_shore);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    auto& ri = SINGLE_RendererInfo::instance;

    // render triangles
    {
      ri.tri_renderer.reset_vert_count();
      ri.tri_renderer.begin_batch();

      auto view = r.view<const SpriteTriangleComponent, const IslandShoreTriangle>();
      for (const auto& [e, sc_c, shore_c] : view.each()) {

        engine::tri_renderer::TriangleDescriptor desc;
        desc.point_0 = sc_c.a;
        desc.point_1 = sc_c.b;
        desc.point_2 = sc_c.c;
        desc.point_0_colour = sc_c.a_colour;
        desc.point_1_colour = sc_c.b_colour;
        desc.point_2_colour = sc_c.c_colour;
        desc.uv_0 = sc_c.uv_0;
        desc.uv_1 = sc_c.uv_1;
        desc.uv_2 = sc_c.uv_2;

        ri.tri_renderer.draw_sprite(desc, ri.island_shore);
      }

      ri.tri_renderer.end_batch();
      ri.tri_renderer.flush(ri.island_shore);
    }
  };
};

void
setup_linear_main_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::linear_main);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    // const auto camera_e = get_first<OrthographicCamera>(r);
    // const auto& camera_t = r.get<const TransformComponent>(camera_e);
    // const auto& camera_c = r.get<const OrthographicCamera>(camera_e);

    // glEnable(GL_BLEND);
    // glEnable(GL_DEPTH_TEST);

    // set the positions of the units with circles units & update TBO
    /*
    const int N_MAX_CIRCLES = 100;
    {
      static std::vector<CircleComponent> points(N_MAX_CIRCLES);

      int i = 0;

      // draw active circles
      const auto view = r.view<const TransformComponent, CircleComponent>();
      for (const auto& [e, transform_c, circle_c] : view.each()) {
        if (i > N_MAX_CIRCLES)
          break;
        circle_c.shader_pos = get_position(r, e);
        points[i] = circle_c;
        i++;
      }

      // reset inactive circles
      for (; i < N_MAX_CIRCLES; i++)
        points[i].shader_pos = { 0, 0 };

      // Update texture
      const int num_rows = N_MAX_CIRCLES;
      const int num_cols = sizeof(game2d::CircleComponent) / sizeof(float); // floats per comp
      // glBindTexture(GL_TEXTURE_2D, ri.renderer.data.TEX);
      // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, num_cols, num_rows, GL_RGBA, GL_FLOAT, points.data());
    }
    */

    // Render some quads
    {
      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

      // Instead of using group.sort (which can be slow for large groups),
      // collect entities and sort pointers to their data, then render in order.

      // auto group = r.group<TransformComponent, SpriteComponent>();
      // // sort by z-index; adds ~0.5ms
      // group.sort([&group](const entt::entity lhs, const entt::entity rhs) {
      //   const auto& a = group.get<TransformComponent>(lhs);
      //   const auto& b = group.get<TransformComponent>(rhs);
      //   if (a.z_index != b.z_index)
      //     return a.z_index < b.z_index;
      //   // sort by eid if the layers are the same
      //   return lhs > rhs;
      // });

      // Collect entities and their z-index into a vector
      std::vector<std::tuple<int, entt::entity, const TransformComponent*, const SpriteComponent*>> sorted_entities;
      auto view = r.view<const TransformComponent, const SpriteComponent>(entt::exclude<AboveHiddenComponent>);
      sorted_entities.reserve(view.size_hint());

      for (const auto e : view) {
        const auto& transform = view.get<const TransformComponent>(e);
        const auto& sc = view.get<const SpriteComponent>(e);
        sorted_entities.emplace_back(transform.z_index, e, &transform, &sc);
      }

      // Sort by z-index, then by entity id for stable ordering
      std::sort(sorted_entities.begin(), sorted_entities.end(), [](const auto& a, const auto& b) {
        if (std::get<0>(a) != std::get<0>(b))
          return std::get<0>(a) < std::get<0>(b);
        return std::get<1>(a) > std::get<1>(b);
      });

      // Render in sorted order
      for (const auto& [z, e, transform, sc] : sorted_entities) {

        engine::quad_renderer::RenderDescriptor desc;
        desc.pos_tl = transform->position - (transform->scale * 0.5f);
        desc.size = transform->scale;
        desc.yaw_pitch_roll_radians = { transform->rotation_radians.x,
                                        transform->rotation_radians.y,
                                        sc->angle_radians + transform->rotation_radians.z };
        desc.colour = sc->colour;
        desc.tex_unit = sc->tex_unit;
        desc.sprite_offset = { sc->tex_pos.x, sc->tex_pos.y };
        desc.sprite_width = { sc->tex_pos.w, sc->tex_pos.h };
        desc.sprites_max = { sc->total_sx, sc->total_sy };

        ri.renderer.draw_sprite(desc, ri.instanced);
      }

      ri.renderer.end_batch();
      ri.renderer.flush(ri.instanced);
    }
  };
};

void
setup_sprites_to_outline_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::sprites_to_outline);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    // const auto camera_e = get_first<OrthographicCamera>(r);
    // const auto& camera_t = r.get<TransformComponent>(camera_e);
    // const auto& camera_c = r.get<OrthographicCamera>(camera_e);

    ri.renderer.reset_quad_vert_count();
    ri.renderer.begin_batch();
    const auto& view = r.view<const TransformComponent, const SpriteComponent, const SpriteOutline>();

    for (const auto& [e, transform, sc, outline_c] : view.each()) {
      engine::quad_renderer::RenderDescriptor desc;
      desc.pos_tl = transform.position - (transform.scale * 0.5f);
      desc.size = transform.scale;
      desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                      transform.rotation_radians.y,
                                      sc.angle_radians + transform.rotation_radians.z };
      desc.colour = sc.colour;
      desc.tex_unit = sc.tex_unit;
      desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
      desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
      desc.sprites_max = { sc.total_sx, sc.total_sy };

      ri.renderer.draw_sprite(desc, ri.instanced);
    }

    ri.renderer.end_batch();
    ri.renderer.flush(ri.instanced);
  };
};

void
setup_outline_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::outline);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;

    ri.outline.bind();
    render_fullscreen_quad(r, ri.outline, ri.viewport_size_render_at);
  };
};

void
setup_sprites_with_shield_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::sprites_with_shield);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;

    ri.renderer.reset_quad_vert_count();
    ri.renderer.begin_batch();
    const auto& view = r.view<const TransformComponent, const SpriteComponent, const ShieldComponent>();

    for (const auto& [e, transform, sc, shield_c] : view.each()) {
      engine::quad_renderer::RenderDescriptor desc;
      desc.pos_tl = transform.position - (transform.scale * 0.5f);
      desc.size = transform.scale;
      desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                      transform.rotation_radians.y,
                                      sc.angle_radians + transform.rotation_radians.z };
      desc.colour = sc.colour;
      desc.tex_unit = sc.tex_unit;
      desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
      desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
      desc.sprites_max = { sc.total_sx, sc.total_sy };

      ri.renderer.draw_sprite(desc, ri.instanced);
    }

    ri.renderer.end_batch();
    ri.renderer.flush(ri.instanced);
  };
};
void
setup_shine_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::shine);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;

    ri.shine.bind();

    ri.renderer.reset_quad_vert_count();
    ri.renderer.begin_batch();
    const auto& view = r.view<const TransformComponent, const SpriteComponent, const ShieldComponent>();

    for (const auto& [e, transform, sc, shield_c] : view.each()) {
      engine::quad_renderer::RenderDescriptor desc;
      desc.pos_tl = transform.position - (transform.scale * 0.5f);
      desc.size = transform.scale;
      desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                      transform.rotation_radians.y,
                                      sc.angle_radians + transform.rotation_radians.z };
      desc.colour = sc.colour;
      desc.tex_unit = sc.tex_unit;
      desc.sprite_offset = { sc.tex_pos.x, sc.tex_pos.y };
      desc.sprite_width = { sc.tex_pos.w, sc.tex_pos.h };
      desc.sprites_max = { sc.total_sx, sc.total_sy };

      ri.renderer.draw_sprite(desc, ri.shine);
    }

    ri.renderer.end_batch();
    ri.renderer.flush(ri.shine);

    // render_fullscreen_quad(r, ri.shine, ri.viewport_size_render_at);
  };
};

void
setup_flame_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::flame);
  auto& pass = ri.passes[pass_idx];
  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;

    ri.flame.bind();

    ri.renderer.reset_quad_vert_count();
    ri.renderer.begin_batch();
    const auto view = r.view<const TransformComponent, const FlamethrowerFlameComponent>();
    for (const auto& [e, transform, flame_c] : view.each()) {

      if (!flame_c.active)
        continue; // flame active determined by game logic

      engine::quad_renderer::RenderDescriptor desc;
      desc.pos_tl = transform.position - (transform.scale * 0.5f);
      desc.size = transform.scale;
      desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                      transform.rotation_radians.y,
                                      transform.rotation_radians.z };

      desc.colour = {};
      desc.tex_unit = {};
      desc.sprite_offset = {};
      desc.sprite_width = {};
      desc.sprites_max = {};

      ri.renderer.draw_sprite(desc, ri.flame);
    }

    ri.renderer.end_batch();
    ri.renderer.flush(ri.flame);
  };
}

/*
void
setup_lighting_emitters_and_occluders_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::lighting_emitters_and_occluders);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt) {
  #if defined(_DEBUG)
  ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera_c = r.get<OrthographicCamera>(camera_e);

    // emitters should be anything but black (i.e. scene lighting)
    const engine::LinearColour emitter_col = engine::SRGBToLinear({ 255, 0, 0, 255 });
    const engine::LinearColour occluder_col(0.0f, 0.0f, 0.0f, 1.0f);

    ri.lighting_emitters_and_occluders.bind();
    ri.lighting_emitters_and_occluders.set_mat4("view", camera_c.view);
    ri.lighting_emitters_and_occluders.set_mat4("projection", camera_c.projection_zoomed);

    {
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0 });
      engine::RenderCommand::clear();

      ri.renderer.reset_quad_vert_count();
      ri.renderer.begin_batch();

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

      // draw occluders
      {
        const auto& occluders = r.view<const LightOccluderComponent, const TransformComponent, const SpriteComponent>();
        for (const auto& [entity, occluder, transform, sc] : occluders.each()) {
          engine::quad_renderer::RenderDescriptor desc;
          desc.pos_tl = transform.position - transform.scale * 0.5f;
          desc.size = transform.scale;
          desc.yaw_pitch_roll_radians = { transform.rotation_radians.x,
                                          transform.rotation_radians.y,
                                          sc.angle_radians + transform.rotation_radians.z };
          desc.colour = occluder_col;
          desc.parallax = { 0, 0, 0, 0 };
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
*/

/*
void
setup_voronoi_seed_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::voronoi_seed);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&ri](entt::registry& r) {
  #if defined(_DEBUG)
  ZoneScoped;
#endif

    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    const int tex_unit_emitters_and_occluders = get_tex_unit(ri, PassName::lighting_emitters_and_occluders);

    ri.voronoi_seed.bind();

    render_fullscreen_quad(r, ri.voronoi_seed, ri.viewport_size_render_at);
  };
};
*/

/*
void
setup_jump_flood_pass(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::jump_flood);
  auto& pass = ri.passes[pass_idx];

  pass.update = [&pass](entt::registry& r) {
  #if defined(_DEBUG)
  ZoneScoped;
#endif

    auto& ri = SINGLE_RendererInfo::instance;
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    ri.jump_flood.bind();

    const auto wh = ri.viewport_size_render_at;
    const int max_dim = glm::max(wh.x, wh.y);
    const int n_jumpflood_passes = (int)(glm::ceil(glm::log(max_dim) / std::log(2.0f)));

    for (int i = 0; i < n_jumpflood_passes; i++) {
      int last_tex_idx = -1;
      int this_tex_idx = 0;
      if (i > 0) {
        last_tex_idx = (i + 1) & 1;
        this_tex_idx = last_tex_idx == 1 ? 0 : 1;
      }

      // #if defined(_DEBUG)
      //       ImGui::Text("jflood pass: %i. last: %i, this: %i", i, last_tex_idx, this_tex_idx);
      // #endif

      engine::Framebuffer::bind_fbo(pass.fbos[this_tex_idx]);
      engine::RenderCommand::set_viewport(0, 0, wh.x, wh.y);
      engine::RenderCommand::set_clear_colour_srgb({ 0, 0, 0, 0 });
      engine::RenderCommand::clear();

      // offset for each pass is half the previous one, starting at half the square resolution rounded up to nearest
      // power 2. #i.e.for 768x512 we round up to 1024x1024 and the offset for the first pass is 512x512, then 256x256,
      // etc.
      const float offset = (float)std::pow(2, n_jumpflood_passes - i - 1);

      int tex_unit = get_tex_unit(ri, PassName::voronoi_seed);
      if (i > 0) {
        tex_unit = pass.texs[last_tex_idx].tex_unit.unit;
        ri.final_jflood_texunit = pass.texs[this_tex_idx].tex_unit.unit;
      }

      ri.jump_flood.set_int("tex", tex_unit);
      ri.jump_flood.set_float("u_offset", offset);

      render_fullscreen_quad(r, ri.jump_flood, ri.viewport_size_render_at);
    }

    // #if defined(_DEBUG)
    //     ImGui::Text("final unit: %i", ri.final_jflood_texunit);
    // #endif
  };
};
*/

/*
void
setup_voronoi_distance_field_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::voronoi_distance);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt) {
  #if defined(_DEBUG)
  ZoneScoped;
#endif

    const auto& ri = SINGLE_RendererInfo::instance;
    const auto camera_e = get_first<OrthographicCamera>(r);
    const auto& camera = r.get<OrthographicCamera>(camera_e);

    const auto jflood_pass_idx = get_pass_idx(ri, PassName::jump_flood);
    const auto& jflood_pass = ri.passes[jflood_pass_idx];
    const auto& jflood_texs = jflood_pass.texs;
    const auto final_jflood_texunit = ri.final_jflood_texunit;
    const auto tex_unit = get_tex_unit(ri, PassName::lighting_emitters_and_occluders);

    ri.voronoi_distance.bind();
    ri.voronoi_distance.set_int("tex_jflood", final_jflood_texunit);
    ri.voronoi_distance.set_int("tex_emitters_and_occluders", tex_unit);

    render_fullscreen_quad(r, ri.voronoi_distance, ri.viewport_size_render_at);
  };
};
*/

void
setup_mix_lighting_and_scene_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::mix_lighting_and_scene);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif
    const auto& ri = SINGLE_RendererInfo::instance;

    engine::RenderCommand::set_clear_colour_linear({ 0, 0, 0, 0 });

    render_fullscreen_quad(r, ri.mix_lighting_and_scene, ri.viewport_size_render_at);
  };
};

void
setup_crt_effect_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::crt_effect);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt, glm::vec2 mouse_pos) {
#if defined(_DEBUG)
    ZoneScoped;
#endif

    const auto& ri = SINGLE_RendererInfo::instance;

    render_fullscreen_quad(r, ri.crt, ri.viewport_size_render_at);
  };
};

/*

void
setup_gaussian_blur_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass0_idx = get_pass_idx(ri, PassName::blur_pingpong_0);
  const auto pass1_idx = get_pass_idx(ri, PassName::blur_pingpong_1);
  auto& pass0 = ri.passes[pass0_idx];
  auto& pass1 = ri.passes[pass1_idx];

  pass0.update = [&pass0, &pass1](entt::registry& r) {
    const auto& ri = SINGLE_RendererInfo::instance;
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

  pass1.update = [](entt::registry& r, float dt) {}; // nothing, only used for fbo
};

void
setup_bloom_update(entt::registry& r)
{
  auto& ri = SINGLE_RendererInfo::instance;
  const auto pass_idx = get_pass_idx(ri, PassName::bloom);
  auto& pass = ri.passes[pass_idx];

  pass.update = [](entt::registry& r, float dt) {
    auto& ri = SINGLE_RendererInfo::instance;
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

*/

} // namespace game2d