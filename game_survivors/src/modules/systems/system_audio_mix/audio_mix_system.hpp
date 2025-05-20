#pragma once

#include "engine/audio/audio_components.hpp"
#include <entt/fwd.hpp>

namespace game2d {

void
handle_audio_complete_event__new_game_track(entt::registry& r, const AudioCompleteEvent& evt);

void
update_audio_mix_system(entt::registry& r);

} // namespace game2d