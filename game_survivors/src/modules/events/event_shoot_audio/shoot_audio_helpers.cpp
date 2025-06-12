#include "pch.hpp"

#include "shoot_audio_helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"

namespace game2d {

void
handle_shoot_event__audio(entt::registry& r, const ShootEvent& evt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto par_e = evt.parent_e;
  const auto wep_e = evt.weapon_e;

  if (par_e == entt::null || wep_e == entt::null)
    return;

  // request to play audio
  const auto& weapon_data = r.get<const Weapon_OnDiskData>(wep_e);
  const auto& weapon_audio_tags = weapon_data.audio;

  // // todo: randomize audio, dont just choose [0]
  // if (weapon_audio_tags.size() == 0) {
  //   const std::string err = std::format("no audio provided for weapon: {}", weapon_data.key);
  //   throw std::runtime_error(err);
  // }
  create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = weapon_audio_tags[0] });
}

} // namespace game2d