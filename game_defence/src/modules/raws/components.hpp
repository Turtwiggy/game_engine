#pragma once

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/system_particles/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

namespace game2d {
using namespace nlohmann;

template<typename T>
void
to_json(json& j, const std::optional<T>& opt)
{
  if (opt.has_value()) {
    T val = opt.value();
    j = val;
  } else
    j = nullptr;
};

template<typename T>
void
from_json(const json& j, std::optional<T>& opt)
{
  if (j.is_null())
    opt = std::nullopt;
  else
    opt = j.get<T>();
};

//
// structures of data that the Raw files support
//

struct Defence
{
  int block;
  std::optional<std::string> slot;

  friend void to_json(json& j, const Defence& val)
  {
    j["block"] = val.block;
    if (val.slot.has_value())
      j["slot"] = val.slot.value();
  }
  friend void from_json(const json& j, Defence& val)
  {
    j.at("block").get_to(val.block);
    if (j.contains("slot"))
      j.at("slot").get_to(val.slot.emplace());
  };
};

struct Melee
{
  int damage = 0;
  int range = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Melee, damage, range);
};

struct Ranged
{
  std::string type;
  int range = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Ranged, type, range);
};

struct Bullet
{
  int damage = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Bullet, damage);
};

struct Renderable
{
  std::string sprite = "EMPTY";
  engine::SRGBColour colour{ 1.0f, 1.0f, 1.0f, 1.0f };

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Renderable, sprite, colour);
};

// struct Consumable
// {
//   std::unordered_map<std::string, std::string> effects;
//   NLOHMANN_DEFINE_TYPE_INTRUSIVE(Consumable, effects);
// };

struct Use
{
  std::string name;
  std::string select;

  // medkit...
  std::optional<int> amount;

  friend void to_json(json& j, const Use& val)
  {
    j["name"] = val.name;
    j["select"] = val.select;
    if (val.amount.has_value())
      j["amount"] = val.amount.value();
  }
  friend void from_json(const json& j, Use& val)
  {
    j.at("name").get_to(val.name);
    j.at("select").get_to(val.select);
    if (j.contains("amount"))
      j.at("amount").get_to(val.amount.emplace());
  };
};

struct Stats
{
  int max_hp = 100;
  int hp = 100;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Stats, max_hp, hp);
};

//
// categories in the raw files...
//

struct Item
{
  std::string name;
  Renderable renderable;
  std::optional<Use> use;

  friend void to_json(json& j, const Item& val)
  {
    j["name"] = val.name;
    j["renderable"] = val.renderable;
    if (val.use.has_value())
      j["use"] = val.use.value();
  }
  friend void from_json(const json& j, Item& val)
  {
    j.at("name").get_to(val.name);
    j.at("renderable").get_to(val.renderable);
    if (j.contains("use"))
      j.at("use").get_to(val.use.emplace());
  };
};

struct Weapon
{
  std::string name;
  Renderable renderable;
  std::optional<Melee> melee = std::nullopt;
  std::optional<Ranged> ranged = std::nullopt;
  std::optional<Bullet> bullet = std::nullopt;

  friend void to_json(json& j, const Weapon& val)
  {
    j["name"] = val.name;
    j["renderable"] = val.renderable;
    if (val.melee.has_value())
      j["melee"] = val.melee.value();
    if (val.ranged.has_value())
      j["ranged"] = val.ranged.value();
    if (val.bullet.has_value())
      j["bullet"] = val.bullet.value();
  }
  friend void from_json(const json& j, Weapon& w)
  {
    j.at("name").get_to(w.name);
    j.at("renderable").get_to(w.renderable);
    if (j.contains("melee"))
      j.at("melee").get_to(w.melee);
    if (j.contains("ranged"))
      j.at("ranged").get_to(w.ranged);
    if (j.contains("bullet"))
      j.at("bullet").get_to(w.bullet);
  };
};

struct Armour
{
  std::string name;
  Renderable renderable;
  Defence defence;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Armour, name, renderable, defence);
};

struct Environment
{
  std::string name;
  Renderable renderable;
  Defence defence;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Environment, name, renderable, defence);
};

struct Mob
{
  std::string name;
  Renderable renderable;
  Stats stats;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mob, name, renderable, stats);
};

//
//
//

struct Raws
{
  std::vector<Item> items;
  std::vector<Weapon> weapons;
  std::vector<Armour> armour;
  std::vector<Environment> environment;
  std::vector<Mob> mobs;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Raws, items, weapons, armour, environment, mobs);
};

//
//
//

Raws
load_raws(std::string path);

entt::entity
create_transform(entt::registry& r, const std::string& name);

entt::entity
spawn_item(entt::registry& r, const std::string& key, const glm::vec2& pos);

entt::entity
spawn_mob(entt::registry& r, const std::string& key, const glm::vec2& pos);

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, entt::entity& parent);

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc);

entt::entity
spawn_wall(entt::registry& r, const std::string& key, const glm::vec2& pos, const glm::vec2& size);

entt::entity
spawn_floor(entt::registry& r, const std::string& key, const glm::vec2& pos, const glm::vec2& size);

} // namespace game2d