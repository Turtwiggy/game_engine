#pragma once

// c++ lib headers
#include <entt/entt.hpp>

namespace game2d {

struct SINGLETON_HierarchyComponent
{
  entt::entity selected_entity = entt::null;
};

} // namespace game2d