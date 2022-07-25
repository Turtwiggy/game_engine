// header
#include "system.hpp"

#include "components.hpp"
#include "modules/ui_hierarchy/components.hpp"

#include <iostream>
#include <vector>

void
game2d::update_lifecycle_system(entt::registry& r, float fixed_dt)
{
  auto& eb = r.ctx().at<SINGLETON_EntityBinComponent>();

  // update all components with timed lifecycle
  const auto& view = r.view<EntityTimedLifecycle>();
  view.each([&eb, &fixed_dt](auto entity, auto& lifecycle) {
    if (lifecycle.time_alive > lifecycle.time_alive_max)
      eb.dead.emplace(entity);
    lifecycle.time_alive += fixed_dt;
  });

  // process destroyed objects
  for (auto entity : eb.dead) {
    std::cout << "destroying... " << std::to_string(static_cast<uint32_t>(entity)) << std::endl;
    r.destroy(entity);
  }

  // update hierarchy...
  // TODO: make this recursive
  auto& h = r.ctx().at<SINGLETON_HierarchyComponent>();
  auto& hroot = r.get<EntityHierarchyComponent>(h.root_node);
  std::vector<entt::entity>::iterator it = hroot.children.begin();
  while (it != hroot.children.end()) {
    if (!r.valid(*it))
      it = hroot.children.erase(it);
    else
      ++it;
  }

  eb.dead.clear();
};