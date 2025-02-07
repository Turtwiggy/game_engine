// header
#include "lifecycle_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/combat/components.hpp"
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

    if (auto* callback = r.try_get<OnDeathCallbacks>(e)) {
      for (const auto& cb : callback->callbacks)
        cb(r, e);
    }

    uniquely_dead.emplace(e);
  }

  for (const auto e : uniquely_dead) {
    // A destroyed fixture belonging to a body;
    // destroy the parent when the fixture dies.
    if (auto* fixture_c = r.try_get<PhysicsFixtureComponent>(e)) {
      const auto parent_e = r.get<HasParentComponent>(e).parent;
      uniquely_dead.emplace(parent_e);
    }
    // A destroyed parent might have fixtures;
    // destroy the fixtures when the parent dies.
    if (auto* body_c = r.try_get<PhysicsBodyComponent>(e)) {
      for (const auto fix_e : body_c->fixtures)
        uniquely_dead.emplace(fix_e);
    }

    // Destroy all the weapons
    if (auto* wep_c = r.try_get<HasWeaponsComponent>(e))
      for (const auto wep_e : wep_c->weapons)
        uniquely_dead.emplace(wep_e);

    //
  }

  for (const auto e : uniquely_dead) {
    // Update physics
    if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
      physics_c.world->DestroyBody(pb->body);

    // Update entt
    r.destroy(e);
  }

  // process create requests
  const auto requests = r.view<WaitForInitComponent>();
  r.remove<WaitForInitComponent>(requests.begin(), requests.end());
};

} // namespace game2d