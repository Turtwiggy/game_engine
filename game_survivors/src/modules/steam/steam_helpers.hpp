#pragma once

#include <SDL2/SDL_log.h>
#include <entt/entt.hpp>
#include <steam/steam_api.h>

namespace game2d {

class SteamOverlayManager
{
private:
  STEAM_CALLBACK(SteamOverlayManager, OnGameOverlayActivated, GameOverlayActivated_t);
};

inline void
SteamOverlayManager::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
{
  if (pCallback->m_bActive)
    SDL_Log("Steam overlay now active");
  else
    SDL_Log("Steam overlay now inactive");
};

void
init_steam(entt::registry& r);

} // namespace game2d