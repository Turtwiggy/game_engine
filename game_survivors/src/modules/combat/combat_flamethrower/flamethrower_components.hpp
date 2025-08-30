#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct FlamethrowerComponent
{
  bool placeholder = true;
};

struct FlamethrowerFlameComponent
{
  bool active = true;
};

// the flame is a child of the flamethrower
// the flamethrower has a child of flame
// fixture comp attached to the fixture on the flame
struct FlamethrowerFlameFixtureComponent
{
  std::unordered_set<entt::entity> burning_fixture_es;
};

} // namespace game2d