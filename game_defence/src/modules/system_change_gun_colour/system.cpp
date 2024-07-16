#include "system.hpp"

#include "helpers.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "modules/ui_colours/helpers.hpp"

namespace game2d {

void
update_change_gun_colour_system(entt::registry& r)
{
  for (const auto& [e, tbc] : r.view<const ActionState>().each()) {
    const auto gun_e = get_gun(r, e);
    if (gun_e == entt::null)
      continue;
    if (tbc.actions_available == 0)
      set_colour(r, gun_e, get_srgb_colour_by_tag(r, "gun_empty"));
    else
      set_colour(r, gun_e, get_srgb_colour_by_tag(r, "weapon_shotgun"));
  }
};

} // namespace game2d