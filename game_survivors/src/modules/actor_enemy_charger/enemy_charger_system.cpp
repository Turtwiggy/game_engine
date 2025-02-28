#include "enemy_charger_system.hpp"

#include "enemy_charger_components.hpp"
#include "engine/actors/actor_helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_cooldown/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_physics_apply_force/components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"

namespace game2d {

void
update_enemy_charger_system(entt::registry& r)
{
  // keep your distance. when you're within the distance threshold,
  // wait x seconds "charging".
  // and then charge at the player.
  // once you pass your target spot... "catch your breath"

  auto view = r.view<ChargerEnemyComponent, CooldownComponent, const DynamicTargetComponent>();
  for (const auto& [e, charger_c, cooldown_c, target_c] : view.each()) {

    if (charger_c.state == ChargerEnemyState::START_APPROACH) {
      set_colour(r, e, hex_to_srgb("#e99f10")); // orange

      ApplyForceToDynamicTarget tgt_c;
      tgt_c.orbit = false;
      tgt_c.reduce_thrusters = false;
      tgt_c.speed = charger_c.approach_speed; // m/s
      r.emplace<ApplyForceToDynamicTarget>(e, tgt_c);
      r.get<PhysicsBodyComponent>(e).body->SetLinearDamping(1.0);

      charger_c.state = ChargerEnemyState::APPROACHING;
    }

    if (charger_c.state == ChargerEnemyState::APPROACHING) {
      // check your distance from your target
      const auto d = get_position(r, e) - get_position(r, target_c.target);
      const auto d2 = d.x * d.x + d.y * d.y;
      if (d2 > charger_c.d2_to_start_charge)
        continue;
      r.remove<ApplyForceToDynamicTarget>(e);

      charger_c.state = ChargerEnemyState::START_CHARGE_UP;
    }

    if (charger_c.state == ChargerEnemyState::START_CHARGE_UP) {
      cooldown_c.time_max = charger_c.time_to_charge_up;
      reset_cooldown(cooldown_c);

      charger_c.state = ChargerEnemyState::CHARGE_UP;
    }

    if (charger_c.state == ChargerEnemyState::CHARGE_UP) {

      // flash while charging...
      auto* scale_c = r.try_get<RequestHitScaleComponent>(e);
      if (!scale_c)
        r.emplace<RequestHitScaleComponent>(e);

      if (cooldown_c.time > 0.0f)
        continue;
      charger_c.state = ChargerEnemyState::START_CHARGE;
    }

    if (charger_c.state == ChargerEnemyState::START_CHARGE) {
      auto target_e = r.get<DynamicTargetComponent>(e).target;
      auto target_pos = get_position(r, target_e);
      auto you_pos = get_position(r, e);

      const auto raw_dir = target_pos - you_pos;
      const auto nrm_dir = engine::normalize_safe(raw_dir);

      ApplyForceInDirectionComponent tgt_c;
      tgt_c.tgt_vel = charger_c.charge_speed * nrm_dir; // add speed modifier?
      r.emplace<ApplyForceInDirectionComponent>(e, tgt_c);
      charger_c.start_charge_position = you_pos;

      charger_c.state = ChargerEnemyState::CHARGE;
    }

    if (charger_c.state == ChargerEnemyState::CHARGE) {
      const auto d = charger_c.start_charge_position - get_position(r, e);
      const float d2 = d.x * d.x + d.y * d.y;
      if (d2 < charger_c.d2_to_charge)
        continue;
      // THATS ENOUGH CHARGING NOW BRO
      r.remove<ApplyForceInDirectionComponent>(e);
      charger_c.state = ChargerEnemyState::START_CATCH_YOUR_BREATH;
    }

    if (charger_c.state == ChargerEnemyState::START_CATCH_YOUR_BREATH) {
      cooldown_c.time_max = charger_c.time_to_catch_breath;
      reset_cooldown(cooldown_c);
      charger_c.state = ChargerEnemyState::CATCH_YOUR_BREATH;
    }

    if (charger_c.state == ChargerEnemyState::CATCH_YOUR_BREATH) {

      // show that the seahorse is "deflated"... change it's colour?
      set_colour(r, e, hex_to_srgb("#f15f22")); // dark_orange

      if (cooldown_c.time > 0.0f)
        continue;
      charger_c.state = ChargerEnemyState::START_APPROACH;
    }
  }
}

} // namespace game2d