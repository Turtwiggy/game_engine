#pragma once

#include "modules/ui_hierarchy/components.hpp"

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <string>
#include <vector>

namespace game2d {

struct EntityHierarchyComponent
{
  entt::entity parent = entt::null;
  std::vector<entt::entity> children;
};

struct SINGLETON_HierarchyComponent
{
  entt::entity selected_entity = entt::null;
  entt::entity root_node = entt::null;
};

} // namespace game2d