// header
#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/components.hpp"

namespace game2d {

void
update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>(entt::exclude<WaitForInitComponent>);
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += static_cast<int>(milliseconds_dt);
  });

  // destroy all dead objects
  //
  auto& physics_c = get_first_component<SINGLE_Physics>(r);
  for (const auto& e : dead.dead) {
    // Death callback
    if (auto* callback = r.try_get<OnDeathCallback>(e))
      callback->callback(r, e);

    // Update physics
    if (auto* pb = r.try_get<PhysicsBodyComponent>(e))
      physics_c.world->DestroyBody(pb->body);
  }

  for (const auto e : dead.dead) {
    if (r.valid(e))
      r.destroy(e);
  }
  dead.dead.clear();

  // process create requests
  const auto requests = r.view<WaitForInitComponent>();
  r.remove<WaitForInitComponent>(requests.begin(), requests.end());
};

} // namespace game2d