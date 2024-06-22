#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_SplashScreen
{
#if defined(_DEBUG)
  float time_on_splashscreen_seconds = 0.0f;
#else
  float time_on_splashscreen_seconds = 3.0f;
#endif
};

} // namespace game2d