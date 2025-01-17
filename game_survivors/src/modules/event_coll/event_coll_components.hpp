#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct CollInfo
{
  std::set<entt::entity> other;
};

} // namespace game2d