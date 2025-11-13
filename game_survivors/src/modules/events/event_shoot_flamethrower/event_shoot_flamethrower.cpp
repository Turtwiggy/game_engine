#include "pch.hpp"

#include "event_shoot_flamethrower.hpp"

#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"

namespace game2d {

void
handle_shoot_event__flamethrower(entt::registry& r, const ShootEvent& evt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  return; // disabled

  const auto wep_e = evt.weapon_e;

  if (wep_e == entt::null)
    return;

  auto& evts_c = SINGLE_Events::instance;

  // no children on the weapon; wont have flames
  if (!r.all_of<HasChildrenComponent>(wep_e))
    return;

  // get weapon damage
  // const auto& bullet_damage_c = r.get<BulletDamage>(wep_e);

  // apply flame to enemies in the flamethrower aoe.
  const auto& weapon_children_c = r.get<HasChildrenComponent>(wep_e).children;
  for (const auto weapon_child_e : weapon_children_c) {
    if (!r.all_of<FlamethrowerFlameComponent>(weapon_child_e))
      continue;
    auto& flame_c = r.get<FlamethrowerFlameComponent>(weapon_child_e);
    auto flame_fixture_e = get_fixture_by_tag(r, weapon_child_e, "fixture_flame");
    auto& flame_fixture_c = r.get<FlamethrowerFlameFixtureComponent>(flame_fixture_e);

    // SDL_Log("Burning: %zu", flame_fixture_c.burning_fixture_es.size());

    // Remove any dead entities from burning_fixture_es
    {
      auto& bs = flame_fixture_c.burning_fixture_es;
      auto count = std::erase_if(bs, [&r](const auto e) {
        // remove if not valid
        if (!r.valid(e))
          return true;
        // remove if dead
        if (auto* hp_c = r.try_get<HealthComponent>(e))
          return hp_c->hp <= 0;
        return false;
      });
      if (count > 0)
        SDL_Log("(flamethrower) Removed: %zu", count);
    }

    // damage all things in the burning list.
    for (const auto burning_fixture_e : flame_fixture_c.burning_fixture_es) {

      // Get the top-level parent, where most of the components will be
      entt::entity burning_parent_e = burning_fixture_e;
      if (auto* has_parent_c = r.try_get<HasParentComponent>(burning_fixture_e))
        burning_parent_e = has_parent_c->parent;

      DamageEvent evt;
      evt.from = wep_e;
      evt.to_parent = burning_parent_e;
      evt.to_fixture = burning_fixture_e;
      evt.type = WEAPON_DAMAGE::FIRE;
      evt.amount = 0; // no damage; gets damaged by fire stacks
      evts_c.dispatcher->trigger(evt);
      evts_c.dispatcher->update();
    }

    // SDL_Log("flamethrower hitting: %zu", filtered.size());
  }
}

} // namespace game2d