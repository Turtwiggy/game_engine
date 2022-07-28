#pragma once

#include <SDL2/SDL_keycode.h>
#include <entt/entt.hpp>

#include <queue>

namespace game2d {

struct AsteroidComponent
{
  bool initialized = false;
  float spin_amount = 0.0f;
};

struct InputEvent
{
  SDL_Scancode key;
  bool release = false;
};

struct PlayerComponent
{
  float speed = 0.0f;

  // these events are processed at fixedinput
  std::queue<InputEvent> unprocessed_keyboard_inputs;
};

struct TurretComponent
{
  float time_between_shots = 1.0f;
  float time_since_last_shot = 0.0f;
  float bullet_speed = 500.0f;
};

entt::entity
create_camera(entt::registry& r);

entt::entity
create_hierarchy_root_node(entt::registry& r);

entt::entity
create_player(entt::registry& r);

entt::entity
create_asteroid(entt::registry& r);

entt::entity
create_bullet(entt::registry& r);

entt::entity
create_turret(entt::registry& r);

} // namespace game2d