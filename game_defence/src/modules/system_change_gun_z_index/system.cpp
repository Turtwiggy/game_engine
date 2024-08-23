#include "system.hpp"

#include "helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_change_gun_z_index_system(entt::registry& r)
{
  for (const auto& [e, t_c, player_c, gun_c] :
       r.view<const ActionState, const TransformComponent, const HasWeaponComponent>().each()) {
    const auto gun_e = gun_c.instance;
    auto& gun_t = r.get<TransformComponent>(gun_e);
    set_z_index(r, gun_e, 0);

    const float player_y = player_c.position.y;
    const float gun_y = gun_t.position.y;

    // if gun is below player...
    if (gun_y > player_y)
      gun_t.position.z = 1.0f; // draw on top
    else
      gun_t.position.z = -1.0f; // draw behind
  }
}

} // namespace game2d