#pragma once

#include "modules/ui_hierarchy/components.hpp"

// other lib headers
#include <entt/entt.hpp>
#include <nlohmann/json.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

struct EntityHierarchyComponent
{
  entt::entity parent = entt::null;
  std::vector<entt::entity> children;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EntityHierarchyComponent, parent, children);
};

struct RootNode
{
  bool placeholder = true;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(RootNode, placeholder);
};

} // namespace game2d