#include "ship_draw_arcs_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/line.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/sprites/sprite_helpers.hpp"
#include "ship_draw_arcs_components.hpp"

#include "engine/imgui/helpers.hpp"

namespace game2d {

void
update_ship_draw_arcs_system(entt::registry& r)
{

  // note: in data format, 90degrees is up, 270 is down.
  // when load in, add ()()()() to convert to engine, where 90 is down, 270 is up
  const glm::vec2 tl_offset{ 22.5f, 4.5f }; // TODO: load from .json config file for hull type
  // ShipArcComponent debug_data;
  constexpr int angle = 55;
  constexpr int arc = 225;
  constexpr float half_arc_radians = (arc / 2.0f) * engine::Deg2Rad;
  const int arc_mid = engine::angle_degrees_flip_y_axis(angle);
  const float arc_mid_radians = arc_mid * engine::Deg2Rad;

  const auto& view = r.view<PlayerComponent, TransformComponent>();
  for (const auto& [e, arc_c, t_c] : view.each()) {

    // gunpoint base
    const float fwd = t_c.rotation_radians.z;
    const auto pos = glm::vec2(t_c.position.x, t_c.position.y);
    const auto size = glm::vec2{ t_c.scale.x, t_c.scale.y };
    const auto tl = pos - (0.5f * size);
    const auto rel_tl = (tl - pos) + tl_offset;
    const auto rotated_point = engine::rotate_point(rel_tl, fwd);
    const auto hardpoint_pos = pos + rotated_point;

    // directiongunpoint is facing
    const float angle_l = fwd + arc_mid_radians - half_arc_radians;
    const float angle_r = fwd + arc_mid_radians + half_arc_radians;
    auto dir = engine::angle_radians_to_direction(fwd + arc_mid_radians);
    auto dir_l = engine::angle_radians_to_direction(angle_l);
    auto dir_r = engine::angle_radians_to_direction(angle_r);

    auto arc_mid_pos = hardpoint_pos + dir * 50.0f;
    auto arc_l_pos = hardpoint_pos + dir_l * 50.0f;
    auto arc_r_pos = hardpoint_pos + dir_r * 50.0f;

    Sprite tl_s;
    tl_s.sprite = "EMPTY";
    tl_s.pos = hardpoint_pos;
    tl_s.size = { 5, 5 };
    tl_s.z_rotation = 0;
    tl_s.col.a = 1.0f * 255;
    draw_sprite(r, tl_s);

    auto draw_line = [&r](const LineInfo& l) {
      Sprite s;
      s.sprite = "EMPTY";
      s.pos = l.position;
      s.size = l.scale;
      s.z_rotation = l.rotation;
      s.col.a = 1.0f * 255;
      draw_sprite(r, s);
    };
    // draw_line(generate_line(hardpoint_pos, arc_mid_pos, 2.0f));
    draw_line(generate_line(hardpoint_pos, arc_l_pos, 2.0f));
    draw_line(generate_line(hardpoint_pos, arc_r_pos, 2.0f));

    constexpr float radius = 50;
    constexpr int segments = 16;
    constexpr float arc_angle_step = (arc * engine::Deg2Rad) / (float)segments;

    glm::vec2 prev_vert = pos + radius * glm::vec2(cos(angle_l), sin(angle_l));

    for (int i = 1; i <= segments; i++) {
      float a = angle_l + i * arc_angle_step;
      glm::vec2 cur_vert = pos + radius * glm::vec2{ cos(a), sin(a) };

      auto line = generate_line(cur_vert, prev_vert, 2.0f);
      draw_line(line);

      prev_vert = cur_vert;
    }
  }
}

} // namespace game2d