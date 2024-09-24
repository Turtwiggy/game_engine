#include "event_damage_helpers.hpp"

#include "fmt/core.h"

namespace game2d {

void
handle_damage_event(entt::registry& r, const DamageEvent& evt)
{
  auto from = evt.from;
  auto to = evt.to;
  auto amount = evt.amount;

  fmt::println("damage event sent... (TODO) process it");
};

} // namespace game2d