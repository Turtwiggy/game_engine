#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SpritestackComponent
{
  entt::entity root = entt::null; // root of spritestack

  int spritestack_index = 0;
  int spritestack_total = 0;

  SpritestackComponent(int index)
    : spritestack_index(index){};
};

struct RotateOnSpotComponent
{
  bool placeholder = true;
};

} // namespace game2d