#include "pch.hpp"

#include "upgrade_hp_regen_system.hpp"

#include "engine/lifecycle/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "upgrade_hp_regen_components.hpp"

namespace game2d {

void
update_upgrade_hp_regen_system(entt::registry& r, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& view = r.view<HealthComponent, HasParentComponent>();
  for (const auto& [e, hp_c, parent_c] : view.each()) {

    // Check the parent for the stat modifier
    auto parent_e = parent_c.parent;

    // the stat modifier is on the parent, not on the fixture
    const auto* stat_c = r.try_get<StatModifierComponent>(parent_e);
    if (!stat_c)
      continue;

    // The health regen comp is on the parent, not on the fixture
    auto* hp_regen_c = r.try_get<ActorHealthRegenComponent>(parent_e);
    if (!hp_regen_c)
      continue;

    const auto val = hp_regen_c->hp_per_second;
    const auto key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_REGEN));
    const auto val_mod = stat_c->apply_modifiers(val, key);

    hp_c.hp += val_mod * dt;
    hp_c.hp = glm::min(hp_c.hp, hp_c.max_hp);
  }
}

} // namespace game2d