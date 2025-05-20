#include "modules/scene/scene_components.hpp"
#include "pch.hpp"

#include "audio_mix_system.hpp"

#include "audio_mix_components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"

namespace game2d {

//
// monitor track ending.
//
void
handle_audio_complete_event__new_game_track(entt::registry& r, const AudioCompleteEvent& evt)
{
  SDL_Log("AudioEventComplete");
  const auto& tag = evt.tag;

  // some other audio ended
  if (tag.find("GAME_") == std::string::npos)
    return;

  // not in the survive scene
  const auto& scene_c = get_first_component<SINGLE_CurrentScene>(r);
  if (scene_c.s != Scene::survive)
    return;

  create_empty<RequestGameTrack>(r);
};

void
update_audio_mix_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  process_requests<RequestGameTrack>(r, [&r](const auto& req) {
    //
    // audio::sdl_mixer::stop_all_audio(r);

    const auto& audio_c = get_first_component<SINGLE_AudioComponent>(r);
    int tracks = 0;
    for (int i = 0; i < audio_c.sounds.size(); i++) {
      const auto& s = audio_c.sounds[i];
      if (s.tag.find("GAME_") == std::string::npos)
        continue;
      tracks++;
    }

    // choose a random game track.

    static int seed = 0;
#if defined(_DEBUG)
    // seed++;
    seed = engine::get_system_time_for_seed();
#else
    seed = engine::get_system_time_for_seed();
#endif
    static engine::RandomState rnd(seed);

    const int rand_track = engine::rand_det_s(rnd.rng, 0, tracks - 1);
    const auto tag = "GAME_" + std::to_string(rand_track);
    SDL_Log("chosen gametrack: %s", tag.c_str());
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = tag, .looping = false });
  });
}

} // namespace game2d