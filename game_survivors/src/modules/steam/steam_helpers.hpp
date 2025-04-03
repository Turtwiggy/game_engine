#pragma once

#include <SDL2/SDL_log.h>
#include <entt/fwd.hpp>
#include <steam/steam_api.h>

namespace game2d {

class SteamOverlayManager
{
private:
  STEAM_CALLBACK(SteamOverlayManager, OnGameOverlayActivated, GameOverlayActivated_t);
};

void
init_steam(entt::registry& r);

} // namespace game2d