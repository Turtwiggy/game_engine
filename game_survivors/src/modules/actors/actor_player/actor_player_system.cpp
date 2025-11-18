#include "pch.hpp"

#include "actor_player_helpers.hpp"
#include "engine/entt/helpers.hpp"

#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/actors/actor_player/actor_player_system.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_ability/ability_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_components.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"

namespace game2d {

// https://www.youtube.com/watch?v=LSNQuFEDOyQ
const auto exp_decay = [](float a, float b, float decay, float dt) -> float {
  //
  return b + (a - b) * glm::exp(-decay * dt);
  //
};

void
fixedupdate_movement_direct(entt::registry& r, const uint64_t ms_dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const float dt = ms_dt / 1000.0f;

  {
    const auto view =
      r.view<const InputComponent, const MovementDirectComponent, const PhysicsBodyComponent, const ActorSpeedComponent>();
    for (const auto& [e, input_c, movetype_c, body_c, speed_c] : view.each()) {

      const glm::vec2 l_raw = { input_c.lx, input_c.ly };

      // normalize values if the magnitude is above 1
      // and leave small values un-normalized
      glm::vec2 l_nrm{ l_raw.x, l_raw.y };
      const float mag = glm::length(l_raw);
      if (mag > 1.0f) {
        l_nrm.x = l_raw.x / mag;
        l_nrm.y = l_raw.y / mag;
      }

      // Apply more force the more your mass
      const float mass = b2Body_GetMass(body_c.bodyId);

      // Speed is an upgradeable stat
      // note: if you are sprinting, your current_speed is modified.
      const float speed_in_meters_per_second = speed_c.current_speed;
      float speed = speed_in_meters_per_second;

      if (auto* upgrade_c = r.try_get<StatModifierComponent>(e)) {
        const auto key_actor_speed = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_SPEED));
        speed = upgrade_c->apply_modifiers(speed_in_meters_per_second, key_actor_speed);
      }

      // use a scaled deadzone.
      const float deadzone = 0.10f;
      const float magnitude = glm::length(l_nrm);
      if (mag < deadzone)
        l_nrm = { 0, 0 };
      else {
        float scale = (magnitude - deadzone) / (1.0f - deadzone);
        l_nrm *= scale / magnitude;
      }

      const b2Vec2 tgt_vel = 100.0f * speed * b2Vec2{ l_nrm.x, l_nrm.y };
      const b2Vec2 cur_vel = b2Body_GetLinearVelocity(body_c.bodyId);

      const b2Vec2 vel_err = tgt_vel - cur_vel;
      const b2Vec2 force = 100.0f * (vel_err);
      // SDL_Log("vel_err: %f, %f, force: %f %f", vel_err.x, vel_err.y, force.x, force.y);

      // try and catch the "sudden" stops that kill all momentum
      if (glm::abs(l_nrm.x) > 0.0f || glm::abs(l_nrm.y) > 0.0f)
        b2Body_ApplyForceToCenter(body_c.bodyId, force, true);

      // if (glm::abs(l_nrm_dir.x) > 0.0f && glm::abs(l_nrm_dir.y) > 0.0f)
      // b2Body_SetLinearVelocity(body_c.bodyId, cur_vel + max_vel_change);

      // const b2Vec2 impulse = mass * speed * b2Vec2{ l_nrm_dir.x, l_nrm_dir.y };
      // body_c.body->ApplyLinearImpulseToCenter(impulse, true);
    }
  }

  const auto view = r.view<const PhysicsBodyComponent, const RotateToVelocityComponent>();
  for (const auto& [e, body_c, rotate_c] : view.each()) {
    // Set Rotation
    const float angle_speed = 50.0f; // higher number = faster to rotate
    const float max_angle = 30.0f * engine::Deg2Rad;

    const auto cur_angle = b2Rot_GetAngle(b2Body_GetRotation(body_c.bodyId));
    const float wrapped_cur_angle = engine::clamp_axis(cur_angle);

    const auto cur_vel = b2Body_GetLinearVelocity(body_c.bodyId);
    const float new_angle = engine::dir_to_angle_radians({ cur_vel.x, cur_vel.y });

    // Calculate angle diff
    float angle_diff = new_angle - wrapped_cur_angle;
    if (angle_diff > engine::PI)
      angle_diff -= engine::TWO_PI; // Take the shorter path (counterclockwise)
    else if (angle_diff < -engine::PI)
      angle_diff += engine::TWO_PI; // Take the shorter path (clockwise)

    const float clamped_angle_diff = glm::clamp(angle_diff, -max_angle, max_angle);
    const float tgt_angle = wrapped_cur_angle + clamped_angle_diff;
    const float fin_angle = exp_decay(wrapped_cur_angle, tgt_angle, angle_speed, dt);

    // SDL_Log("cur: %f tgt: %f, new: %f", cur_angle, tgt_angle, new_angle);
    b2Body_SetTransform(body_c.bodyId, b2Body_GetPosition(body_c.bodyId), b2MakeRot(fin_angle));
  }
};

void
update_player_controller_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& steam_gs_c = get_first_component<SINGLE_SteamControllerGameState>(r);
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  int sdl_controllers_used = 0;

  // reset all inputs;
  for (const auto& [e, input_c] : r.view<InputComponent>().each()) {
    input_c.lx = 0.0f;
    input_c.ly = 0.0f;
    input_c.rx = 0.0f;
    input_c.ry = 0.0f;
    input_c.select.clear();
    input_c.pause.clear();
    input_c.ability1.clear();
    input_c.ability2.clear();
    input_c.dpad_u.clear();
    input_c.dpad_d.clear();
    input_c.dpad_l.clear();
    input_c.dpad_r.clear();
    input_c.button_n.clear();
    input_c.button_s.clear();
    input_c.button_e.clear();
    input_c.button_w.clear();
  }

  // keyboards
  {
    const auto& view = r.view<InputComponent, const KeyboardComponent>();
    for (const auto& [e, i, keyboard_c] : view.each()) {
      const auto input = generate_from_keyboard(r);
      merge_inputs(i, input);
    }
  }

  // controllers via steam
  {
    const auto& view = r.view<InputComponent, const SteamControllerComponent>();
    for (const auto& [e, i, controller_c] : view.each()) {
      for (const auto handle : controller_c.handles) {

        // Handle assigned via menu
        if (handle == 0)
          continue;

        if (handle_joined_this_frame(steam_gs_c, handle))
          return; // prevent immediately doing do_ui_action

        // need to improve this...
        const auto input = generate_from_handle(r, handle);
        merge_inputs(i, input);
      }
    }
  }

  // clamp inputs
  {
    for (const auto& [e, i] : r.view<InputComponent>().each()) {
      i.lx = glm::clamp(i.lx, -1.0f, 1.0f);
      i.ly = glm::clamp(i.ly, -1.0f, 1.0f);
      i.rx = glm::clamp(i.rx, -1.0f, 1.0f);
      i.ry = glm::clamp(i.ry, -1.0f, 1.0f);
    }
  }
};

void
fixed_update_player_controller_system(entt::registry& r, const uint64_t ms_dt, const glm::ivec2& mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // What happens if multiple fixedupdate() before?

  fixedupdate_movement_direct(r, ms_dt);
};

} // namespace game2d
