#include "system.hpp"

#include "actors.hpp"
#include "audio/components.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/controller.hpp"
#include "events/helpers/mouse.hpp"
#include "helpers/line.hpp"
#include "lifecycle/components.hpp"
#include "modules/physics_box2d/components.hpp"
#include "modules/player/components.hpp"
#include "modules/turret/components.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>
#include <glm/glm.hpp>

void
game2d::update_player_controller_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  const auto& finputs = get_first_component<SINGLETON_FixedUpdateInputHistory>(r);
  const auto& inputs = finputs.history.at(finputs.fixed_tick);
  const auto& i = get_first_component<SINGLETON_InputComponent>(r);
  const auto dt = milliseconds_dt / 1000.0f;
  const float time_between_bullets = 0.25f;

  // player movement
  const auto& view = r.view<PlayerComponent, const ActorComponent, const TransformComponent, InputComponent>();
  for (auto [entity, player, actor, t, input] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    float& lx = input.stick_l.x;
    float& ly = input.stick_l.y;
    float& rx = input.stick_r.x;
    float& ry = input.stick_r.y;

    bool lmb_press = false;
    lmb_press |= get_mouse_lmb_press();

    if (keyboard) {
      for (const InputEvent& i : inputs) {
        auto held = i.state == InputState::held;
        auto press = i.state == InputState::press;
        auto release = i.state == InputState::release;
        if (i.key == keyboard->W && (held || press))
          ly = -1;
        if (i.key == keyboard->S && (held || press))
          ly = 1;
        if (i.key == keyboard->A && (held || press))
          lx = -1;
        if (i.key == keyboard->D && (held || press))
          lx = 1;
        if ((i.key == keyboard->W || i.key == keyboard->S) && release)
          ly = 0;
        if ((i.key == keyboard->A || i.key == keyboard->D) && release)
          lx = 0;
      }
    }

    // hack: should used the fixed-input inputs
    if (controller && i.controllers.size() > 0) {
      SDL_GameController* c = i.controllers[0];
      lx = get_axis_01(c, controller->c_left_stick_x);
      ly = get_axis_01(c, controller->c_left_stick_y);
      rx = get_axis_01(c, controller->c_right_stick_x);
      ry = get_axis_01(c, controller->c_right_stick_y);
      lmb_press = get_axis_01(c, controller->c_right_trigger) > 0.7f;
    };

    // do the move
    const float speed = 500.0f;
    const b2Vec2 move_dir = { lx * speed, ly * speed };
    actor.body->SetLinearVelocity(move_dir);

    // do a shoot
    const float bullet_speed = 200.0f;

    // work out analogue direction
    const glm::vec2 analog_dir = { rx, ry };
    glm::vec2 nrm_dir = analog_dir;
    if (analog_dir.x != 0.0f || analog_dir.y != 0.0f)
      nrm_dir = glm::normalize(analog_dir);

    // offset the bullet by a distance
    // to stop the bullet spawning inside the entity
    glm::ivec2 offset_pos = { t.position.x + nrm_dir.x * 30.0f, t.position.y + nrm_dir.y * 30.0f };

    // visually display the gun angle
    // note: this should be in update() not fixedupdate()
    if (glm::abs(nrm_dir.x) + glm::abs(nrm_dir.y) > 0.001f) {
      auto& debug_gunspot_transform = r.emplace_or_replace<TransformComponent>(player.debug_gun_spot);
      debug_gunspot_transform.position.x = offset_pos.x;
      debug_gunspot_transform.position.y = offset_pos.y;
      debug_gunspot_transform.scale.x = 5.0f;
      debug_gunspot_transform.scale.y = 5.0f;
    } else {
      if (auto* gun_transform = r.try_get<TransformComponent>(player.debug_gun_spot))
        r.remove<TransformComponent>(player.debug_gun_spot);
    }

    if (player.time_between_bullets_left > 0.0f)
      player.time_between_bullets_left -= dt;
    if (lmb_press && player.time_between_bullets_left <= 0.0f) {
      const auto& player_velocity = actor.body->GetLinearVelocity();

      // create a bullet
      CreateEntityRequest req;
      req.type = EntityType::actor_bullet;
      req.position = { offset_pos.x, offset_pos.y, 0 };
      req.velocity = glm::ivec3(
        //
        nrm_dir.x * bullet_speed,
        nrm_dir.y * bullet_speed,
        0);
      r.emplace<CreateEntityRequest>(r.create(), req);

      // request sound
      r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_01");

      // reset timer
      player.time_between_bullets_left = time_between_bullets;
    }
  }
};
