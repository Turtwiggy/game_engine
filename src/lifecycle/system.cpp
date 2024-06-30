// header
#include "system.hpp"

#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"

namespace game2d {

void
update_lifecycle_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // update all components with timed lifecycle
  //
  const auto& view = r.view<EntityTimedLifecycle>(entt::exclude<WaitForInitComponent>);
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += static_cast<int>(milliseconds_dt);
  });

  // provide a list of entity ids that were killed
  dead.things_that_died.clear();
  for (const auto& to_be_killed : dead.dead)
    dead.things_that_died.push_back(to_be_killed);

  // destroy all dead objects
  r.destroy(dead.dead.begin(), dead.dead.end());
  dead.dead.clear();

  // process create requests
  const auto requests = r.view<WaitForInitComponent>();
  r.remove<WaitForInitComponent>(requests.begin(), requests.end());
};

} // namespace game2d