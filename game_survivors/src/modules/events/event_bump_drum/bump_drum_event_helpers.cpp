#include "pch.hpp"

#include "bump_drum_event_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_island_drum/drum_component.hpp"
#include "modules/systems/system_quip/quip_helpers.hpp"

namespace game2d {

void
handle_bump_event__drum(entt::registry& r, const BumpEvent& evt)
{
  auto* drum_c = r.try_get<DrumComponent>(evt.to);
  if (!drum_c)
    return;

  // play some audio!
  static engine::RandomState rnd(0);
  const auto drum_tracks = 5;
  const auto drum_track_key = std::format("DRUM_0{}", engine::rand_det_s(rnd.rng, 1, drum_tracks + 1));
  AudioRequestPlayEvent audio_evt;
  audio_evt.tag = drum_track_key;
  create_empty<AudioRequestPlayEvent>(r, audio_evt);

  // request a quip from the thing that banged the drum.
  // request_quip(r, evt.from);
}

} // namespace game2d