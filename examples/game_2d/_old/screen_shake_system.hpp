#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct ScreenShakeTimer
{
  float time_left = 0.0f;
};

void
update_screen_shake_system(entt::registry& registry, float dt);

}