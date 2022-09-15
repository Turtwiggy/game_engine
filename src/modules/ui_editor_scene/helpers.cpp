#include "helpers.hpp"

#include "serialize.hpp"

#include "game/components/components.hpp"
#include "game/entities/actors.hpp"
#include "modules/events/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/sprites/components.hpp"
#include "modules/ui_editor_tilemap/components.hpp"
#include "modules/ui_hierarchy/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"

#include "magic_enum.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace game2d {

void
save(const entt::registry& registry, std::string path)
{
  std::cout << "saving...\n";

  // convert entt registry to string(or bson)
  NJSONOutputArchive json_archive;
  entt::basic_snapshot{ registry }
    .entities(json_archive)
    .component<
      // editor
      TilemapComponent,
      // game/renderer
      TagComponent,
      EntityHierarchyComponent,
      RootNode,
      TransformComponent,
      SpriteComponent
      // physics
      // PhysicsActorComponent,
      // PhysicsSolidComponent,
      // PhysicsSizeComponent,
      // VelocityComponent,
      // gameplay
      // AttackComponent,
      // DefenseComponent,
      // HealthComponent,
      // MeleeComponent,
      // RangedComponent,
      // PlayerComponent,
      // ShopKeeperComponent,
      // InBackpackComponent,
      // ConsumableComponent,
      // WantsToUse,
      // WantsToDrop,
      // WantsToPurchase,
      // WantsToSell,
      // TakeDamageComponent,
      // GiveHealsComponent,
      // TakeHealsComponent,
      // WantsToAttack
      //
      >(json_archive);
  json_archive.close();

  std::string data = json_archive.as_string();

  // save to disk
  std::ofstream fout(path);
  fout << data.c_str();
};

template<class T>
void
ctx_reset(entt::registry& r)
{
  if (r.ctx().contains<T>())
    r.ctx().erase<T>();
  r.ctx().emplace<T>();
};

void
load(entt::registry& r, std::string path)
{
  std::cout << "loading...\n";
  r.each([&r](auto entity) { r.destroy(entity); });

  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data = buffer.str();

  // convert string (or bson) to entt registry
  auto& registry_to_load_in_to = r;

  NJSONInputArchive json_in(data);
  entt::basic_snapshot_loader{ registry_to_load_in_to }
    .entities(json_in)
    .component<
      // editor
      TilemapComponent,
      // game/renderer
      TagComponent,
      EntityHierarchyComponent,
      RootNode,
      TransformComponent,
      SpriteComponent
      // physics
      // PhysicsActorComponent,
      // PhysicsSolidComponent,
      // PhysicsSizeComponent,
      // VelocityComponent,
      // gameplay
      // AttackComponent,
      // DefenseComponent,
      // HealthComponent,
      // MeleeComponent,
      // RangedComponent,
      // PlayerComponent,
      // ShopKeeperComponent,
      // InBackpackComponent,
      // ConsumableComponent,
      // WantsToUse,
      // WantsToDrop,
      // WantsToPurchase,
      // WantsToSell,
      // TakeDamageComponent,
      // GiveHealsComponent,
      // TakeHealsComponent,
      // WantsToAttack
      //
      >(json_in);

  // initialize game
  ctx_reset<SINGLETON_PhysicsComponent>(r);
  ctx_reset<SINGLETON_GameOverComponent>(r);
  ctx_reset<SINGLETON_EntityBinComponent>(r);
  ctx_reset<SINGLETON_FixedUpdateInputHistory>(r);
  // reset editor tools?
  // ctx_reset<SINGLETON_TilemapComponent>(r);

  const auto& view = r.view<const TagComponent>();
  view.each([&r](auto entity, const TagComponent& tag) {
    //
    EntityType value = magic_enum::enum_cast<EntityType>(tag.tag).value();
    create_gameplay(r, entity, value);
    // create_renderable(r, e, value);
  });
};

void
load_if_exists(entt::registry& registry, std::string path){
  //   std::ifstream file(path.c_str());
  //   if (file)
  //     load(registry, path);
};

} // namespace game2d