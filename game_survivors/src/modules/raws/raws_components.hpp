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
  int block = 0;
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

struct Combat
{
  int damage = 0;
  int range = 0;
  std::string type;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Combat, damage, range, type);
};

struct Bullet
{
  int damage = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Bullet, damage);
};

struct Renderable
{
  std::string sprite = "EMPTY";
  std::string colour = "white";

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Renderable, sprite, colour);
};

struct Use
{
  std::string name;
  std::string select;

  // medkit...
  std::optional<int> amount;
  std::optional<int> uses;

  friend void to_json(json& j, const Use& val)
  {
    j["name"] = val.name;
    j["select"] = val.select;
    if (val.amount.has_value())
      j["amount"] = val.amount.value();
    if (val.uses.has_value())
      j["uses"] = val.amount.value();
  }
  friend void from_json(const json& j, Use& val)
  {
    j.at("name").get_to(val.name);
    j.at("select").get_to(val.select);
    if (j.contains("amount"))
      j.at("amount").get_to(val.amount.emplace());
    if (j.contains("uses"))
      j.at("uses").get_to(val.uses.emplace());
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

struct MoveSpeed
{
  int speed = 1;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(MoveSpeed, speed);
};

struct Trait
{
  std::string key;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Trait, key);
};

struct PhysicsDesc
{
  bool is_sensor = false;
  std::optional<bool> is_static = std::nullopt;

  friend void to_json(nlohmann ::json& j, const PhysicsDesc& val)
  {
    j["is_sensor"] = val.is_sensor;
    if (j.contains("is_static"))
      j["is_static"] = val.is_static.value();
  }
  friend void from_json(const nlohmann ::json& j, PhysicsDesc& val)
  {
    j.at("is_sensor").get_to(val.is_sensor);
    if (j.contains("is_static"))
      j.at("is_static").get_to(val.is_static.emplace());
  };
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
  std::optional<Stats> stats = std::nullopt;
  std::optional<Use> use = std::nullopt;
  std::optional<Defence> defence = std::nullopt;
  std::optional<Combat> combat = std::nullopt;
  std::optional<std::vector<Trait>> traits = std::nullopt;
  std::optional<Inventory> inventory = std::nullopt;
  std::optional<PhysicsDesc> physics_desc = std::nullopt;
  // std::optional<Bullet> bullet = std::nullopt;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Item,
                                              name,
                                              display_name,
                                              display_desc,
                                              renderable,
                                              stats,
                                              use,
                                              defence,
                                              combat,
                                              traits,
                                              inventory,
                                              physics_desc);
};

struct Colour
{
  std::string hex;
  std::string tag;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Colour, hex, tag);
};

//
//

struct Raws
{
  std::vector<Colour> colours;
  std::vector<Item> items;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Raws, colours, items);
};

//
//

const int default_size = 32;

Raws
load_raws(std::string path);

std::vector<std::string>
get_raws_keys(const Raws& raws);

entt::entity
create_transform(entt::registry& r, const std::string& name);

void
give_life(entt::registry& r,
          const entt::entity e,
          const glm::vec2& pos,
          const glm::vec2& size = { default_size, default_size });

void
remove_life(entt::registry& r, const entt::entity e);

entt::entity
spawn(entt::registry& r, const std::string& key);

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, const entt::entity parent);

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc);

} // namespace game2d