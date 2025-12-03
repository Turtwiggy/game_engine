#include "pch.hpp"

#include "engine/maths/maths.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "entt/entity/fwd.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/systems/system_island_revive/island_revive_components.hpp"
#include "modules/systems/system_move_to_target_via_lerp/components.hpp"
#include "modules/systems/system_physics_apply_force/components.hpp"
#include "reassign_targets_helpers.hpp"

namespace game2d {

void
handle_death_event__reassign_targets(entt::registry& r, const DeathEvent& evt)
{
  if (!r.all_of<PlayerBoatComponent>(evt.dead))
    return;
  auto boat_e = evt.dead;

  std::vector<entt::entity> alive_players;
  for (const auto& [e, boat_c] : r.view<PlayerBoatComponent>().each()) {
    auto fixture_e = get_fixture_by_tag(r, e, "fixture_player");
    auto& hp_c = r.get<HealthComponent>(fixture_e);
    if (hp_c.hp > 0)
      alive_players.push_back(e);
  }

  if (alive_players.empty())
    return; // no one is alive?!

  static engine::RandomState random_target(0);

#if defined(_DEBUG)
  int reassigned = 0;
#endif

  auto view = r.view<DynamicTargetComponent, PhysicsDynamicTarget, EnemyComponent>();
  for (const auto& [e, target_c, physics_target_c, enemy_c] : view.each()) {
    if (target_c.target == boat_e) {
      auto it = alive_players.begin();
      auto rnd_idx = engine::rand_det_s(random_target.rng, 0, (int)alive_players.size());

      target_c.target = alive_players[rnd_idx];
      physics_target_c.target = alive_players[rnd_idx];

#if defined(_DEBUG)
      // auto& player_c = r.get<PlayerComponent>(target_c.target);
      // SDL_Log("New target: %s", player_c.display_name.c_str());
      reassigned++;
#endif
    }
  }

#if defined(_DEBUG)
  SDL_Log("Someone died... reassigned %i targets for enemies", reassigned);
#endif
}

} // namespace game2d