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
#include "modules/items/components.hpp"
#include "modules/physics/components.hpp"
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

  const float player_speed = 10.0f;
  const float bullet_speed = 10.0f;
  const float gun_offset_distance = 30.0f;

  // player movement
  const auto& view = r.view<PlayerComponent, InputComponent, const PhysicsActorComponent, const TransformComponent>();
  for (auto [entity, player, input, actor, t] : view.each()) {

    auto* keyboard = r.try_get<KeyboardComponent>(entity);
    auto* controller = r.try_get<ControllerComponent>(entity);

    float& lx = input.stick_l.x;
    float& ly = input.stick_l.y;
    float& rx = input.stick_r.x;
    float& ry = input.stick_r.y;

    bool left_bumper_pressed = false;
    bool right_bumper_pressed = false;
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
      right_bumper_pressed = get_button_held(c, controller->c_r_bumper);
    };

    const glm::vec2 r_analog_dir = { rx, ry };
    glm::vec2 r_nrm_dir = r_analog_dir;
    if (r_nrm_dir.x != 0.0f || r_nrm_dir.y != 0.0f)
      r_nrm_dir = glm::normalize(r_nrm_dir);

    const glm::vec2 l_analog_dir = { lx, ly };
    glm::vec2 l_nrm_dir = l_analog_dir;
    if (l_nrm_dir.x != 0.0f || l_nrm_dir.y != 0.0f)
      l_nrm_dir = glm::normalize(l_nrm_dir);

    // do the move
    if (auto* vel = r.try_get<VelocityComponent>(entity)) {
      const glm::vec2 move_dir = l_nrm_dir * player_speed;
      vel->x = move_dir.x;
      vel->y = move_dir.y;
    }

    // offset the bullet by a distance
    // to stop the bullet spawning inside the entity
    glm::ivec2 offset_pos = { t.position.x + r_nrm_dir.x * gun_offset_distance,
                              t.position.y + r_nrm_dir.y * gun_offset_distance };

    // visually display the gun angle
    // note: this should be in update() not fixedupdate()
    if (glm::abs(r_nrm_dir.x) + glm::abs(r_nrm_dir.y) > 0.001f) {
      auto& debug_gunspot_transform = r.emplace_or_replace<TransformComponent>(player.debug_gun_spot);
      debug_gunspot_transform.position.x = offset_pos.x;
      debug_gunspot_transform.position.y = offset_pos.y;
      debug_gunspot_transform.scale.x = 5.0f;
      debug_gunspot_transform.scale.y = 5.0f;
    } else {
      if (auto* gun_transform = r.try_get<TransformComponent>(player.debug_gun_spot))
        r.remove<TransformComponent>(player.debug_gun_spot);
    }

    //
    // request to shoot
    //
    {
      if (player.time_between_bullets_left > 0.0f)
        player.time_between_bullets_left -= dt;
      if (lmb_press && player.time_between_bullets_left <= 0.0f) {

        CreateEntityRequest req;
        req.type = EntityType::actor_bullet;
        req.position = { offset_pos.x, offset_pos.y, 0 };
        req.velocity = glm::vec3(r_nrm_dir.x * bullet_speed, r_nrm_dir.y * bullet_speed, 0);
        r.emplace<CreateEntityRequest>(r.create(), req);

        // request audio
        // r.emplace<AudioRequestPlayEvent>(r.create(), "SHOOT_01");

        // reset timer
        player.time_between_bullets_left = time_between_bullets;
      }
    }

    // pickup objects?
    {
      bool pickup_objects_pressed = false;
      bool e_pressed = std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
                         return e.type == InputType::keyboard && e.key == SDL_SCANCODE_E && e.state == InputState::press;
                       }) != std::end(inputs);
      pickup_objects_pressed = e_pressed;             // keyboard
      pickup_objects_pressed |= right_bumper_pressed; // controller
      if (pickup_objects_pressed) {
        std::cout << "Pickup pressed!\n";
        r.emplace_or_replace<WantsToPickUp>(entity);
      }
    }

    //
    // hack: place turret?
    // bug: turrets_placed not reset on scene reset
    //
    static int turrets_placed = 0;
    bool place_turret_pressed = false;
    bool space_pressed =
      std::find_if(inputs.begin(), inputs.end(), [](const InputEvent& e) {
        return e.type == InputType::keyboard && e.key == SDL_SCANCODE_SPACE && e.state == InputState::press;
      }) != std::end(inputs);

    place_turret_pressed |= space_pressed; // keyboard
    if (place_turret_pressed && turrets_placed < 4) {
      turrets_placed++;
      CreateEntityRequest req;
      req.type = EntityType::actor_turret;
      req.position = t.position;
      r.emplace<CreateEntityRequest>(r.create(), req);
    }
  }
};
