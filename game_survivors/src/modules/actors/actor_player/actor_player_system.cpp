#include "pch.hpp"

#include "actor_player_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/actor_player_system.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/systems/system_ability/ability_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"
#include "modules/ui/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_helpers.hpp"

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
  const float dt = ms_dt / 1000.0f;

  {
    const auto& view =
      r.view<const InputComponent, const MovementDirectComponent, PhysicsBodyComponent, const ActorSpeedComponent>();
    for (const auto& [e, input_c, movetype_c, body_c, speed_c] : view.each()) {

      const glm::vec2 l_nrm_raw = { input_c.lx, input_c.ly };
      glm::vec2 l_nrm_dir = l_nrm_raw; // not normalized

      // Apply more force the more your mass
      const float mass = body_c.body->GetMass();

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
      const float magnitude = glm::length(l_nrm_dir);
      if (magnitude < deadzone)
        l_nrm_dir = { 0, 0 };
      else {
        // scale input from deadzone to full range
        // const float x = engine::scale(l_nrm_dir.x, deadzone, 1.0f, 0.0f, 1.0f);
        // const float y = engine::scale(l_nrm_dir.y, deadzone, 1.0f, 0.0f, 1.0f);

        float scale = (magnitude - deadzone) / (1.0f - deadzone);
        l_nrm_dir *= scale / magnitude;
      }

      const b2Vec2 tgt_vel = 100.0f * speed * b2Vec2{ l_nrm_dir.x, l_nrm_dir.y };
      const b2Vec2 cur_vel = body_c.body->GetLinearVelocity();

      const b2Vec2 vel_err = tgt_vel - cur_vel;
      const b2Vec2 force = 100.0f * (vel_err);
      // SDL_Log("vel_err: %f, %f, force: %f %f", vel_err.x, vel_err.y, force.x, force.y);

      // try and catch the "sudden" stops that kill all momentum
      if (glm::abs(l_nrm_dir.x) > 0.0f && glm::abs(l_nrm_dir.y) > 0.0f)
        body_c.body->ApplyForceToCenter(force, true);

      // if (glm::abs(l_nrm_dir.x) > 0.0f && glm::abs(l_nrm_dir.y) > 0.0f)
      // body_c.body->SetLinearVelocity(cur_vel + max_vel_change);

      // const b2Vec2 impulse = mass * speed * b2Vec2{ l_nrm_dir.x, l_nrm_dir.y };
      // body_c.body->ApplyLinearImpulseToCenter(impulse, true);
    }
  }

  const auto view =
    r.view<const PhysicsBodyComponent, const RotateToVelocityComponent>(entt::exclude<LockedInSpotComponent>);
  for (const auto& [e, body_c, rotate_c] : view.each()) {
    // Set Rotation
    const float angle_speed = 50.0f; // higher number = faster to rotate
    const float max_angle = 30.0f * engine::Deg2Rad;

    const auto cur_angle = body_c.body->GetAngle();
    const float wrapped_cur_angle = engine::clamp_axis(cur_angle);

    const auto cur_vel = body_c.body->GetLinearVelocity();
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

    body_c.body->SetTransform(body_c.body->GetPosition(), fin_angle);
  }
};

void
update_player_controller_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& steam_gs_c = get_first_component<SINGLE_SteamControllerGameState>(r);
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  int sdl_controllers_used = 0;

  const auto& view = r.view<InputComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, i] : view.each()) {
    i = {}; // reset all inputs every frame

    // set rx based on mouse input if selected
    if (const auto* keyboard_c = r.try_get<KeyboardComponent>(e)) {

      // Add a deadzone to stop weird jitter when mouse is too close
      // const auto d = glm::vec2{ mouse_pos.x, mouse_pos.y } - glm::vec2{ t_c.position.x, t_c.position.y };
      // const float d2 = d.x * d.x + d.y * d.y;
      // auto nrm_dir = glm::vec2(0, 0);
      // if (d2 > keyboard_c->keyboard_deadzone_sqr)
      //   nrm_dir = engine::normalize_safe(d);
      // i.rx = nrm_dir.x;
      // i.ry = nrm_dir.y;

      i.ly += get_key_held(input_c, SDL_SCANCODE_W) ? -1.0f : 0.0f;
      i.ly += get_key_held(input_c, SDL_SCANCODE_S) ? 1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_A) ? -1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_D) ? 1.0f : 0.0f;

      if (get_mouse_lmb_press())
        i.ability1.push_back(ActionStateEnum::DOWN);
      if (get_mouse_lmb_held())
        i.ability1.push_back(ActionStateEnum::HELD);
      if (get_mouse_lmb_release())
        i.ability1.push_back(ActionStateEnum::RELEASE);
      if (get_mouse_rmb_press())
        i.ability2.push_back(ActionStateEnum::DOWN);
      if (get_mouse_rmb_held())
        i.ability2.push_back(ActionStateEnum::HELD);
      if (get_mouse_rmb_release())
        i.ability2.push_back(ActionStateEnum::RELEASE);

      auto generate_actions_from_keyboard = [&input_c](std::vector<ActionStateEnum>& acts, const SDL_Scancode key) {
        if (get_key_down(input_c, key))
          acts.push_back(ActionStateEnum::DOWN);
        if (get_key_held(input_c, key))
          acts.push_back(ActionStateEnum::HELD);
        if (get_key_up(input_c, key))
          acts.push_back(ActionStateEnum::RELEASE);
      };
      generate_actions_from_keyboard(i.pause, SDL_SCANCODE_ESCAPE);
      generate_actions_from_keyboard(i.dpad_u, SDL_SCANCODE_UP);
      generate_actions_from_keyboard(i.dpad_d, SDL_SCANCODE_DOWN);
      generate_actions_from_keyboard(i.dpad_l, SDL_SCANCODE_LEFT);
      generate_actions_from_keyboard(i.dpad_r, SDL_SCANCODE_RIGHT);
      // generate_actions_from_keyboard(i.button_n, SDL_SCANCODE_UP);
      // generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_DOWN);
      // generate_actions_from_keyboard(i.button_e, SDL_SCANCODE_LEFT);
      // generate_actions_from_keyboard(i.button_w, SDL_SCANCODE_RIGHT);
      generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_KP_ENTER);
      generate_actions_from_keyboard(i.button_s, SDL_SCANCODE_RETURN);
    }

    if (auto* sdl_controller_c = r.try_get<SDLControllerComponent>(e)) {
      // ... not impl
    }

    if (auto* controller_c = r.try_get<SteamControllerComponent>(e)) {

      for (const auto handle : controller_c->handles) {

        // Handle assigned via menu
        if (handle == 0)
          continue;

        if (handle_joined_this_frame(steam_gs_c, handle))
          return; // prevent immediately doing do_ui_action

        // need to improve this...
        const auto input = generate_from_handle(r, handle);
        i.pause.insert(i.pause.end(), input.pause.begin(), input.pause.end());
        i.ability1.insert(i.ability1.end(), input.ability1.begin(), input.ability1.end());
        i.ability2.insert(i.ability2.end(), input.ability2.begin(), input.ability2.end());
        i.dpad_u.insert(i.dpad_u.end(), input.dpad_u.begin(), input.dpad_u.end());
        i.dpad_d.insert(i.dpad_d.end(), input.dpad_d.begin(), input.dpad_d.end());
        i.dpad_l.insert(i.dpad_l.end(), input.dpad_l.begin(), input.dpad_l.end());
        i.dpad_r.insert(i.dpad_r.end(), input.dpad_r.begin(), input.dpad_r.end());
        i.button_n.insert(i.button_n.end(), input.button_n.begin(), input.button_n.end());
        i.button_s.insert(i.button_s.end(), input.button_s.begin(), input.button_s.end());
        i.button_e.insert(i.button_e.end(), input.button_e.begin(), input.button_e.end());
        i.button_w.insert(i.button_w.end(), input.button_w.begin(), input.button_w.end());
      }
    }

    i.lx = glm::clamp(i.lx, -1.0f, 1.0f);
    i.ly = glm::clamp(i.ly, -1.0f, 1.0f);
    i.rx = glm::clamp(i.rx, -1.0f, 1.0f);
    i.ry = glm::clamp(i.ry, -1.0f, 1.0f);
  }
};

void
fixed_update_player_controller_system(entt::registry& r, const uint64_t ms_dt, const glm::ivec2& mouse_pos)
{
  // What happens if multiple fixedupdate() before?

  fixedupdate_movement_direct(r, ms_dt);
};

} // namespace game2d
