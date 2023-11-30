#pragma once

#include <entt/entt.hpp>

#include <set>
#include <vector>

namespace game2d {

struct SINGLE_HoverInfo
{
  std::set<entt::entity> hovered;
  std::vector<entt::entity> selected;
};

struct HoverableComponent
{
  bool placeholder = true;
};

} // namespace game2d