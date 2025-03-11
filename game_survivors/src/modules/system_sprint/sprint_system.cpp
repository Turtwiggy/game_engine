#include "pch.hpp"

#include "sprint_system.hpp"

#include "modules/actor_player/components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "sprint_components.hpp"

namespace game2d {

void
update_sprint_system(entt::registry& r, float dt)
{
  const auto view = r.view<const InputComponent, ActorStaminaComponent, ActorSpeedComponent, StatModifierComponent>();
  for (const auto& [e, input_c, stamina_c, speed_c, upgrades_c] : view.each()) {

    // apply stamina mods here...
    const int val = stamina_c.max_stamina;
    const auto key_stamina = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_STAMINA));
    const auto mod_stamina = upgrades_c.apply_modifiers(val, key_stamina);

    if (input_c.sprint)
      stamina_c.cur_stamina -= stamina_c.depletion_rate * dt;
    else
      stamina_c.cur_stamina += stamina_c.depletion_rate * dt; // increase at depletion rate?

    // clamp stamina
    stamina_c.cur_stamina = glm::clamp(stamina_c.cur_stamina, 0.0f, mod_stamina);

    // you're sprinting!
    if (input_c.sprint && stamina_c.cur_stamina != 0.0f)
      speed_c.current_speed = 2.0 * speed_c.base_speed;
    // yer walkin
    else
      speed_c.current_speed = speed_c.base_speed;
  }
}

} // namespace game2d