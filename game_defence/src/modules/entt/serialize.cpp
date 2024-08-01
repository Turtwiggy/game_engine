#include "serialize.hpp"

#include "actors.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"

#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <string>

namespace game2d {

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
  fmt::println("saving... {}", path);

  // limit snapshots to actors
  const auto v =
    r.view<const EntityTypeComponent, const TransformComponent>(entt::exclude<CursorComponent, WaitForInitComponent>);

  nlohmann::json root = nlohmann::json::array();

  // Which types to save?
  // Should probably be outside this function
  std::vector<EntityType> savetypes{
    EntityType::actor_player,
    EntityType::actor_enemy_patrol,
  };

  for (const auto& [entity, type, transform] : v.each()) {

    // our type?
    const auto interest = std::find(savetypes.begin(), savetypes.end(), type.type) != savetypes.end();
    if (!interest)
      continue;

    nlohmann::json j; // create an empty structure

    // conversions

    nlohmann::json entity_type_component_json = type;
    j.push_back(entity_type_component_json);

    nlohmann::json transform_component_json = transform;
    j.push_back(transform_component_json);

    // if (const auto* patrol_c = r.try_get<PatrolComponent>(entity)) {
    //   const PatrolComponent& pc = (*patrol_c);
    //   nlohmann::json patrol_component_json = pc;
    //   j.push_back(patrol_component_json);
    // }

    root.push_back(j);
  }

  // NJSONOutputArchive o;
  // entt::snapshot{ r } //
  //   .get<EntityTypeComponent>(o, v.begin(), v.end())
  //   .get<TransformComponent>(o, v.begin(), v.end());
  // o.close();

  std::string data = root.dump();
  // std::cout << data << std::endl;

  // save to disk
  std::ofstream fout(path);
  fout << data;

  fmt::println("saved...");
};

void
load(entt::registry& r, const std::string& path)
{
  fmt::println("loading...");

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

    const auto e = create_gameplay(r, type.type, { transform.position.x, transform.position.y });
    set_size(r, e, { transform.scale.x, transform.scale.y });

    // if (type.type == EntityType::actor_enemy_patrol) {
    //   const PatrolComponent patrol = element[2].template get<PatrolComponent>();
    //   r.emplace_or_replace<PatrolComponent>(e, patrol);
    // }

    // if (type.type == EntityType::actor_player) {
    //   const PatrolComponent patrol = element[2].template get<PatrolComponent>();
    //   r.emplace_or_replace<PatrolComponent>(e, patrol);
    //   r.emplace<CameraFollow>(e);
    // }
  }

  fmt::println("loaded...");
};

void
load_if_exists(entt::registry& registry, const std::string& path)
{
  std::ifstream file(path.c_str());
  if (file)
    load(registry, path);
  else
    fmt::println("error, file did not exist: {}", path);
};

} // namespace game2d
