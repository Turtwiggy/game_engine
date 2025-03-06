// header
#include "lifecycle_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include <unordered_set>

#if defined(_MSC_VER)
#include <optick.h>
#endif

namespace game2d {

void
update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
#if defined(_MSC_VER)
  OPTICK_EVENT();
#endif

  const auto& physics_c = get_first_component<SINGLE_Physics>(r);
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>(entt::exclude<WaitForInitComponent>);
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);

    lifecycle.milliseconds_alive += static_cast<int>(milliseconds_dt);
  });

  // Death callbacks.
  // OnDeathCallbacks can cause more dead.dead entities (explosions)
  std::unordered_set<entt::entity> uniquely_dead;
  while (!dead.dead.empty()) {
    const auto e = dead.dead.front();
    dead.dead.pop();

    // Skip entities that are already in uniquely_dead
    if (uniquely_dead.find(e) != uniquely_dead.end())
      continue;
    uniquely_dead.emplace(e);

    // do callbacks.
    if (auto* callback = r.try_get<OnDeathCallbacks>(e)) {
      for (const auto& cb : callback->callbacks)
        cb(r, e);
    }
  }

  for (const auto e : uniquely_dead) {

    // A destroyed fixture belonging to a body;
    // destroy the parent when the fixture dies.
    if (const auto* fixture_c = r.try_get<PhysicsFixtureComponent>(e))
      uniquely_dead.emplace(r.get<HasParentComponent>(e).parent);

    // A destroyed parent might have fixtures;
    // destroy the fixtures when the parent dies.
    if (const auto* body_c = r.try_get<PhysicsBodyComponent>(e))
      uniquely_dead.insert(body_c->fixtures.begin(), body_c->fixtures.end());

    // Destroy all the children
    if (auto* children_c = r.try_get<HasChildrenComponent>(e))
      uniquely_dead.insert(children_c->children.begin(), children_c->children.end());
  }

  for (const auto e : uniquely_dead) {

    // Update physics
    if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
      physics_c.world->DestroyBody(pb->body);

    // Update entt
    if (r.valid(e))
      r.destroy(e);
  }

  // Check invalid entities...
  const auto& storage = r.storage<entt::entity>();
  for (const std::tuple<entt::entity>& ent_tuple : storage.each()) {
    const auto& [e] = ent_tuple;

    if (!r.valid(e)) {
      SDL_Log("Warning: removing invalid entity. How did it occur?");
      r.destroy((e));
    }

    if (const auto* has_parent = r.try_get<HasParentComponent>(e)) {
      const auto parent_e = has_parent->parent;
      if (parent_e == entt::null) {
        auto* tag_c = r.try_get<TagComponent>(e);
        auto* item_key_c = r.try_get<ItemKey>(e);
        SDL_Log("%s has a null parent, key: %s", tag_c->tag.c_str(), item_key_c->key.c_str());
        dead.dead.emplace(e);
      }
      if (!r.valid(parent_e)) {
        auto* tag_c = r.try_get<TagComponent>(e);
        auto* item_key_c = r.try_get<ItemKey>(e);
        SDL_Log("%s has an invalid parent, key: %s", tag_c->tag.c_str(), item_key_c->key.c_str());
        dead.dead.emplace(e);
      }
    }
  }

  // process create requests
  const auto requests = r.view<WaitForInitComponent>();
  r.remove<WaitForInitComponent>(requests.begin(), requests.end());
};

} // namespace game2d