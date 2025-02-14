#include "upgrade_hp_regen_system.hpp"

#include "engine/lifecycle/components.hpp"
#include "magic_enum.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "upgrade_hp_regen_components.hpp"

#include <glm/glm.hpp>

namespace game2d {

void
update_upgrade_hp_regen_system(entt::registry& r, float dt)
{
  const auto& view = r.view<PlayerFixtureComponent, HealthComponent, HasParentComponent>();
  for (const auto& [e, player_fix_c, hp_c, parent_c] : view.each()) {

    // Check the parent for the stat modifier
    auto parent_e = parent_c.parent;
    auto& stat_c = r.get<StatModifierComponent>(parent_e);

    auto* hp_regen_c = r.try_get<ActorHealthRegenComponent>(parent_e);
    if (!hp_regen_c)
      continue;

    const auto val = hp_regen_c->hp_per_second;
    const auto key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_REGEN));
    const auto val_mod = stat_c.apply_modifiers(val, key);

    hp_c.hp += val_mod * dt;
    hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);
  }
}

} // namespace game2d