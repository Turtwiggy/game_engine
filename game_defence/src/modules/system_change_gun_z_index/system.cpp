#include "system.hpp"

#include "helpers.hpp"
#include "modules/system_change_gun_colour/helpers.hpp"
#include "modules/system_turnbased_enemy/components.hpp"
#include "renderer/transform.hpp"

namespace game2d {

void
update_change_gun_z_index_system(entt::registry& r)
{
  for (const auto& [e, tbc, player_t] : r.view<const ActionState, const TransformComponent>().each()) {
    const auto gun_e = get_gun(r, e);
    if (gun_e == entt::null)
      continue;
    auto& gun_t = r.get<TransformComponent>(gun_e);
    set_z_index(r, gun_e, 0);

    const int player_y = player_t.position.y;
    const int gun_y = gun_t.position.y;

    // if gun is below player...
    if (gun_y > player_y)
      gun_t.position.z = 1; // draw on top
    else
      gun_t.position.z = -1; // draw behind
  }
}

} // namespace game2d