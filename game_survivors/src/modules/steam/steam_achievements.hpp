#pragma once

#include <entt/fwd.hpp>

namespace game2d {

enum class SteamAchievementID
{
  ACH_WIN_ONE_GAME = 0,

  count,
};

struct SteamAchievement
{
  SteamAchievementID id;
  std::string name;
  std::string desc;
  bool m_bAchieved;
  int m_iIconImage;
};

class SteamAchievementManager
{
private:
  STEAM_CALLBACK(SteamAchievementManager, OnUserStatsReceived, UserStatsReceived_t);
};

struct SINGLE_SteamAchievementInfo
{
  std::vector<SteamAchievement> achievements;

  std::unordered_map<std::string, int32> stats;

  bool m_bStoreStats; // Store stats end of frame
  bool m_StatsInitialized = false;

  static SINGLE_SteamAchievementInfo instance;
};

void
init_steam_achievements(entt::registry& r);

void
on_game_complete__store_stats(entt::registry& r);

void
update_occasionally__steam_achievements(entt::registry& r);

} // namespace game2d