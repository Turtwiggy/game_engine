#pragma once

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_traits/trait_components.hpp"

#include <box2d/b2_fixture.h>
#include <entt/fwd.hpp>
#include <glm/fwd.hpp>
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
  float max_hp = 100;
  float hp = 100;

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

//
// categories in the raw files...
//

struct ItemKey
{
  std::string key;
};

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
  std::optional<std::vector<TraitOnDisk>> traits = std::nullopt;
  std::optional<Inventory> inventory = std::nullopt;
  std::optional<PhysicsBodyDef> phys_body = std::nullopt;
  std::optional<std::vector<PhysicsFixtureDef>> phys_fixtures = std::nullopt;
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
                                              phys_body,
                                              phys_fixtures);
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

const int default_size = 32;

Raws
load_raws(std::string path);

std::vector<std::string>
get_raws_keys(const Raws& raws);

const Item
find_item(entt::registry& r, std::string key);

entt::entity
create_transform(entt::registry& r, const std::string& name);

b2Fixture*
create_fixture(b2Body* body, const PhysicsFixtureDef& fix, b2Vec2 size_in_meters);

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
spawn_particle_emitter(entt::registry& r, const RequestToSpawnParticles& req);

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc);

} // namespace game2d