#include "trait_electromancy_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/physics/physics_components.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/event_damage/event_damage_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_alpha_based_on_lifecycle/alpha_based_on_lifecycle_components.hpp"
#include "modules/system_particles/components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_traits/trait_helpers.hpp"

#include <SDL2/SDL_log.h>

namespace game2d {

struct ElectromancyTraitComponent
{
  int shots_until_electromancy = 2;
  int shots_until_electromancy_left = 2;

  int electricity_damage = 20;
};

void
handle_shoot_event__trait_electromancy(entt::registry& r, const ShootEvent& evt)
{
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  const auto from_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (from_e == entt::null || wep_e == entt::null)
    return;

  const auto* trait_c = r.try_get<TraitComponent>(from_e);
  if (!trait_c)
    return;

  const auto trait = AquirableTrait::ELECTROMANCY;
  if (!has_trait(r, trait_c->traits, trait))
    return;

  auto& data_c = r.get_or_emplace<ElectromancyTraitComponent>(from_e);
  data_c.shots_until_electromancy_left -= 1;

  // activation: based on number of shots fired
  if (data_c.shots_until_electromancy_left > 0)
    return;
  data_c.shots_until_electromancy_left = data_c.shots_until_electromancy;

  // LET THERE BE THUNDER.
  // Call down a lightning strike on a nearby enemy

  const auto parent_pos = get_position(r, evt.parent_e);
  const auto search_radius = 100.0f;

  const auto is_enemy = [](entt::registry& r, const entt::entity e) -> bool {
    return r.try_get<EnemyComponent>(e) != nullptr;
  };
  auto enemies = get_all_in_area_filtered(r, parent_pos, search_radius, is_enemy);
  if (enemies.size() == 0)
    return;

  // Get the nearest enemy
  auto sort_by_distance = [](const auto& a, const auto& b) { return a.first < b.first; };
  std::sort(enemies.begin(), enemies.end(), sort_by_distance);
  auto nearest_e = enemies[0].second;
  auto nearest_fixture_e = get_fixture(r, nearest_e);

  //
  // Send a damage event. (immediately)
  //
  {
    DamageEvent evt;
    evt.from = entt::null; // system
    evt.to = nearest_fixture_e;
    evt.type = DamageType::PHYSICAL;
    evt.amount = data_c.electricity_damage;
    evts_c.dispatcher->trigger(evt);
    evts_c.dispatcher->update();
  }

  //
  // Spawn an effect
  //
  const auto pos = get_position(r, nearest_e);
  const auto size = glm::vec2{ 24 * 1.5, 48 * 1.5 };
  const auto thunder_e = spawn(r, "effect_thunder");
  give_life(r, thunder_e, pos, size);

  EntityTimedLifecycle lifecycle_c{ .milliseconds_alive_max = (int)(1 * 1000) };
  r.emplace<EntityTimedLifecycle>(thunder_e, lifecycle_c);
  r.emplace<SetAlphaBasedOnLifecycleComponent>(thunder_e);

  ScaleOverTimeComponent sotc;
  sotc.seconds_until_complete = lifecycle_c.milliseconds_alive_max / 1000.0f;
  sotc.start_size = size;
  sotc.end_size = { 0, 0 };
  r.emplace<ScaleOverTimeComponent>(thunder_e, sotc);

  r.remove<OnDeathCallbacks>(thunder_e); // you are the effect, dont spawn particles on death
  set_z_index(r, thunder_e, ZLayer::VFX);
}

} // namespace game2d