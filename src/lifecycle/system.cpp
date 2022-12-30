// header
#include "system.hpp"

void
game2d::update_lifecycle_system(SINGLETON_EntityBinComponent& dead, entt::registry& r, const uint64_t& milliseconds_dt)
{
  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&dead, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      dead.dead.emplace(entity);
    lifecycle.milliseconds_alive += milliseconds_dt;
  });

  // process destroyed objects
  for (const auto& entity : dead.dead)
    r.destroy(entity);

  // update hierarchy...
  // update hroot.children if entity is removed
  // const auto& h = r.view<RootNode>().front();
  // auto& hroot = r.get<EntityHierarchyComponent>(h);
  // std::vector<entt::entity>::iterator it = hroot.children.begin();
  // while (it != hroot.children.end()) {
  //   if (!r.valid(*it))
  //     it = hroot.children.erase(it);
  //   else
  //     ++it;
  // }

  dead.dead.clear();
};