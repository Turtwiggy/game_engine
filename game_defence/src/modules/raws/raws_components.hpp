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
  std::optional<std::string> worn_on;

  friend void to_json(json& j, const Defence& val)
  {
    j["block"] = val.block;
    if (val.worn_on.has_value())
      j["worn_on"] = val.worn_on.value();
  }
  friend void from_json(const json& j, Defence& val)
  {
    j.at("block").get_to(val.block);
    if (j.contains("worn_on"))
      j.at("worn_on").get_to(val.worn_on.emplace());
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
  int damage = 0;
  std::string type;
  int range = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Ranged, damage, type, range);
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

struct Inventory
{
  int size = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Inventory, size);
};

//
// categories in the raw files...
//

struct Item
{
  std::string name;
  std::string display_name;
  std::string display_desc;
  Renderable renderable;
  std::optional<Use> use;
  std::optional<Melee> melee = std::nullopt;
  std::optional<Ranged> ranged = std::nullopt;
  std::optional<Bullet> bullet = std::nullopt;
  std::optional<Defence> defence = std::nullopt;
  std::optional<Inventory> inventory = std::nullopt;

  friend void to_json(json& j, const Item& val)
  {
    j["name"] = val.name;
    j["display_name"] = val.display_name;
    j["display_desc"] = val.display_desc;
    j["renderable"] = val.renderable;
    if (val.use.has_value())
      j["use"] = val.use.value();
    if (val.melee.has_value())
      j["melee"] = val.melee.value();
    if (val.ranged.has_value())
      j["ranged"] = val.ranged.value();
    if (val.bullet.has_value())
      j["bullet"] = val.bullet.value();
    if (val.defence.has_value())
      j["defence"] = val.defence.value();
    if (val.inventory.has_value())
      j["inventory"] = val.inventory.value();
  }
  friend void from_json(const json& j, Item& val)
  {
    j.at("name").get_to(val.name);
    if (j.contains("display_name"))
      j.at("display_name").get_to(val.display_name);
    if (j.contains("display_desc"))
      j.at("display_desc").get_to(val.display_desc);
    j.at("renderable").get_to(val.renderable);
    if (j.contains("use"))
      j.at("use").get_to(val.use.emplace());
    if (j.contains("melee"))
      j.at("melee").get_to(val.melee);
    if (j.contains("ranged"))
      j.at("ranged").get_to(val.ranged);
    if (j.contains("bullet"))
      j.at("bullet").get_to(val.bullet);
    if (j.contains("defence"))
      j.at("defence").get_to(val.defence);
    if (j.contains("inventory"))
      j.at("inventory").get_to(val.inventory);
  };
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
  bool is_sensor;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Mob, name, renderable, stats, is_sensor);
};

struct ShipParts
{
  std::string name;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShipParts, name);
};

//
//
//

struct Raws
{
  std::vector<Item> items;
  std::vector<Environment> environment;
  std::vector<Mob> mobs;
  std::vector<ShipParts> ship_parts;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Raws, items, environment, mobs, ship_parts);
};

//
//
//

Raws
load_raws(std::string path);

entt::entity
create_transform(entt::registry& r, const std::string& name);

entt::entity
spawn_item(entt::registry& r, const std::string& key);

entt::entity
spawn_mob(entt::registry& r, const std::string& key, const glm::vec2& pos);

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, const entt::entity parent);

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc);

entt::entity
spawn_floor(entt::registry& r, const std::string& key, const glm::vec2& pos, const glm::vec2& size);

} // namespace game2d