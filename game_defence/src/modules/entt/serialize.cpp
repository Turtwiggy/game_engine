#include "serialize.hpp"

#include "actors.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_cursor/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/camera/components.hpp"
#include "modules/renderer/components.hpp"

#include <fstream>
#include <iostream>
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
  std::cout << "saving... " << path << std::endl;

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

  std::cout << "saved..." << std::endl;
};

void
load(entt::registry& r, const std::string& path)
{
  std::cout << "loading..." << std::endl;

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

    const auto e = create_gameplay(r, type.type);
    set_position(r, e, { transform.position.x, transform.position.y });
    set_size(r, e, { transform.scale.x, transform.scale.y });

    if (type.type == EntityType::actor_player) {
      // ??
      r.emplace<CameraFollow>(e);
    }
  }

  std::cout << "loaded..." << std::endl;
};

void
load_if_exists(entt::registry& registry, const std::string& path)
{
  std::ifstream file(path.c_str());
  if (file)
    load(registry, path);
  else
    std::cerr << "(error) file did not exist: " << path << std::endl;
};

} // namespace game2d
