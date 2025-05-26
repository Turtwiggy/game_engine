#include "pch.hpp"

#include "exploder_screenshake.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/systems/system_screenshake/components.hpp"
#include "modules/systems/system_traits/trait_components.hpp"

namespace game2d {

void
handle_death_event__exploder_screenshake(entt::registry& r, const DeathEvent& evt)
{
  //
  // once the exploder has finished their deaththroes...
  //
  auto dead_e = evt.dead;
  if (dead_e == entt::null)
    return;

  auto* traits_c = r.try_get<AiBehavioursComponent>(dead_e);
  if (!traits_c)
    return;

  auto it = std::find(traits_c->traits.begin(), traits_c->traits.end(), AiBehaviour::EXPLODE);
  if (it == traits_c->traits.end())
    return;

  create_empty<RequestScreenshakeComponent>(r, RequestScreenshakeComponent{ ScreenshakeType::EXPLODE });

  static engine::RandomState rnd(0);
  const int random_sound_idx = engine::rand_det_s(rnd.rng, 1, 5); // play a explode sound effect [1-4]
  SDL_Log("random_sound_idx: %i", random_sound_idx);
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = "ENEMY_EXPLODER_01" });
};

} // namespace game2d