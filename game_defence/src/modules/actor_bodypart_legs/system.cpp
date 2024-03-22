#include "system.hpp"

#include "components.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "helpers/line.hpp"
#include "imgui.h"
#include "imgui/helpers.hpp"
#include "maths/maths.hpp"
#include "physics/components.hpp"

#include <glm/gtx/compatibility.hpp> // lerp

namespace game2d {

void
update_actor_bodypart_legs_system(entt::registry& r, const float dt, const glm::ivec2& mouse_position)
{
  // borrowed from gamemaker
  const auto lengthdir_x = [](const float dst, const float angle_rad) { return dst * glm::cos(angle_rad); };
  const auto lengthdir_y = [](const float dst, const float angle_rad) { return dst * glm::sin(angle_rad); };
  const auto ik = [](float a, float b, float c) { return glm::acos(((a * a) + (b * b) - (c * c)) / (2.0f * a * b)); };

  static bool debug_leg_actors = true;
  ImGui::Begin("Debug_ActorLegs", &debug_leg_actors);

  static float hip_to_knee_length = 13;
  static float knee_to_foot_length = 13;
  static float hip_offset_x = 4;
  static float hip_offset_y = 12;
  imgui_draw_float("hip_to_knee", hip_to_knee_length);
  imgui_draw_float("knee_to_foot_length", knee_to_foot_length);
  imgui_draw_float("hip_offset_x", hip_offset_x);
  imgui_draw_float("hip_offset_y", hip_offset_y);
  float total_length = hip_to_knee_length + knee_to_foot_length;

  static float gait_size = 4;
  static float speed = 10.5;
  static float foot_extention = 24;
  imgui_draw_float("gait_size", gait_size);
  imgui_draw_float("speed", speed);
  imgui_draw_float("foot_extention", foot_extention);

  static int hip_to_knee_width = 5;
  static int knee_to_foot_width = 2;
  imgui_draw_int("hip_to_knee_width", hip_to_knee_width);
  imgui_draw_int("knee_to_foot_width", knee_to_foot_width);

  const auto& view = r.view<LegsComponent>();
  for (const auto& [e, legs] : view.each()) {
    const auto& [body_position, body_velocity] = r.get<const AABB, const VelocityComponent>(legs.body);

    // const glm::vec2 mouse_dir_raw = body_position.center - mouse_position;
    // const auto mouse_dir_nrm = engine::normalize_safe(mouse_dir_raw);
    // const auto angle = engine::dir_to_angle_radians(mouse_dir_nrm);
    // const auto mouse_unit_x = lengthdir_x(1, angle);
    // const auto mouse_unit_y = lengthdir_y(1, angle);
    // ImGui::Text("Mouse Angle: %f deg:%f", angle, angle * engine::Rad2Deg);
    // ImGui::Text("MouseUnit: %f %f", mouse_unit_x, mouse_unit_y);

    const auto body_dir = glm::vec2(body_velocity.x, body_velocity.y);
    const float angle = engine::dir_to_angle_radians(body_dir) - engine::PI;
    // get the horizontal unit vector
    const auto unit_x = lengthdir_x(1, angle);
    // const auto unit_y = lengthdir_y(1, angle);
    const bool moving = glm::abs(body_velocity.x) > 0.0f || glm::abs(body_velocity.y) > 0.0f;

    auto& l_hip = legs.l_hip;
    auto& r_hip = legs.r_hip;
    auto& l_foot_target = legs.l_foot_target;
    auto& r_foot_target = legs.r_foot_target;

    auto l_knee = legs.l_knee;
    auto l_foot = legs.l_foot;
    auto r_knee = legs.r_knee;
    auto r_foot = legs.r_foot;

    // SET YOUR HIPS MAKE SURE THEY DONT POP
    // TODO: swap hip position dending on dir
    const float x = float(body_position.center.x);
    const float y = float(body_position.center.y);
    l_hip = { x - hip_offset_x, y + hip_offset_y };
    r_hip = { x + hip_offset_x, y + hip_offset_y };

    if (moving) {

      l_foot_target = { l_hip.x, l_hip.y + foot_extention };
      r_foot_target = { r_hip.x, r_hip.y + foot_extention };

      legs.timer += body_dir.x > 0.0f ? dt : -dt;              // walk towards direction
      const float r_legs_timer = legs.timer + engine::HALF_PI; // offset walk cycle

      l_foot.x = l_foot_target.x + gait_size * sin(-1.0f * speed * legs.timer);
      l_foot.y = l_foot_target.y + gait_size * cos(-1.0f * speed * legs.timer);
      r_foot.x = r_foot_target.x + gait_size * sin(-1.0f * speed * r_legs_timer);
      r_foot.y = r_foot_target.y + gait_size * cos(-1.0f * speed * r_legs_timer);

      const auto l_hip_to_foot_distance = glm::length(l_hip - l_foot);
      const auto l_knee_angle = ik(hip_to_knee_length, l_hip_to_foot_distance, knee_to_foot_length);
      const auto l_angle_from_hip_to_foot = engine::dir_to_angle_radians(l_hip - l_foot);
      const auto l_angle_from_hip_to_knee = l_angle_from_hip_to_foot + l_knee_angle;

      const auto r_hip_to_foot_distance = glm::length(r_hip - r_foot);
      const auto r_knee_angle = ik(hip_to_knee_length, r_hip_to_foot_distance, knee_to_foot_length);
      const auto r_angle_from_hip_to_foot = engine::dir_to_angle_radians(r_hip - r_foot);
      const auto r_angle_from_hip_to_knee = r_angle_from_hip_to_foot + r_knee_angle;

      // Calculate the knee position relative to the hip
      if (l_hip_to_foot_distance > total_length) {
        l_knee.x = l_hip.x + lengthdir_x(hip_to_knee_length, l_angle_from_hip_to_foot);
        l_knee.y = l_hip.y + lengthdir_y(hip_to_knee_length, l_angle_from_hip_to_foot);
      } else {
        l_knee.x = l_hip.x + lengthdir_x(hip_to_knee_length, l_angle_from_hip_to_knee);
        l_knee.y = l_hip.y + lengthdir_y(hip_to_knee_length, l_angle_from_hip_to_knee);
      }

      // Calculate the knee position relative to the hip
      if (r_hip_to_foot_distance > total_length) {
        r_knee.x = r_hip.x + lengthdir_x(hip_to_knee_length, r_angle_from_hip_to_foot);
        r_knee.y = r_hip.y + lengthdir_y(hip_to_knee_length, r_angle_from_hip_to_foot);
      } else {
        r_knee.x = r_hip.x + lengthdir_x(hip_to_knee_length, r_angle_from_hip_to_knee);
        r_knee.y = r_hip.y + lengthdir_y(hip_to_knee_length, r_angle_from_hip_to_knee);
      }

      // Set L knee position
      {
        // Vec from foot to hip.
        const glm::vec2 v = l_foot - l_hip;
        // Vec in Cartesian  form
        const float a = v.y;
        const float b = -v.x;
        const float c = -v.y * l_hip.x + v.x * l_hip.y;
        // Distance between the hip-to-foot-line and the knee.
        const auto& x0 = l_knee.x;
        const auto& y0 = l_knee.y;
        const float knee_dst = glm::abs(a * x0 + b * y0 + c) / glm::sqrt(a * a + b * b);
        // multiply the knee joint by this dir vector to look in the direction of the mouse
        const float intersection_angle = engine::dir_to_angle_radians(glm::ivec2(-v.y, v.x));
        const float intersection_point_x = l_knee.x + lengthdir_x(knee_dst, intersection_angle);
        const float intersection_point_y = l_knee.y + lengthdir_y(knee_dst, intersection_angle);
        l_knee.x = intersection_point_x + lengthdir_x(knee_dst * unit_x, intersection_angle);
        l_knee.y = intersection_point_y + lengthdir_y(knee_dst * unit_x, intersection_angle);
      }

      // Set R knee position
      {
        // Vec from foot to hip.
        const glm::vec2 v = r_foot - r_hip;
        // Vec in Cartesian  form
        const float a = v.y;
        const float b = -v.x;
        const float c = -v.y * r_hip.x + v.x * r_hip.y;
        // Distance between the hip-to-foot-line and the knee.
        const auto& x0 = r_knee.x;
        const auto& y0 = r_knee.y;
        const float knee_dst = glm::abs(a * x0 + b * y0 + c) / glm::sqrt(a * a + b * b);
        // multiply the knee joint by this dir vector to look in the direction of the mouse
        const float intersection_angle = engine::dir_to_angle_radians(glm::ivec2(-v.y, v.x));
        const float intersection_point_x = r_knee.x + lengthdir_x(knee_dst, intersection_angle);
        const float intersection_point_y = r_knee.y + lengthdir_y(knee_dst, intersection_angle);
        r_knee.x = intersection_point_x + lengthdir_x(knee_dst * unit_x, intersection_angle);
        r_knee.y = intersection_point_y + lengthdir_y(knee_dst * unit_x, intersection_angle);
      }

      //
      // lerp to new position
      const float lerpspeed = 0.15f;
      legs.r_knee.x = glm::lerp(legs.r_knee.x, r_knee.x, lerpspeed);
      legs.r_knee.y = glm::lerp(legs.r_knee.y, r_knee.y, lerpspeed);
      legs.l_knee.x = glm::lerp(legs.l_knee.x, l_knee.x, lerpspeed);
      legs.l_knee.y = glm::lerp(legs.l_knee.y, l_knee.y, lerpspeed);
      legs.r_foot.x = glm::lerp(legs.r_foot.x, r_foot.x, lerpspeed);
      legs.r_foot.y = glm::lerp(legs.r_foot.y, r_foot.y, lerpspeed);
      legs.l_foot.x = glm::lerp(legs.l_foot.x, l_foot.x, lerpspeed);
      legs.l_foot.y = glm::lerp(legs.l_foot.y, l_foot.y, lerpspeed);

    } else {
      // make legs stop moving
      const float lerpspeed = 0.15f;
      legs.r_knee.x = glm::lerp(legs.r_knee.x, legs.r_hip.x, lerpspeed);
      legs.r_knee.y = glm::lerp(legs.r_knee.y, legs.r_hip.y + hip_to_knee_length, lerpspeed);
      legs.l_knee.x = glm::lerp(legs.l_knee.x, legs.l_hip.x, lerpspeed);
      legs.l_knee.y = glm::lerp(legs.l_knee.y, legs.l_hip.y + hip_to_knee_length, lerpspeed);
      legs.r_foot.x = glm::lerp(legs.r_foot.x, legs.r_knee.x, lerpspeed);
      legs.r_foot.y = glm::lerp(legs.r_foot.y, legs.r_knee.y + knee_to_foot_length, lerpspeed);
      legs.l_foot.x = glm::lerp(legs.l_foot.x, legs.l_knee.x, lerpspeed);
      legs.l_foot.y = glm::lerp(legs.l_foot.y, legs.l_knee.y + knee_to_foot_length, lerpspeed);
    }

    // set_transform_with_line(r, legs.lines[3], generate_line(l_knee, { intersection_point_x, intersection_point_y }, 8));
    set_transform_with_line(r, legs.lines[0], generate_line(l_hip, l_knee, hip_to_knee_width));
    set_transform_with_line(r, legs.lines[1], generate_line(l_knee, l_foot, knee_to_foot_width));
    set_transform_with_line(r, legs.lines[2], generate_line(r_hip, r_knee, hip_to_knee_width));
    set_transform_with_line(r, legs.lines[3], generate_line(r_knee, r_foot, knee_to_foot_width));
  }

  ImGui::End();
}

} // namespace game2d
