#pragma once

#include <entt/entt.hpp>

#include <vector>

namespace game2d {

//
// weapon components
//

struct AttackComponent
{
  int damage = 5;
};

struct RangeComponent
{
  int range = 1;
};

struct HoldAndReleaseShootComponent
{
  bool in_windup = false;

  // keep a track of arrows created
  // so you can map them to
  std::vector<entt::entity> my_bullets;
};

struct BurnableComponent
{
  bool placeholder = true;
};

enum class AvailableTeams
{
  enemy,
  player,
  neutral,

  count
};

struct TeamComponent
{
  AvailableTeams team = AvailableTeams::neutral;
};

// struct MagazineComponent
// {
//   bool placeholder = true;
//   // int bullets_in_clip = 6;
//   // int bullets_in_clip_left = 6;
//   // float time_between_reloads = 0.25f;
//   // float time_between_reloads_left = 0.25f;
// };

// player/entity components

struct HealthComponent
{
  int hp = 100;
};

//
// When attacking,
// create a DealDamageRequest
//

struct DealDamageRequest
{
  entt::entity from;
  entt::entity to;
};

} // namespace game2d