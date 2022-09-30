#pragma once

#include <SDL2/SDL_scancode.h>
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

#include <vector>

namespace game2d {

struct SINGLETON_GameOverComponent
{
  bool over = false;
};

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
  int max_hp = 100;
  int hp = 100;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(HealthComponent, max_hp, hp);
};

struct MeleeComponent
{
  bool placeholder = true;
  //   int weapon_radius = 30;
  //   float attack_rate = 0.15f;
  //   int use_cooldown = 1;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(MeleeComponent, placeholder);
};

struct RangedComponent
{
  bool placeholder = true;
  //   int projectiles = 1;
  //   int ammo = 1;
  //   float fire_rate = 1.0f;
  //   bool infinite_ammo = false;
  //   float time_between_shots = 1.0f;
  //   float time_since_last_shot = 0.0f;
  //   float bullet_speed = 500.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(RangedComponent, placeholder);
};

//
// "tag" components i.e. specific entities
//

struct PlayerComponent
{
  // movement
  int speed = 100;

  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;

  entt::entity aim_line;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerComponent, speed, W, A, S, D);
};

struct ShopKeeperComponent
{
  bool placeholder = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShopKeeperComponent, placeholder);
};

//
// AI / pathfinding
//

const int k_milliseconds_between_ai_updates = 1000;

struct AiBrainComponent
{
  int milliseconds_between_ai_updates_left = 1000;

  // NLOHMANN_DEFINE_TYPE_INTRUSIVE(AiBrainComponent);
};

struct GridComponent
{
  int x = 0;
  int y = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GridComponent, x, y);
};

struct PathfindableComponent
{
  int cost = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PathfindableComponent, cost);
};

//
// item/inventory
//

struct InBackpackComponent
{
  entt::entity parent = entt::null;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(InBackpackComponent, parent);
};

struct ConsumableComponent
{
  int uses_left = 1;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConsumableComponent, uses_left);
};

// "intent" components surrounding items
struct Use
{
  entt::entity entity = entt::null;
  std::vector<entt::entity> targets;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Use, entity, targets);
};
struct WantsToUse
{
  std::vector<Use> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToUse, items);
};
struct WantsToDrop
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToDrop, items);
};
struct WantsToPurchase
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToPurchase, items);
};
struct WantsToSell
{
  std::vector<entt::entity> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToSell, items);
};

//
// combat
//

// struct GiveDamageComponent
// {
//   int damage = 1;
//   // std::vector<int> damage_q;
// };

// struct TakeDamageComponent
// {
//   std::vector<int> damage;

//   NLOHMANN_DEFINE_TYPE_INTRUSIVE(TakeDamageComponent, damage);
// };

struct GiveHealsComponent
{
  int health = 1;
  // std::vector<int> health_q;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GiveHealsComponent, health);
};

struct TakeHealsComponent
{
  std::vector<int> heals;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TakeHealsComponent, heals);
};

// "intent" components surrounding combat
struct WantsToAttack
{
  bool placeholder = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WantsToAttack, placeholder);
};

// struct AreaOfEffectComponent
// {
//   int radius = 3;
// };

// struct ConfusionComponent
// {
//   float time_left = 1.0f;
// };

//
//
//

// struct BreedComponent
// {
//   std::string name;
//   int max_health;
//   AttackComponent attack;
//   std::vector<UseComponent> moves;
//   DropComponent loot;
// };

// enum class EquipmentSlot
// {
//   HEAD,
//   BODY,
//   HAND_L,
//   HAND_R,
//   COUNT,
// };

// struct EqippableComponent
// {
//   EquipmentSlot slot;
// };

// bool
// Gun::use(entt::registry& r, std::vector<entt::entity>& entities)
//   std::cout << "shootin..!" << "\n";
//   // Shoot()
//   // entt::entity bullet = create_bullet(r);
//   // const int BULLET_SPEED = 500;
//   // const auto& mouse_pos = input.mouse_position_in_worldspace;
//   // glm::vec2 dir = { mouse_pos.x - transform.position.x, mouse_pos.y - transform.position.y };
//   // if (dir.x != 0.0f && dir.y != 0.0f)
//   //   dir = glm::normalize(dir);
//   // auto& bullet_velocity = r.get<VelocityComponent>(bullet);
//   // bullet_velocity.x = dir.x * BULLET_SPEED;
//   // bullet_velocity.y = dir.y * BULLET_SPEED;
//   // auto& bullet_transform = r.get<TransformComponent>(bullet);
//   // bullet_transform.position = transform.position;
//   // float angle = engine::dir_to_angle_radians(dir);
//   // bullet_transform.rotation.z = angle - engine::HALF_PI;
//   // // add some randomness to the bullet's direction
//   // // todo: replace complete randomness with a recoil factor.
//   // constexpr float angle_min_max = 10.0f * engine::PI / 180.0f;
//   // const float random_angle = engine::rand_det_s(rnd.rng, -angle_min_max, angle_min_max);
//   // Create an attack ID
//   // std::cout << "bullet attack, attack id: " << a.id << "\n";
//   // Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
//   // gs.attacks.push_back(a);
//   return true;
// };

// bool
// Sword::use(entt::registry& r, std::vector<entt::entity>& entities)
// {
//   std::cout << "slashin..!" << "\n";
// s.slash_attack_time_left = s.slash_attack_time;
// s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc
// if (s.attack_left_to_right)
//   s.weapon_current_angle = keys.angle_around_player;
// else
//   s.weapon_current_angle = keys.angle_around_player;
// // set angle, but freezes weapon angle throughout slash?
// weapon.angle_radians = keys.angle_around_player;
// // remove any other slash attacks from this player
// std::vector<Attack>::iterator it = attacks.begin();
// while (it != attacks.end()) {
//   Attack& att = (*it);
//   if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == ShopItem::SHOVEL) {
//     it = attacks.erase(it);
//   } else {
//     ++it;
//   }
// }
// if (s.slash_attack_time_left > 0.0f) {
//   s.slash_attack_time_left -= delta_time_s;
//   weapon.do_render = true;
//   weapon.do_physics = true;
//   if (s.attack_left_to_right)
//     s.weapon_current_angle += s.weapon_angle_speed;
//   else
//     s.weapon_current_angle -= s.weapon_angle_speed;
//   glm::ivec2 pos = rotate_b_around_a(player_obj, weapon, s.weapon_radius, s.weapon_current_angle);
//   s.weapon_target_pos = pos;
// } else {
//   weapon.do_physics = false;
//   s.weapon_target_pos = player_obj.pos;
// }
// // Create a new slash with unique attack.id
// Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
// attacks.push_back(a);
// lerp weapon to target position
//   weapon.pos = glm::lerp(glm::vec3(weapon.pos.x, weapon.pos.y, 0.0f),
//                          glm::vec3(s.weapon_target_pos.x, s.weapon_target_pos.y, 0.0f),
//                          glm::clamp(delta_time_s * s.weapon_damping, 0.0f, 1.0f));
//   return true;
// };

// // An "Attack" is basically a limiter that prevents collisions
// // applying damage on every frame. This could end up being super weird.

} // namespace game2d