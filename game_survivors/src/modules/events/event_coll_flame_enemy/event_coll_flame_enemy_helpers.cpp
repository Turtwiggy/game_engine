#include "pch.hpp"

#include "event_coll_flame_enemy_helpers.hpp"

#include "modules/actors/actor_enemy/components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"

namespace game2d {

void
handle_flame_enemy__coll_enter(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [flame_fixture_e, enemy_fixture_e] = coll<FlamethrowerFlameFixtureComponent, EnemyComponent>(r, evt.a, evt.b);
  if (flame_fixture_e == entt::null || enemy_fixture_e == entt::null)
    return;

  // keep track of the burning enemy
  auto& flame_fixture_c = r.get<FlamethrowerFlameFixtureComponent>(flame_fixture_e);
  flame_fixture_c.burning_fixture_es.emplace(enemy_fixture_e);
}

void
handle_flame_enemy__coll_exit(entt::registry& r, const OnCollisionExit& evt)
{
  const auto [flame_fixture_e, enemy_fixture_e] = coll<FlamethrowerFlameFixtureComponent, EnemyComponent>(r, evt.a, evt.b);
  if (flame_fixture_e == entt::null || enemy_fixture_e == entt::null)
    return;

  // remove track of the burning enemy
  auto& flame_fixture_c = r.get<FlamethrowerFlameFixtureComponent>(flame_fixture_e);
  const auto& bf_c = flame_fixture_c.burning_fixture_es;
  if (bf_c.find(enemy_fixture_e) == bf_c.end())
    return; // enemy left that was never burning?
  flame_fixture_c.burning_fixture_es.erase(enemy_fixture_e);
}

} // namespace game2d