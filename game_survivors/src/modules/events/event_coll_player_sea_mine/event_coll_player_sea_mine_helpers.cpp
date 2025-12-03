#include "pch.hpp"

#include "event_coll_player_sea_mine_components.hpp"
#include "event_coll_player_sea_mine_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actors/actor_enemy/components.hpp"
#include "modules/actors/actor_exploder/actor_exploder_helpers.hpp"
#include "modules/actors/actor_player/components.hpp"

namespace game2d {

void
handle_player_enter_sea_mine(entt::registry& r, const OnCollisionEnter& evt)
{
  const auto [pfixture_e, item_e] = coll<PlayerFixtureComponent, ItemSeaMineComponent>(r, evt.a, evt.b);
  if (pfixture_e == entt::null || item_e == entt::null)
    return;

  SDL_Log("You collided with a sea mine... exploding");

  //
  // use callbacks when need to update physics things,
  // because currently collision via physics world
  // and updating things during physics step makes physics sad
  //

  // Same as exploder logic...
  const auto item_par_e = r.get<const HasParentComponent>(item_e).parent;

  const std::function<bool(entt::registry&, entt::entity)> filter_criteria = [](entt::registry& r, entt::entity e) -> bool {
    bool valid_target = false;
    valid_target |= r.try_get<EnemyComponent>(e) != nullptr;
    return valid_target;
  };

  const float bomb_radius_pixels = 200;
  add_explode_on_death_callback(r, item_par_e, bomb_radius_pixels, filter_criteria, ParticleType::DEATH_SEA_MINE);

  // explode audio
  const auto spawn_audio = [](entt::registry& r, const entt::entity e) {
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "BOMB_EXPLOSION_01" });
  };
  auto& callbacks_c = r.get<OnDeathCallbacks>(item_par_e);
  callbacks_c.callbacks.push_back(spawn_audio);

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  dead.dead.push_back(item_par_e);
}

} // namespace game2d