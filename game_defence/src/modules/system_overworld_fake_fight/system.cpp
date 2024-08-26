#include "system.hpp"

#include "actors/helpers.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/combat_wants_to_shoot/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"


namespace game2d {

void
update_overworld_fake_fight_system(entt::registry& r)
{
  if (get_first<SINGLE_OverworldFakeFight>(r) == entt::null)
    return;
  // get the player and enemy to shoot eachother...
  const auto player_e = get_first<PlayerComponent>(r);
  const auto enemy_e = get_first<EnemyComponent>(r);

  static bool first_time = false;
  if (first_time) {
    first_time = false;

    const auto setup_weapon = [&r](const auto& ship_e, const auto& target_e) {
      // TODO: finish implementing this
      /*
      // setup weapon
      const auto weapon_e = create_gameplay(r, EntityType::weapon_shotgun, get_position(r, ship_e));
      r.emplace_or_replace<TeamComponent>(weapon_e, r.get<TeamComponent>(ship_e).team);

      // set entity to aim at the other ship
      // (incorrectly, this needs ch anging),
      // this is on the parent not the gun
      // r.emplace<DynamicTargetComponent>(ship_e, target_e);

      // setup weapon
      r.emplace_or_replace<AbleToShoot>(weapon_e);
      r.get<HasParentComponent>(weapon_e).parent = ship_e;

      // link player&weapon
      r.emplace<HasWeaponComponent>(ship_e, HasWeaponComponent{ weapon_e });

      set_sprite(r, weapon_e, "EMPTY");
      */
    };

    // give player ship a gun
    setup_weapon(player_e, enemy_e);

    // give enemy ship a gun
    setup_weapon(enemy_e, player_e);
  }
}

} // namespace game2d