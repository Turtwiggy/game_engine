#include "pch.hpp"

#include "steam_achievements.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_hull/hull_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_gameover/ui_gameover_components.hpp"

namespace game2d {

SINGLE_SteamAchievementInfo SINGLE_SteamAchievementInfo::instance;

// OUT_Z_ARRAY indicates an output array that will be null-terminated.
#if _MSC_VER >= 1600
// Include the annotation header file.
#include <sal.h>
#if _MSC_VER >= 1700
// VS 2012+
#define OUT_Z_ARRAY _Post_z_
#else
// VS 2010
#define OUT_Z_ARRAY _Deref_post_z_
#endif
#else
// gcc, clang, old versions of VS
#define OUT_Z_ARRAY
#endif

template<size_t maxLenInChars>
void
sprintf_safe(OUT_Z_ARRAY char (&pDest)[maxLenInChars], const char* pFormat, ...)
{
  va_list params;
  va_start(params, pFormat);
#ifdef POSIX
  vsnprintf(pDest, maxLenInChars, pFormat, params);
#else
  _vsnprintf(pDest, maxLenInChars, pFormat, params);
#endif
  pDest[maxLenInChars - 1] = '\0';
  va_end(params);
}

void
SteamAchievementManager::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
  SDL_Log("Steam has downloaded user stats.");
  SINGLE_SteamAchievementInfo& info_c = SINGLE_SteamAchievementInfo::instance;

  if (pCallback->m_eResult != k_EResultOK)
    return;

  auto* steam_user = SteamUser();
  auto* steam_user_stats = SteamUserStats();

  // load stats
  bool b0 = steam_user_stats->GetStat("NumKills", &info_c.stats["NumKills"]);
  // if (!b0)
  //   steam_user_stats->SetStat("NumKills", 0);

  bool b1 = steam_user_stats->GetStat("NumGames", &info_c.stats["NumGames"]);
  // if (!b1)
  //   steam_user_stats->SetStat("NumGames", 0);

  bool b2 = steam_user_stats->GetStat("NumWins", &info_c.stats["NumWins"]);
  // if (!b2)
  //   steam_user_stats->SetStat("NumWins", 0);

  for (auto& stat : info_c.stats)
    SDL_Log("(stat) %s: %i", stat.first.c_str(), stat.second);

  info_c.m_StatsInitialized = true;
};

void
request_stats(SINGLE_SteamAchievementInfo& info_c)
{
  info_c.stats.clear();

  auto* steam_user = SteamUser();
  auto* steam_user_stats = SteamUserStats();

  // download stats
  SteamUserStats()->RequestUserStats(steam_user->GetSteamID());
};

void
unlock_achievement(SINGLE_SteamAchievementInfo& info_c, SteamAchievement& ach)
{
  if (!info_c.m_StatsInitialized)
    return;
  SDL_Log("unlocking achievement: %s", ach.name.c_str());

  ach.m_bAchieved = true;

  ach.m_iIconImage = 0;

  // mark it down
  const auto id_as_str = std::string(magic_enum::enum_name(ach.id));
  auto* steam_user_stats = SteamUserStats();
  steam_user_stats->SetAchievement(id_as_str.c_str());

  // Store stats end of frame
  info_c.m_bStoreStats = true;
}

void
init_steam_achievements(entt::registry& r)
{
  // SteamFriends()->SetRichPresence( "status", "Main menu: finding lobbies" );

  auto* steam_user = SteamUser();
  auto* steam_user_stats = SteamUserStats();
  SINGLE_SteamAchievementInfo& achievement_c = SINGLE_SteamAchievementInfo::instance;

#if defined(_DEBUG)
#define RESET_ACHIEVEMENTS 1
#if defined(RESET_ACHIEVEMENTS)
  // steam_user_stats->ResetAllStats(true);
#endif
#endif

  for (int i = 0; i < (int)SteamAchievementID::count; i++) {
    const auto achivement = static_cast<SteamAchievementID>(i);
    const auto ach_str = std::string(magic_enum::enum_name(achivement));
    const auto ach_cstr = ach_str.c_str();

    std::string name = steam_user_stats->GetAchievementDisplayAttribute(ach_cstr, "name");
    std::string desc = steam_user_stats->GetAchievementDisplayAttribute(ach_cstr, "desc");

    bool achieved = false;
    steam_user_stats->GetAchievement(ach_cstr, &achieved);

    SteamAchievement ach{
      .id = static_cast<SteamAchievementID>(i),
      .name = name,
      .desc = desc,
      .m_bAchieved = achieved,
      .m_iIconImage = 0,
    };
    SDL_Log("(found achievement) %s: %s, achieved: %i", name.c_str(), desc.c_str(), achieved);

    achievement_c.achievements.push_back(ach);
  }

  request_stats(achievement_c);
};

void
on_game_complete__store_stats(entt::registry& r)
{
  SINGLE_SteamAchievementInfo& info_c = SINGLE_SteamAchievementInfo::instance;

  if (!info_c.m_StatsInitialized)
    return;

  int killed_before = info_c.stats["NumKills"];

  auto survive_e = get_first<SINGLE_SurviveStatsComponent>(r);
  if (survive_e != entt::null) {
    auto& survive_ui_c = r.get<SINGLE_SurviveStatsComponent>(survive_e);
    auto& gameover_ui_c = r.get<SINGLE_GameoverUI>(get_first<SINGLE_GameoverUI>(r));

    // update enemy killed.
    int killed_in_run = survive_ui_c.enemies_killed;
    info_c.stats["NumKills"] += killed_in_run;

    // update NumGames
    info_c.stats["NumGames"] += 1;

    // update NumWins
    if (gameover_ui_c.request.value().win_condition)
      info_c.stats["NumWins"] += 1;

    SDL_Log("updating steam stats... (prv) %i, (inc) %i, (new) %i", killed_before, killed_in_run, info_c.stats["NumKills"]);
  }

  // We want to update stats the next frame.
  info_c.m_bStoreStats = true;
  SDL_Log("Next frame will store stats");
};

void
on_game_complete__check_achievements(entt::registry& r)
{
  SINGLE_SteamAchievementInfo& achievement_c = SINGLE_SteamAchievementInfo::instance;
  if (!achievement_c.m_StatsInitialized)
    return;

  auto& gameover_ui_c = r.get<SINGLE_GameoverUI>(get_first<SINGLE_GameoverUI>(r));
  if (!gameover_ui_c.request.has_value())
    return;

  // these achievements are on win
  if (!gameover_ui_c.request->win_condition)
    return;

  const auto check_unlock_hull = [&](SteamAchievement& ach, std::string key) -> bool {
    auto view = r.view<PlayerComponent, HullKeyComponent>();
    for (const auto& [ent, player_c, hull_c] : view.each()) {
      if (hull_c.key == key) {
        unlock_achievement(achievement_c, ach);
        return true;
      }
    }
    return false;
  };
  const auto check_unlock_weapon = [&](SteamAchievement& ach, std::string key) -> bool {
    auto view = r.view<PlayerComponent, WeaponKeyComponent>();
    for (const auto& [ent, player_c, weapon_c] : view.each()) {
      if (weapon_c.key == key) {
        unlock_achievement(achievement_c, ach);
        return true;
      }
    }
    return false;
  };

  for (auto& ach : achievement_c.achievements) {
    if (ach.m_bAchieved)
      continue;

    switch (ach.id) {
      case SteamAchievementID::ACH_WIN_WITH_BOAT_DINGHY:
        if (check_unlock_hull(ach, "dinghy"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_BOAT_CASTAWAY:
        if (check_unlock_hull(ach, "castaway"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_BOAT_BOND:
        if (check_unlock_hull(ach, "bond"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_BOAT_PBR:
        if (check_unlock_hull(ach, "pbr"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_BOAT_RHIB:
        if (check_unlock_hull(ach, "rhib"))
          continue;
        break;

      case SteamAchievementID::ACH_WIN_WITH_WEAPON_DECK_CANNON:
        if (check_unlock_weapon(ach, "weapon_deck_cannon"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_WEAPON_GRAPESHOT_CANNON:
        if (check_unlock_weapon(ach, "weapon_grapeshot_cannon"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_WEAPON_SWIVEL_CANNON:
        if (check_unlock_weapon(ach, "weapon_swivel_cannon"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_WEAPON_SEA_TURRET:
        if (check_unlock_weapon(ach, "weapon_sea_turret"))
          continue;
        break;
      case SteamAchievementID::ACH_WIN_WITH_WEAPON_FLAMETHROWER:
        if (check_unlock_weapon(ach, "weapon_flamethrower"))
          continue;
        break;

      case SteamAchievementID::ACH_WIN_COOP:
        auto view = r.view<const PlayerBoatComponent>();
        if (view.size() > 1)
          unlock_achievement(achievement_c, ach);
        break;
    }
  }
};

void
on_shop_purchase__check_achievements(entt::registry& r)
{
  SINGLE_SteamAchievementInfo& achievement_c = SINGLE_SteamAchievementInfo::instance;
  if (!achievement_c.m_StatsInitialized)
    return;

  const auto& on_disk_upg_c = get_first_component<SINGLE_PersistentUpgrades>(r);
  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  const int available_stats = (int)on_disk_upg_c.upgrades.size();
  const int available_boats = (int)hulls_c.hulls.size();
  int available_weapons = 0;
  for (auto& weapon : weapons_c.weapons)
    if (weapon.useable_by_as_enum == WEAPON_USEABLE_BY::BOATS)
      available_weapons++;
  const int available_unlocks = available_boats + available_weapons + available_stats;

  int purchased_boats = 0;
  for (auto& hull : hulls_c.hulls) {
    const auto val_opt = savefile_get_key(r, hull.key);
    if (!val_opt.has_value())
      continue;
    purchased_boats++;
  }

  int purchased_weapons = 0;
  for (auto& weapon : weapons_c.weapons) {
    const auto val_opt = savefile_get_key(r, weapon.key);
    if (!val_opt.has_value())
      continue;
    purchased_weapons++;
  }

  int purchased_upgrades = 0;
  for (int i = 0; i < (int)UpgradeableStat::count; i++) {
    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
    const auto val_opt = savefile_get_key(r, stat_str);
    if (!val_opt.has_value())
      continue;
    const auto val_json = val_opt.value();

    // note ( ignore levels of the upgrade ), its either unlocked or not
    purchased_upgrades++;
  }

  // note: minus 2 because 2 are unlocked by default so dont count them as purchased
  int total_purchased = (purchased_boats + purchased_weapons + purchased_upgrades) - 2;
  total_purchased = std::max(total_purchased, 0);
  int max_unlocks = available_unlocks - 2;

  for (auto& ach : achievement_c.achievements) {
    if (ach.m_bAchieved)
      continue;

    switch (ach.id) {
      case SteamAchievementID::ACH_PERMAUPGRADE_ONE:
        if (total_purchased > 0)
          unlock_achievement(achievement_c, ach);
        break;
      case SteamAchievementID::ACH_PERMAUPGRADE_ALL:
        if (total_purchased >= max_unlocks)
          unlock_achievement(achievement_c, ach);
        break;
    }
  }
}

void
update_occasionally__steam_achievements(entt::registry& r)
{
  SINGLE_SteamAchievementInfo& achievement_c = SINGLE_SteamAchievementInfo::instance;

  if (!achievement_c.m_StatsInitialized)
    return;

  // EvaluateAchievements...
  for (auto& ach : achievement_c.achievements) {
    if (ach.m_bAchieved)
      continue;

    switch (ach.id) {
      case SteamAchievementID::ACH_WIN_ONE_GAME: {
        if (achievement_c.stats["NumWins"])
          unlock_achievement(achievement_c, ach);
        break;
      }

      // kill X thing achievements
      case SteamAchievementID::ACH_KILL_404: {
        if (achievement_c.stats["NumKills"] >= 404)
          unlock_achievement(achievement_c, ach);
        break;
      }
      case SteamAchievementID::ACH_KILL_10800: {
        if (achievement_c.stats["NumKills"] >= 10800)
          unlock_achievement(achievement_c, ach);
        break;
      }
      case SteamAchievementID::ACH_KILL_21600: {
        if (achievement_c.stats["NumKills"] >= 21600)
          unlock_achievement(achievement_c, ach);
        break;
      }

      default:
        break;
    }
  }

  // note: this should probably be an event, not in the update() loop.
  // However, if the steam api call fails,
  // you would miss unlocking the achievement. for the moment, its just in the
  // update loop so it will try again and again, but it this could be improved.
  on_shop_purchase__check_achievements(r);

  // storestatsifnecessary...
  if (achievement_c.m_bStoreStats) {
    auto* steam_user_stats = SteamUserStats();

    // update all stats
    for (const auto& [key, value] : achievement_c.stats)
      steam_user_stats->SetStat(key.c_str(), value);

    // If this failed, we never sent anything to the server, try again later.
    bool bSuccess = steam_user_stats->StoreStats();
    achievement_c.m_bStoreStats = !bSuccess;
  }
}

} // namespace game2d