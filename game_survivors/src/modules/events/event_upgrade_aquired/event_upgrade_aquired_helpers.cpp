#include "pch.hpp"

#include "engine/lifecycle/components.hpp"
#include "event_upgrade_aquired_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/events/event_upgrade/event_upgrade_components.hpp"

namespace game2d {

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt)
{
  auto player_e = evt.e;
  auto type_str = evt.data.type;
  const auto upgrade = evt.data.stat;

  const auto rarity_str = std::string(magic_enum::enum_name(evt.data.rarity));
  const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));
  const auto amount = evt.data.value;

  auto& stats_c = r.get<StatModifierComponent>(player_e);
  if (type_str == "stat_flat_increase")
    stats_c.add(std::make_shared<StatFlatIncrease>(amount, upgrade_str));
  else if (type_str == "stat_percent_increase")
    stats_c.add(std::make_shared<StatPercentIncrease>(amount, upgrade_str));
  else
    throw std::runtime_error("Unknown stat type");

  //
  // If it's a weapon upgrade, upgrade the weapon level by 1.
  {
    const auto& wab = weapon_and_bullet_stats;
    const bool is_wep_stat = std::find(wab.begin(), wab.end(), upgrade) != wab.end();
    if (is_wep_stat) {
      auto& children_c = r.get<HasChildrenComponent>(player_e);
      for (const auto& child_e : children_c.children) {

        auto* wep_level_c = r.try_get<WeaponLevelComponent>(child_e);
        if (!wep_level_c)
          continue;

        // upgrade all weapons? this could be improved
        // by associating an upgrade with a weapon to level.
        wep_level_c->level++;

        // TODO: if weapon level reaches a critical level,
        // send a weapon upgrade level event
      }
    }
  }

  /*
    if (effect.trait.has_value()) {
      auto trait_str = effect.trait.value();
      SDL_Log("giving players trait: %s", trait_str.c_str());

      const auto trait_enum_opt = magic_enum::enum_cast<AquirableTrait>(trait_str);

      // .value() should be fine because all
      // traits validated when .upgrades.jsonc is loaded
      const auto trait_enum_val = trait_enum_opt.value();

      // Note: unlock for all players, which may not be desired behaviour
      const auto& view = r.view<PlayerComponent, TraitComponent>();
      for (const auto& [e, player_c, trait_c] : view.each())
        trait_c.traits.emplace(trait_enum_val);
    }
  }
  */
}

} // namespace game2d