#include "helpers.hpp"

#include "actors.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/scene/helpers.hpp"
// #include "serialize.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace game2d {

// What to serialize?
// bow
// player
// enemy
// hearth
// spawner
// turret
// camera

// what about "inbetween" state info?
// basically, any state that deviates from the start state
// of create_gameobject
// e.g. combat state
// AttackCooldownComponent
// ClosestInfo
// Attack, Range, HoldAndReleaseShoot, Burnable, TeamComponent
// Health,DamageRequests,  CreateEntityRequests
// FlashOnDamageComponent
// e.g. item/death state
// WantsToDrop
// WantsToPickup, PickupZone, Abletopickedup
// HasTargetPositionComponent, LerpToTargetComponent
// physics info?
// current collisions?
// infinitelives component
// app preferences?
// audio preferences?
// colour preferences?
// deadthings?
// animation state

// probably no to:
// cursor
// SINGLETON_gameover
// controllers
// ui
// audio?
// input events?

void
save(const entt::registry& r, const std::string& path)
{
  std::cout << "saving...\n";

  // limit snapshots to actors
  const auto v = r.view<const EntityTypeComponent, const TransformComponent>(entt::exclude<CursorComponent>);

  nlohmann::json root = nlohmann::json::array();
  for (const auto& [entity, type, transform] : v.each()) {

    nlohmann::json j; // create an empty structure

    // conversions

    nlohmann::json entity_type_component_json = type;
    j.push_back(entity_type_component_json);

    nlohmann::json transform_component_json = transform;
    j.push_back(transform_component_json);

    root.push_back(j);
  }

  // NJSONOutputArchive o;
  // entt::snapshot{ r } //
  //   .get<EntityTypeComponent>(o, v.begin(), v.end())
  //   .get<TransformComponent>(o, v.begin(), v.end());
  // o.close();

  std::string data = root.dump();
  std::cout << data << std::endl;

  // save to disk
  std::ofstream fout(path);
  fout << data.c_str();
};

void
load(entt::registry& r, const std::string& path)
{
  std::cout << "loading...\n";

  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data = buffer.str();

  // NJSONInputArchive json_in(data);
  // entt::snapshot_loader{ r } //
  //   .get<EntityTypeComponent>(json_in)
  //   .get<TransformComponent>(json_in);

  nlohmann::json root = nlohmann::json::parse(data);

  // populate the registry
  for (auto& element : root) {
    const EntityTypeComponent type = element[0].template get<EntityTypeComponent>();
    const TransformComponent transform = element[1].template get<TransformComponent>();

    CreateEntityRequest req;
    req.type = type.type;
    req.transform = transform;
    r.emplace<CreateEntityRequest>(r.create(), req);
  }
};

void
load_if_exists(entt::registry& registry, std::string path){
  //   std::ifstream file(path.c_str());
  //   if (file)
  //     load(registry, path);
};

} // namespace game2d
