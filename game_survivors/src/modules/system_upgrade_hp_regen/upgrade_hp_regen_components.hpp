#pragma once

namespace game2d {

struct ActorHealthRegenComponent
{
  float hp_per_second = 0;
  float dt = 0;
};

} // namespace game2d