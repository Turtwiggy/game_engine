#include "pch.hpp"

#include "upgrade_xp_zone_size_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_enemy_grower/enemy_grower_helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "upgrade_xp_zone_size_components.hpp"

namespace game2d {

void
update_upgrade_xp_zone_size_system(entt::registry& r)
{
  auto& physics_c = get_first_component<SINGLE_Physics>(r);

  auto view = r.view<const PlayerComponent, const ActorXpZoneSizeComponent, const StatModifierComponent>();
  for (const auto& [e, player_c, xp_zone_c, stats_c] : view.each()) {

    const auto key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_XP_ZONE_SIZE));
    const auto val = xp_zone_c.radius_meters;
    const auto val_mod = stats_c.apply_modifiers(val, key);

#if defined(_DEBUG)
    if (val != val_mod)
      SDL_Log("Upgrading xp zone...");
#endif
    const auto fixture_e = get_fixture_by_tag(r, e, "fixture_xp_zone");
    update_circle_fixture_size(r, e, fixture_e, meters_to_pixels(val_mod));
  }
}

} // namespace game2d