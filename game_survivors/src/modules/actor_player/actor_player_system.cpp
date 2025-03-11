#include "pch.hpp"

#include "modules/actor_player/actor_player_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/controller.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/events/helpers/mouse.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

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
      const glm::vec2 l_nrm_dir = engine::normalize_safe(l_nrm_raw);

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

      const b2Vec2 vel = speed * b2Vec2{ l_nrm_dir.x, l_nrm_dir.y };
      const b2Vec2 impulse = mass * vel;
      body_c.body->ApplyLinearImpulseToCenter(impulse, true);
    }
  }

  const auto view = r.view<const PhysicsBodyComponent, const RotateToVelocityComponent>();
  for (const auto& [e, body_c, rotate_c] : view.each()) {
    // Set Rotation
    const float angle_speed = 10.0f; // higher number = faster to rotate
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
update_player_controller_system(entt::registry& r, const uint64_t milliseconds_dt, const glm::ivec2& mouse_pos)
{
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);
  int sdl_controllers_used = 0;

  const auto& view = r.view<InputComponent, TransformComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, i, t_c] : view.each()) {
    //
    i.lx = 0.0f;
    i.ly = 0.0f;
    i.rx = 0.0f;
    i.ry = 0.0f;
    i.shoot = false;
    i.sprint = false;

    // set rx based on mouse input if selected
    if (const auto* keyboard_c = r.try_get<KeyboardComponent>(e)) {

      // Add a deadzone to stop weird jitter when mouse is too close
      const auto d = glm::vec2{ mouse_pos.x, mouse_pos.y } - glm::vec2{ t_c.position.x, t_c.position.y };
      const float d2 = d.x * d.x + d.y * d.y;
      auto nrm_dir = glm::vec2(0, 0);
      if (d2 > keyboard_c->keyboard_deadzone_sqr)
        nrm_dir = engine::normalize_safe(d);

      i.rx = nrm_dir.x;
      i.ry = nrm_dir.y;
      i.ly += get_key_held(input_c, SDL_SCANCODE_W) ? -1.0f : 0.0f;
      i.ly += get_key_held(input_c, SDL_SCANCODE_S) ? 1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_A) ? -1.0f : 0.0f;
      i.lx += get_key_held(input_c, SDL_SCANCODE_D) ? 1.0f : 0.0f;
      i.shoot |= get_mouse_lmb_held();
      i.sprint |= get_key_held(input_c, SDL_SCANCODE_LSHIFT);
    }

    if (const auto* controller_c = r.try_get<SDLControllerComponent>(e)) {
      if (sdl_controllers_used < int(input_c.controllers.size())) {

        // todo: map plugged in controller idxs to player
        // todo: dont just use idx 0
        auto* controller = input_c.controllers[0];

        i.lx += get_axis_01(controller, controller_c->c_left_stick_x);
        i.ly += get_axis_01(controller, controller_c->c_left_stick_y);
      }
      // else {
      //   SDL_Log("Not enough controllers plugged in...");
      // }
      sdl_controllers_used++;
    }

    if (auto* controller_c = r.try_get<SteamControllerComponent>(e)) {

      // Handle assigned via menu
      if (controller_c->handle == 0)
        continue;

      const auto handle = controller_c->handle;
      const auto l_analog = controller_axis(r, handle, AA::LAnalogControls);
      const auto r_analog = controller_axis(r, handle, AA::RAnalogControls);
      const auto shoot = controller_button_held(steam_c, handle, DA::Game_Shoot);
      const auto sprint = controller_button_held(steam_c, handle, DA::Game_Sprint);
      i.lx = l_analog.x;
      i.ly = -l_analog.y; // flip y
      i.rx = r_analog.x;
      i.ry = -r_analog.y; // flip y
      i.shoot |= shoot;
      i.sprint |= sprint;
    }
  }
};

void
fixed_update_player_controller_system(entt::registry& r, const uint64_t ms_dt, const glm::ivec2& mouse_pos)
{
  // What happens if multiple fixedupdate() before?

  fixedupdate_movement_direct(r, ms_dt);
};

} // namespace game2d
