#include "pch.hpp"

#include "event_steam_remote_player_connected_helpers.hpp"

#include "engine/audio/audio_helpers.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "modules/scene/scene_helpers.hpp"


namespace game2d {

void
handle_steam_event__remote_player_connected(entt::registry& r, const RemotePlayerConnectedEvent& evt)
{
  audio::sdl_mixer::stop_all_audio(r);

  refresh_audio(r);

  move_to_scene_start(r, Scene::menu);
}

} // namespace game2d