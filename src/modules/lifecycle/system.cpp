// header
#include "system.hpp"

#include "components.hpp"
#include "modules/ui_hierarchy/components.hpp"

#include <vector>

void
game2d::update_lifecycle_system(entt::registry& r, uint64_t milliseconds_dt)
{
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&eb, &milliseconds_dt](auto entity, auto& lifecycle) {
    if (lifecycle.milliseconds_alive > lifecycle.milliseconds_alive_max)
      eb.dead.emplace(entity);
    lifecycle.milliseconds_alive += milliseconds_dt;
  });

  // process destroyed objects
  for (const auto& entity : eb.dead) {
    r.destroy(entity);
  }

  // update hierarchy...
  // update hroot.children if entity is removed
  const auto& h = r.view<RootNode>().front();
  auto& hroot = r.get<EntityHierarchyComponent>(h);
  std::vector<entt::entity>::iterator it = hroot.children.begin();
  while (it != hroot.children.end()) {
    if (!r.valid(*it))
      it = hroot.children.erase(it);
    else
      ++it;
  }

  eb.dead.clear();
};