#pragma once

#include <SDL2/SDL_scancode.h>

namespace game2d {

struct SINGLETON_GameOverComponent
{
  bool over = false;
};

// A "potion" is able to "heal" a "health component".
// A "sword" is able to "slash" a "health component"
// A "crossbow" is able to "shoot" a "health component"
// A "player" is able to "walk".
// A "fire sword" is able to "slash" a "health component"

// A sword legs the player "attack"

// A "Player" is able to "walk".
// An player has "Health"
// An player has "Items"
// An player has "Equipment"
// A player has an item ("sword")
// A player has an item ("legs")

// An enemy has "Health" that can be "attacked"
// An enemy has "Items" that can be "Used".
// An enemy has "Equipment" so can equip "Items".
// An enemy has "Legs" so can "walk".

// Actions:
// AttackAction
// HealAction
// RestAction
// WalkAction
// UseAction

// verbs? capabilities?

struct AttackComponent
{
  int min_damage = 0;
  int max_damage = 0;

  AttackComponent() = default;
  AttackComponent(int min, int max)
    : min_damage(min)
    , max_damage(max){};
};

struct DefenseComponent
{
  int ac = 0;

  DefenseComponent() = default;
  DefenseComponent(int ac)
    : ac(ac){};
};

struct UseComponent
{
  bool placeholder = true;
  int use_cooldown = 1;
};

struct HealComponent
{
  bool placeholder = true;
};

// nouns?

struct HealthComponent
{
  int hp = 0;
};

struct ItemComponent
{
  bool placeholder = true;
};

struct EquipmentComponent
{
  bool placeholder = true;
};

struct CooldownComponent
{
  float cooldown = 0.0f;
};

// verbs?

// actors?

struct EnemyComponent
{
  bool placeholder = true;
};

struct PlayerComponent
{
  // movement
  int speed = 0;

  // input
  SDL_Scancode W = SDL_SCANCODE_W;
  SDL_Scancode A = SDL_SCANCODE_A;
  SDL_Scancode S = SDL_SCANCODE_S;
  SDL_Scancode D = SDL_SCANCODE_D;
};

struct SwordComponent
{
  bool placeholder = true;
  //   int use_cooldown = 1;
  //   int damage = 1;
  //   int weapon_radius = 30;
  //   float attack_rate = 0.15f;
};

struct FireSwordComponent
{
  bool spawns_fireballs = true;
};

struct CrossbowComponent
{
  bool placeholder = true;
  //   int use_cooldown = 1;
  //   int damage = 1;
  //   int projectiles = 1;
  //   int ammo = 1;
  //   float fire_rate = 1.0f;
  //   bool infinite_ammo = false;
  //   float time_between_shots = 1.0f;
  //   float time_since_last_shot = 0.0f;
  //   float bullet_speed = 500.0f;
};

struct BulletComponent
{
  bool placeholder = true;
};

struct ShieldComponent
{
  bool placeholder = true;
};

struct PotionComponent : public ItemComponent
{
  int heal_amount = 1;
};

void
use(entt::registry& r, entt::entity& source, std::vector<entt::entity> targets)
{
  // Potion Impl
  if (r.all_of<PotionComponent>(source)) {
    auto& potion = r.get<PotionComponent>(source);
    for (auto& entity : targets) {
      if (r.all_of<HealthComponent>(entity)) {
        auto& hp = r.get<HealthComponent>(entity);
        hp.hp += potion.heal_amount;
      }
    }
    potion.use_cooldown = 10.0f; // cooldown after using on all targets?
  }
};

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
// {
//   std::cout << "shootin..!" << std::endl;

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
//   // std::cout << "bullet attack, attack id: " << a.id << std::endl;
//   // Attack a = Attack(fire_from_this_entity.id, bullet_copy.id, ShopItem::PISTOL, s.damage);
//   // gs.attacks.push_back(a);

//   return true;
// };

// bool
// Sword::use(entt::registry& r, std::vector<entt::entity>& entities)
// {
//   std::cout << "slashin..!" << std::endl;

//   // s.slash_attack_time_left = s.slash_attack_time;
//   // s.attack_left_to_right = !s.attack_left_to_right; // keep swapping left to right to right to left etc
//   // if (s.attack_left_to_right)
//   //   s.weapon_current_angle = keys.angle_around_player;
//   // else
//   //   s.weapon_current_angle = keys.angle_around_player;
//   // // set angle, but freezes weapon angle throughout slash?
//   // weapon.angle_radians = keys.angle_around_player;
//   // // remove any other slash attacks from this player
//   // std::vector<Attack>::iterator it = attacks.begin();
//   // while (it != attacks.end()) {
//   //   Attack& att = (*it);
//   //   if (att.entity_weapon_owner_id == player_obj.id && att.weapon_type == ShopItem::SHOVEL) {
//   //     it = attacks.erase(it);
//   //   } else {
//   //     ++it;
//   //   }
//   // }

//   // if (s.slash_attack_time_left > 0.0f) {
//   //   s.slash_attack_time_left -= delta_time_s;
//   //   weapon.do_render = true;
//   //   weapon.do_physics = true;
//   //   if (s.attack_left_to_right)
//   //     s.weapon_current_angle += s.weapon_angle_speed;
//   //   else
//   //     s.weapon_current_angle -= s.weapon_angle_speed;
//   //   glm::ivec2 pos = rotate_b_around_a(player_obj, weapon, s.weapon_radius, s.weapon_current_angle);
//   //   s.weapon_target_pos = pos;
//   // } else {
//   //   weapon.do_physics = false;
//   //   s.weapon_target_pos = player_obj.pos;
//   // }

//   // // Create a new slash with unique attack.id
//   // Attack a = Attack(player_obj.id, weapon.id, ShopItem::SHOVEL, s.damage);
//   // attacks.push_back(a);

//   // lerp weapon to target position
//   //   weapon.pos = glm::lerp(glm::vec3(weapon.pos.x, weapon.pos.y, 0.0f),
//   //                          glm::vec3(s.weapon_target_pos.x, s.weapon_target_pos.y, 0.0f),
//   //                          glm::clamp(delta_time_s * s.weapon_damping, 0.0f, 1.0f));

//   return true;
// };

// bool
// Turret::use(entt::registry& r, std::vector<entt::entity>& entities)
// {
//   return true;
// };

// struct ShopItem
// {
//   std::shared_ptr<Equipment> item;
//   bool infinite_quantity = true;
//   int quantity = 0;
//   int cost = 0;
// };

// struct SINGLETON_ShopComponent
// {
//   // std::vector<ShopItem> items;
//   bool placeholder = true;
// };

// // An "Attack" is basically a limiter that prevents collisions
// // applying damage on every frame. This could end up being super weird.
// struct Attack
// {
// private:
//   static inline uint32_t global_attack_int_counter = 0;

// public:
//   uint32_t id = 0;

//   int entity_weapon_owner_id; // player or enemy
//   int entity_weapon_id;
//   ShopItem weapon_type;
//   int weapon_damage = 0;

//   Attack(int parent, int weapon, ShopItem type, int damage)
//     : entity_weapon_owner_id(parent)
//     , entity_weapon_id(weapon)
//     , weapon_type(type)
//     , weapon_damage(damage)
//   {
//     id = ++Attack::global_attack_int_counter;
//   };
// };

} // namespace game2d