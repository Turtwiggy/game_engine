#include "upgrade_max_hp_system.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

#include "magic_enum.hpp"

namespace game2d {

void
update_upgrade_max_hp_system(entt::registry& r)
{
  const auto& view = r.view<PlayerFixtureComponent, HealthComponent, HasParentComponent>();
  for (const auto& [e, player_fix_c, hp_c, parent_c] : view.each()) {

    // Check the parent for the stat modifier
    auto parent_e = parent_c.parent;

    if (!r.valid(parent_e))
      continue; // you probably died

    auto& upgrades_c = r.get<StatModifierComponent>(parent_e);

    const auto key_max_hp = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_MAX_HEALTH));

    const auto val_max_hp = 10; // default hp

    const auto mod_max_hp = (int)upgrades_c.apply_modifiers(val_max_hp, key_max_hp);

    hp_c.max_hp = mod_max_hp;
  }
}

} // namespace game2d