#include "ui_units_helpers.hpp"

#include "engine/io/settings.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"
#include "ui_units_components.hpp"
#include "ui_units_helpers.hpp"

namespace game2d {

std::vector<UnitType>
load_units(entt::registry& r)
{
  SDL_Log("Loading units from disk...");
  std::string key = "units";

  const auto units_opt = get_vector<UnitType>(key);
  if (!units_opt.has_value()) {
    SDL_Log("(Warning) Key: 'Units' was not found in save file");
    return {};
  }

  return units_opt.value();
};

void
save_units(entt::registry& r, const std::vector<UnitType>& units)
{
  std::string key = "units";
  save_vector(key, units);
};

void
add_unit_to_entt(entt::registry& r, const UnitType& unit)
{
  auto e = spawn_mob(r, "dungeon_actor_hero");
  r.emplace<PlayerComponent>(e);
  r.emplace<TeamComponent>(e, AvailableTeams::player);
  r.emplace<DebugBodyAndInventory>(e);
  r.emplace_or_replace<NameComponent>(e, NameComponent{ unit.name });
  r.emplace<UnitPersistentState>(e, UnitPersistentState{ unit.active, unit.permadead });
};

} // namespace game2d