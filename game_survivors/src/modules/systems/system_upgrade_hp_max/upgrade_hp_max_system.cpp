#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_hull/hull_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "upgrade_hp_max_system.hpp"

namespace game2d {

void
update_upgrade_hp_max_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);

  const auto& view = r.view<const PlayerFixtureComponent, HealthComponent, const HasParentComponent>();
  for (const auto& [e, player_fix_c, hp_c, parent_c] : view.each()) {

    // Check the parent for the stat modifier
    const auto parent_e = parent_c.parent;
    const auto hull_key = r.get<HullKeyComponent>(parent_e).key;
    auto& upgrades_c = r.get<StatModifierComponent>(parent_e);

    // get the base hp calculated off the hull size.
    auto get_key = []<typename T>(const std::vector<T>& data, const std::string& key) -> std::optional<T> {
      const auto it = std::find_if(data.begin(), data.end(), [&key](const T& item) { return item.key == key; });
      if (it == data.end())
        return std::nullopt;
      return (*it);
    };
    const auto hull = get_key(hulls_c.hulls, hull_key).value();
    const auto hull_size = glm::vec2{ hull.width, hull.height };
    const auto val_max_hp = (hull_size.x * hull_size.y) / 100.0f;

    const auto key_max_hp = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_HEALTH_MAX));
    const auto mod_max_hp = upgrades_c.apply_modifiers(val_max_hp, key_max_hp);

    hp_c.max_hp = mod_max_hp;
  }
}

} // namespace game2d