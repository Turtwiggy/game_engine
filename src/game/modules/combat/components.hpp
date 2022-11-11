#pragma once

#include "engine/colour.hpp"

#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

namespace game2d {

//
// combat
//

struct AttackComponent
{
  int min_damage = 0;
  int max_damage = 0;

  AttackComponent() = default;
  AttackComponent(int min, int max)
    : min_damage(min)
    , max_damage(max){};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(AttackComponent, min_damage, max_damage);
};

struct DefenseComponent
{
  int ac = 0;

  DefenseComponent() = default;
  DefenseComponent(int ac)
    : ac(ac){};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(DefenseComponent, ac);
};

struct HealthComponent
{
  int max_hp = 10;
  int hp = 10;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(HealthComponent, max_hp, hp);
};

// struct MeleeComponent
// {
//   bool placeholder = true;
//   //   int weapon_radius = 30;
//   //   float attack_rate = 0.15f;
//   //   int use_cooldown = 1;

//   NLOHMANN_DEFINE_TYPE_INTRUSIVE(MeleeComponent, placeholder);
// };

// struct RangedComponent
// {
//   bool placeholder = true;
//   //   int projectiles = 1;
//   //   int ammo = 1;
//   //   float fire_rate = 1.0f;
//   //   bool infinite_ammo = false;
//   //   float time_between_shots = 1.0f;
//   //   float time_since_last_shot = 0.0f;
//   //   float bullet_speed = 500.0f;

//   NLOHMANN_DEFINE_TYPE_INTRUSIVE(RangedComponent, placeholder);
// };

struct RecieveDamageRequest
{
  int base_damage = 0;
  int extra_damage = 0;
  int mitigated_damage = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(RecieveDamageRequest, base_damage, extra_damage, mitigated_damage);
};

struct TakeDamageComponent
{
  std::vector<RecieveDamageRequest> damage;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TakeDamageComponent, damage);
};

// "intent" components surrounding combat
struct WantsToAttack
{
  bool placeholder = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToAttack, placeholder);
};

struct IsDead
{
  bool placeholder = true;
};

struct FlashSpriteComponent
{
  bool started = false;
  int milliseconds_left = 200;
};

// struct AreaOfEffectComponent
// {
//   int radius = 3;
// };

// struct ConfusionComponent
// {
//   float time_left = 1.0f;
// };

// struct MonsterBreedComponent
// {
//   std::string name;
//   int max_health;
//   AttackComponent attack;
//   std::vector<UseComponent> moves;
//   DropComponent loot;
// };

} // namespace game2d