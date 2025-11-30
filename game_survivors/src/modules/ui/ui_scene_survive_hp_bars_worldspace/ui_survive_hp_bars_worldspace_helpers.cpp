#include "pch.hpp"

#include "modules/actors/actor_player/components.hpp"
#include "ui_survive_hp_bars_worldspace_components.hpp"
#include "ui_survive_hp_bars_worldspace_helpers.hpp"

namespace game2d {

void
handle_damage_event__worldspace_hp_bars(entt::registry& r, const TookDamageEvent& evt)
{
  if (!r.all_of<PlayerFixtureComponent>(evt.to_fixture))
    return;

  auto& buffer_c = r.get_or_emplace<UI_BufferComponent>(evt.to_fixture);
  TimedEntry entry;
  entry.damage = (float)evt.amount;
  entry.time_left_max = 3.0f;
  entry.time_left = entry.time_left_max;
  buffer_c.entries.push_back(entry);
  buffer_c.time_since_damage = 0.0f;
}

} // namespace game2d