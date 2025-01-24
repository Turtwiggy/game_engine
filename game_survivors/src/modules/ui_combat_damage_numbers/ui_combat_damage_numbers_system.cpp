#include "ui_combat_damage_numbers_system.hpp"

#include "modules/event_damage/event_damage_helpers.hpp"
#include "ui_combat_damage_numbers_components.hpp"

#include <SDL2/SDL_log.h>
#include <format>

namespace game2d {
using namespace std::literals;

void
handle_damage_event_for_ui(entt::registry& r, const DamageEvent& evt)
{
  const auto to_e = evt.to;
  const auto damage = calculate_damage_to_take(r, evt);

  // add a new entry to the UI_BufferComponent...
  auto& buffer_c = r.get_or_emplace<UI_BufferComponent>(to_e);
  TimedEntry entry;
  entry.damage = (float)damage;
  entry.time_left_max = 3.0f;
  entry.time_left = entry.time_left_max;
  buffer_c.entries.push_back(entry);

  SDL_Log("%s", std::format("adding damage event for ui...").c_str());
};

void
update_ui_combat_damage_numbers_system(entt::registry& r, const float dt, const glm::ivec2 mouse_pos)
{
  // TODO...
}

} // namespace game2d