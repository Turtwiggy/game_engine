#include "pch.hpp"

#include "event_upgrade_helpers.hpp"

#include "event_upgrade_components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"

namespace game2d {

void
handle_upgrade_event(entt::registry& r, const UpgradeEvent& evt)
{
  SDL_Log("processing upgrade... %s", evt.upgrade.name.c_str());

  // Add it to the list of aquired upgrades (for ui purposes)
  auto& upgrade_c = r.get<UpgradeComponent>(evt.e);
  upgrade_c.aquired_upgrades.push_back(evt.upgrade.name);

  const auto& effects = evt.upgrade.effects;
  for (const auto& effect : effects) {

    //
    // Give the player stat modifiers
    //
    if (effect.stat.has_value()) {

      const auto& stat = effect.stat.value();
      const auto& type = effect.type.value();
      const auto& val_json = effect.value.value();

      float val_float = 0.0f;
      if (val_json.is_number())
        val_json.get_to(val_float);

      if (type == "stat_flat_increase") {
        StatFlatIncrease inc(val_float, stat);

        // Note: unlock for all players, which may not be desired behaviour
        const auto& view = r.view<PlayerComponent, StatModifierComponent>();
        for (const auto& [e, player_c, stat_c] : view.each())
          stat_c.add(std::make_shared<StatFlatIncrease>(inc));

      } else if (type == "stat_percent_increase") {
        StatPercentIncrease per(val_float, stat);

        // Note: unlock for all players, which may not be desired behaviour
        const auto& view = r.view<PlayerComponent, StatModifierComponent>();
        for (const auto& [e, player_c, stat_c] : view.each())
          stat_c.add(std::make_shared<StatPercentIncrease>(per));

      } else {
        std::string err = std::format("Unknown upgrade: {}", type);
        throw std::runtime_error(err);
        continue;
      }
    }

    //
    // Give the player a trait
    //
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

    //
  }
}

} // namespace game2d