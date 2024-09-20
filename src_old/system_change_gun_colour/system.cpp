#include "system.hpp"

#include "actors/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_colours/helpers.hpp"

namespace game2d {

void
update_change_gun_colour_system(entt::registry& r)
{
  for (const auto& [e, tbc, gun_c] : r.view<const ActionState, const HasWeaponComponent>().each()) {
    if (tbc.actions_available == 0)
      set_colour(r, gun_c.instance, get_srgb_colour_by_tag(r, "gun_empty"));
    else
      set_colour(r, gun_c.instance, get_srgb_colour_by_tag(r, "weapon_shotgun"));
  }
};

} // namespace game2d