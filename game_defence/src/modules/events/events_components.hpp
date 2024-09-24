#pragma once

#include <entt/entt.hpp>
#include <entt/signal/dispatcher.hpp>

namespace game2d {

struct SINGLE_Events
{
  entt::dispatcher* dispatcher;
};

} // namespace game2d