#pragma once

#include <entt/fwd.hpp>

namespace game2d {

enum class SteamStats
{
  NumKills,
  NumGames,
  NumWins,
};

enum class SteamAchievementID
{
  ACH_WIN_ONE_GAME = 0,
  ACH_WIN_WITH_BOAT_DINGHY,
  ACH_WIN_WITH_BOAT_CASTAWAY,
  ACH_WIN_WITH_BOAT_BOND,
  ACH_WIN_WITH_BOAT_PBR,
  ACH_WIN_WITH_BOAT_RHIB,
  ACH_WIN_WITH_WEAPON_DECK_CANNON,
  ACH_WIN_WITH_WEAPON_GRAPESHOT_CANNON,
  ACH_WIN_WITH_WEAPON_SWIVEL_CANNON,
  ACH_WIN_WITH_WEAPON_SEA_TURRET,
  ACH_WIN_WITH_WEAPON_FLAMETHROWER,
  ACH_KILL_404,
  ACH_KILL_21600,
  ACH_KILL_10800,
  ACH_PERMAUPGRADE_ONE,
  ACH_PERMAUPGRADE_ALL,
  ACH_WIN_COOP,

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
on_game_complete__check_achievements(entt::registry& r);
void
on_shop_purchase__check_achievements(entt::registry& r);

void
update_occasionally__steam_achievements(entt::registry& r);

} // namespace game2d